#include <kernel/sched/sched.h>
#include <kernel/load/elf.h>
#include <stddef.h>
#include <kernel/mm/mm.h>
#include <kernel/libk/io.h>
#include <limits.h>
#include <kernel/libk/ds/list.h>
#include <kernel/tls.h>
#include <kernel/fs/uvfs.h>
#include <kernel/libk/string.h>
#include <kernel/cpu/paging.h>

#define IA32_KERNEL_GS_BASE 0xC0000102

// Forward declarations
static void task_init(struct task* task);
static struct list tasks;

[[noreturn]] void
sched_init(void)
{
    list_init(&tasks);

    struct task* task = kzmalloc(sizeof(struct task));
    task_init(task);
    list_push(&tasks, &task->link);
    tls.current_task = task;

    load_init_process("/bin/init");
}

[[noreturn]] void
sched_exit(uint64_t code)
{
    assert(tls.current_task);

    struct task* next_task = container_of(
        list_next_circular(&tasks, &tls.current_task->link), struct task, link);

    if (next_task == tls.current_task)
        panic("last task exited with code %lld\n", code);

    panic("unimplemented task switch\n");
}

static void
task_init(struct task* task)
{
    memset(task, 0, sizeof(struct task));

    task->kernel_rsp = (uint64_t)alloc_kernel_stack();
    task->pml4 = boot_pml4;
    list_init(&task->files);

    list_node_init(&tasks, &task->link);
    list_push(&tasks, &task->link);
}

static void*
copy_address_space(struct pt_entry* pml4_src)
{
    // Allocate a new PML4 for the child
    struct pt_entry* pml4_new = alloc_pagez(1);

    // Copy all entries
    for (size_t i = 0; i < PML4_ENTRIES; ++i) {
        struct pt_entry* src_pml4e = &pml4_src[i];
        struct pt_entry* dst_pml4e = &pml4_new[i];
        if (!src_pml4e->present) continue;

        // If this is a user entry, recursively copy the lower tables
        if (src_pml4e->user_supervisor) {
            // Allocate a new PDPT for the child
            struct pt_entry* pdpt_src = paddr_to_vaddr_kernel_data(
                (void*)((uintptr_t)src_pml4e->address << PAGE_SIZE_BITS));
            struct pt_entry* pdpt_new = alloc_pagez(1);
            init_pt_entry(dst_pml4e, vaddr_to_paddr_kernel_data(pdpt_new),
                          src_pml4e->read_write, src_pml4e->user_supervisor,
                          src_pml4e->page_write_through,
                          src_pml4e->page_cache_disabled,
                          src_pml4e->execute_disable);

            for (size_t j = 0; j < PDPT_ENTRIES; ++j) {
                struct pt_entry* src_pdpte = &pdpt_src[j];
                struct pt_entry* dst_pdpte = &pdpt_new[j];
                if (!src_pdpte->present) continue;
                if (src_pdpte->user_supervisor) {
                    struct pt_entry* pd_src = paddr_to_vaddr_kernel_data(
                        (void*)((uintptr_t)src_pdpte->address
                                << PAGE_SIZE_BITS));
                    struct pt_entry* pd_new = alloc_pagez(1);
                    init_pt_entry(dst_pdpte, vaddr_to_paddr_kernel_data(pd_new),
                                  src_pdpte->read_write,
                                  src_pdpte->user_supervisor,
                                  src_pdpte->page_write_through,
                                  src_pdpte->page_cache_disabled,
                                  src_pdpte->execute_disable);

                    for (size_t k = 0; k < PD_ENTRIES; ++k) {
                        struct pt_entry* src_pde = &pd_src[k];
                        struct pt_entry* dst_pde = &pd_new[k];
                        if (!src_pde->present) continue;
                        if (src_pde->user_supervisor) {
                            struct pt_entry* pt_src =
                                paddr_to_vaddr_kernel_data(
                                    (void*)((uintptr_t)src_pde->address
                                            << PAGE_SIZE_BITS));
                            struct pt_entry* pt_new = alloc_pagez(1);
                            init_pt_entry(
                                dst_pde, vaddr_to_paddr_kernel_data(pt_new),
                                src_pde->read_write, src_pde->user_supervisor,
                                src_pde->page_write_through,
                                src_pde->page_cache_disabled,
                                src_pde->execute_disable);

                            for (size_t l = 0; l < PT_ENTRIES; ++l) {
                                struct pt_entry* src_pte = &pt_src[l];
                                struct pt_entry* dst_pte = &pt_new[l];
                                if (!src_pte->present) continue;
                                if (src_pte->user_supervisor) {
                                    // Allocate a new physical page for the
                                    // child
                                    void* src_page = paddr_to_vaddr_kernel_data(
                                        (void*)((uintptr_t)src_pte->address
                                                << PAGE_SIZE_BITS));
                                    void* new_page = alloc_pagez(1);
                                    memcpy(new_page, src_page, PAGE_SIZE);
                                    init_pt_entry(
                                        dst_pte,
                                        vaddr_to_paddr_kernel_data(new_page),
                                        src_pte->read_write,
                                        src_pte->user_supervisor,
                                        src_pte->page_write_through,
                                        src_pte->page_cache_disabled,
                                        src_pte->execute_disable);
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // Kernel entry: just copy the entry by value (shared tables)
            *dst_pml4e = *src_pml4e;
        }
    }
    return pml4_new;
}

uint64_t
sched_fork(void)
{
    kprintf("sched_fork\n");
    struct task* next_task = kzmalloc(sizeof(struct task));

    // Allocate a new kernel stack
    next_task->kernel_rsp = (uint64_t)alloc_kernel_stack();

    // Copy the user registers
    next_task->user_regs = tls.current_task->user_regs;

    // Copy the address space
    next_task->pml4 = copy_address_space(tls.current_task->pml4);

    // Copy files
    list_init(&next_task->files);
    list_foreach(&tls.current_task->files, node)
    {
        struct file* file = container_of(node, struct file, link);
        struct file* new_file = kzmalloc(sizeof(struct file));
        *new_file = *file;
        list_node_init(&next_task->files, &new_file->link);
        list_push(&next_task->files, &new_file->link);
    }

    list_node_init(&tasks, &next_task->link);
    list_push(&tasks, &next_task->link);

    return 0;
}

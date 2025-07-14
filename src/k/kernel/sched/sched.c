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
static void task_init(void);
static struct list tasks;

[[noreturn]] void
sched_init(void)
{
    list_init(&tasks);
    task_init();
    load_process("/bin/init");
}

[[noreturn]] void
sched_switch(struct task* next_task)
{
    list_remove(&tasks, &tls.current_task->link);

    tls.current_task = next_task;
    tls.kernel_rsp = (uint64_t)alloc_kernel_stack();
    tls.user_rsp = next_task->user_regs.rsp;

    asm volatile("movq %0, %%cr3\n"
                 "movq %[user_regs], %%rdi\n"
                 // Restore general purpose registers
                 "movq 0(%%rdi), %%r15\n"
                 "movq 8(%%rdi), %%r14\n"
                 "movq 16(%%rdi), %%r13\n"
                 "movq 24(%%rdi), %%r12\n"
                 "movq 32(%%rdi), %%rbp\n"
                 "movq 40(%%rdi), %%rbx\n"
                 "movq 48(%%rdi), %%r11\n"
                 "movq 56(%%rdi), %%r10\n"
                 "movq 64(%%rdi), %%r9\n"
                 "movq 72(%%rdi), %%r8\n"
                 "movq 80(%%rdi), %%rax\n"
                 "movq 88(%%rdi), %%rcx\n"
                 "movq 96(%%rdi), %%rdx\n"
                 "movq 104(%%rdi), %%rsi\n"
                 "movq 120(%%rdi), %%rsp\n"
                 "movq 112(%%rdi), %%rdi\n"
                 "swapgs\n"
                 "sysretq\n"
                 :
                 : "r"(vaddr_to_paddr_kernel_data(next_task->pml4)),
                   [user_regs] "r"(&next_task->user_regs)
                 : "memory");

    panic("unreachable\n");
}

[[noreturn]] void
sched_exit(uint64_t code)
{
    assert(tls.current_task);

    struct task* next_task = container_of(
        list_next_circular(&tasks, &tls.current_task->link), struct task, link);

    if (next_task == tls.current_task)
        panic("last task exited with code %lld\n", code);

    sched_switch(next_task);
}

static void
task_init(void)
{
    assert(tls.current_task != NULL);
    assert(tls.current_task->pml4 != NULL);

    list_init(&tls.current_task->files);
    list_node_init(&tasks, &tls.current_task->link);
    list_push(&tasks, &tls.current_task->link);
}

static void*
copy_address_space(struct pt_entry* pml4_src)
{
    struct pt_entry* pml4_new = alloc_pagez(1);

    for (size_t i = 0; i < PML4_ENTRIES; ++i) {
        struct pt_entry* src_pml4e = &pml4_src[i];
        struct pt_entry* dst_pml4e = &pml4_new[i];
        if (!src_pml4e->present) continue;

        // Copy the PML4 entry by value
        *dst_pml4e = *src_pml4e;

        // Walk down to the PT level and deep-copy user pages
        struct pt_entry* pdpt_src = paddr_to_vaddr_kernel_data(
            (void*)((uintptr_t)src_pml4e->address << PAGE_SIZE_BITS));
        struct pt_entry* pdpt_new = NULL;
        for (size_t j = 0; j < PDPT_ENTRIES; ++j) {
            struct pt_entry* src_pdpte = &pdpt_src[j];
            if (!src_pdpte->present) continue;

            struct pt_entry* pd_src = paddr_to_vaddr_kernel_data(
                (void*)((uintptr_t)src_pdpte->address << PAGE_SIZE_BITS));
            struct pt_entry* pd_new = NULL;
            for (size_t k = 0; k < PD_ENTRIES; ++k) {
                struct pt_entry* src_pde = &pd_src[k];
                if (!src_pde->present) continue;

                struct pt_entry* pt_src = paddr_to_vaddr_kernel_data(
                    (void*)((uintptr_t)src_pde->address << PAGE_SIZE_BITS));
                struct pt_entry* pt_new = NULL;
                for (size_t l = 0; l < PT_ENTRIES; ++l) {
                    struct pt_entry* src_pte = &pt_src[l];
                    if (!src_pte->present) continue;

                    if (src_pte->user_supervisor) {
                        // Allocate new PT if needed
                        if (!pt_new) {
                            pt_new = alloc_pagez(1);
                            // Allocate new PD if needed
                            if (!pd_new) {
                                pd_new = alloc_pagez(1);
                                // Allocate new PDPT if needed
                                if (!pdpt_new) {
                                    pdpt_new = alloc_pagez(1);
                                    // Link new PDPT to new PML4
                                    init_pt_entry(
                                        dst_pml4e,
                                        vaddr_to_paddr_kernel_data(pdpt_new),
                                        src_pml4e->read_write,
                                        src_pml4e->user_supervisor,
                                        src_pml4e->page_write_through,
                                        src_pml4e->page_cache_disabled,
                                        src_pml4e->execute_disable);
                                }
                                // Link new PD to new PDPT
                                struct pt_entry* pdpt_new_entries = pdpt_new;
                                struct pt_entry* dst_pdpte =
                                    &pdpt_new_entries[j];
                                init_pt_entry(
                                    dst_pdpte,
                                    vaddr_to_paddr_kernel_data(pd_new),
                                    src_pdpte->read_write,
                                    src_pdpte->user_supervisor,
                                    src_pdpte->page_write_through,
                                    src_pdpte->page_cache_disabled,
                                    src_pdpte->execute_disable);
                            }
                            // Link new PT to new PD
                            struct pt_entry* pd_new_entries = pd_new;
                            struct pt_entry* dst_pde = &pd_new_entries[k];
                            init_pt_entry(
                                dst_pde, vaddr_to_paddr_kernel_data(pt_new),
                                src_pde->read_write, src_pde->user_supervisor,
                                src_pde->page_write_through,
                                src_pde->page_cache_disabled,
                                src_pde->execute_disable);
                        }
                        // Deep copy user page
                        struct pt_entry* pt_new_entries = pt_new;
                        struct pt_entry* dst_pte = &pt_new_entries[l];
                        void* src_page = paddr_to_vaddr_kernel_data(
                            (void*)((uintptr_t)src_pte->address
                                    << PAGE_SIZE_BITS));
                        void* new_page = alloc_pagez(1);
                        memcpy(new_page, src_page, PAGE_SIZE);
                        init_pt_entry(
                            dst_pte, vaddr_to_paddr_kernel_data(new_page),
                            src_pte->read_write, src_pte->user_supervisor,
                            src_pte->page_write_through,
                            src_pte->page_cache_disabled,
                            src_pte->execute_disable);
                    }
                    // Kernel page: nothing to do, already copied by value
                }
            }
        }
    }
    return pml4_new;
}

void
sched_fork(void)
{
    struct task* next_task = kzmalloc(sizeof(struct task));

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

    // Child gets return value of 0
    next_task->user_regs.rax = 0;
    // Parent gets return value of child's id
    tls.current_task->user_regs.rax = next_task->link.id;
}

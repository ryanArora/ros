.equ KERNEL_RSP_OFFSET, 0x0
.equ USER_RSP_OFFSET, 0x8

.global syscall_handler
syscall_handler:
    # Save the user stack pointer, restore the kernel stack pointer
    swapgs
    movq %rsp, %gs:USER_RSP_OFFSET
    movq %gs:KERNEL_RSP_OFFSET, %rsp

    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %r10, %rcx            # So that syscall_handler_c works with SystemV
    call syscall_handler_c

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    # Restore the user stack pointer
    movq %gs:USER_RSP_OFFSET, %rsp
    swapgs

    sysretq

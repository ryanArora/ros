.equ CURRENT_TASK_OFFSET, 0
.equ KERNEL_RSP_OFFSET, 8
.equ USER_RSP_OFFSET, 16

.global syscall_handler
syscall_handler:
    # Save the user stack pointer, restore the kernel stack pointer
    swapgs
    movq %rsp, %gs:USER_RSP_OFFSET
    movq %gs:KERNEL_RSP_OFFSET, %rsp

    # Save rdi, and we will use rdi as the base address to struct user_regs
    pushq %rdi
    movq %gs:CURRENT_TASK_OFFSET, %rdi

    # Save general purpose registers to struct user_regs
    movq %r15, 0(%rdi)
    movq %r14, 8(%rdi)
    movq %r13, 16(%rdi)
    movq %r12, 24(%rdi)
    movq %rbp, 32(%rdi)
    movq %rbx, 40(%rdi)
    movq %r11, 48(%rdi)
    movq %r10, 56(%rdi)
    movq %r9,  64(%rdi)
    movq %r8,  72(%rdi)
    movq %rax, 80(%rdi)
    movq %rcx, 88(%rdi)
    movq %rdx, 96(%rdi)
    movq %rsi, 104(%rdi)
    
    # Save rdi to struct user_regs
    popq %rdi                          # Restore rdi
    movq %gs:CURRENT_TASK_OFFSET, %rsi # Use rsi as the base address instead
    movq %rdi, 112(%rsi)               # Save rdi to struct user_regs
    movq 104(%rsi), %rsi               # Restore rsi

    # Save rsp to struct user_regs
    pushq %rdi
    pushq %rsi
    movq %gs:CURRENT_TASK_OFFSET, %rsi
    movq %gs:USER_RSP_OFFSET, %rdi
    movq %rdi, 120(%rsi)
    popq %rsi
    popq %rdi

    movq %r10, %rcx            # So that syscall_handler_c works with SystemV
    xorq %rbp, %rbp            # Mark the start of stack frames
    call syscall_handler_c

    # Restore the user registers using rdi as the base address to struct user_regs
    movq %gs:CURRENT_TASK_OFFSET, %rdi

    # Restore general purpose registers from struct user_regs
    movq 0(%rdi), %r15
    movq 8(%rdi), %r14
    movq 16(%rdi), %r13
    movq 24(%rdi), %r12
    movq 32(%rdi), %rbp
    movq 40(%rdi), %rbx
    movq 48(%rdi), %r11
    movq 56(%rdi), %r10
    movq 64(%rdi), %r9
    movq 72(%rdi), %r8
    movq 80(%rdi), %rax
    movq 88(%rdi), %rcx
    movq 96(%rdi), %rdx
    movq 104(%rdi), %rsi
    movq 112(%rdi), %rdi

    # Restore the user stack pointer
    movq %gs:USER_RSP_OFFSET, %rsp
    swapgs

    sysretq

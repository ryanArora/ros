.equ CURRENT_TASK_OFFSET, 0
.equ KERNEL_RSP_OFFSET, 8
.equ USER_RSP_OFFSET, 16

.equ R15_OFFSET, 0
.equ R14_OFFSET, 8
.equ R13_OFFSET, 16
.equ R12_OFFSET, 24
.equ RBP_OFFSET, 32
.equ RBX_OFFSET, 40
.equ R11_OFFSET, 48
.equ R10_OFFSET, 56
.equ R9_OFFSET, 64
.equ R8_OFFSET, 72
.equ RAX_OFFSET, 80
.equ RCX_OFFSET, 88
.equ RDX_OFFSET, 96
.equ RSI_OFFSET, 104
.equ RDI_OFFSET, 112
.equ RIP_OFFSET, 120
.equ RFLAGS_OFFSET, 128
.equ RSP_OFFSET, 136

.global timer_interrupt_handler
timer_interrupt_handler:
    pushfq
    # After pushfq, CS is now at 16(%rsp) (was at 8(%rsp) before pushfq)
    testb $3, 16(%rsp)
    jnz .Luser_interrupt
    jmp .Lkernel_interrupt

.Luser_interrupt:
    popfq

    swapgs
    movq %gs:CURRENT_TASK_OFFSET, %rdi

    movq %r15, R15_OFFSET(%rdi)
    movq %r14, R14_OFFSET(%rdi)
    movq %r13, R13_OFFSET(%rdi)
    movq %r12, R12_OFFSET(%rdi)
    movq %rbp, RBP_OFFSET(%rdi)
    movq %rbx, RBX_OFFSET(%rdi)
    movq %r11, R11_OFFSET(%rdi)
    movq %r10, R10_OFFSET(%rdi)
    movq %r9, R9_OFFSET(%rdi)
    movq %r8, R8_OFFSET(%rdi)
    movq %rax, RAX_OFFSET(%rdi)
    movq %rcx, RCX_OFFSET(%rdi)
    movq %rdx, RDX_OFFSET(%rdi)
    movq %rsi, RSI_OFFSET(%rdi)
    
    # rip
    movq (%rsp), %rsi
    movq %rsi, RIP_OFFSET(%rdi)

    # cs is at 8(%rsp) but we don't need to save it

    # rflags
    movq 16(%rsp), %rsi
    movq %rsi, RFLAGS_OFFSET(%rdi)

    # rsp
    movq 24(%rsp), %rsi
    movq %rsi, RSP_OFFSET(%rdi)

    # rdi
    movq %rdi, RDI_OFFSET(%rdi)

    call timer_interrupt_handler_user_c

    # Restore user_regs
    movq %gs:CURRENT_TASK_OFFSET, %rdi
    movq R15_OFFSET(%rdi), %r15
    movq R14_OFFSET(%rdi), %r14
    movq R13_OFFSET(%rdi), %r13
    movq R12_OFFSET(%rdi), %r12
    movq RBP_OFFSET(%rdi), %rbp
    movq RBX_OFFSET(%rdi), %rbx
    movq R11_OFFSET(%rdi), %r11
    movq R10_OFFSET(%rdi), %r10
    movq R9_OFFSET(%rdi), %r9
    movq R8_OFFSET(%rdi), %r8
    movq RAX_OFFSET(%rdi), %rax
    movq RCX_OFFSET(%rdi), %rcx
    movq RDX_OFFSET(%rdi), %rdx
    movq RSI_OFFSET(%rdi), %rsi
    movq RDI_OFFSET(%rdi), %rdi

    swapgs

    pushq %rax
    movb $0x20, %al
    outb %al, $0x20
    outb %al, $0xA0
    popq %rax
    
    iretq

.Lkernel_interrupt:
    popfq

    pushq %rax
    movb $0x20, %al
    outb %al, $0x20
    outb %al, $0xA0
    popq %rax
    iretq

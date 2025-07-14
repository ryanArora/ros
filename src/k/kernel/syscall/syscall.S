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

.global syscall_handler
syscall_handler:
    # Save the user stack pointer, restore the kernel stack pointer
    swapgs
    movq %rsp, %gs:USER_RSP_OFFSET
    movq %gs:KERNEL_RSP_OFFSET, %rsp

    # Save user registers
    pushq %rdi
    movq %gs:CURRENT_TASK_OFFSET, %rdi
    movq %r15, R15_OFFSET(%rdi)
    movq %r14, R14_OFFSET(%rdi)
    movq %r13, R13_OFFSET(%rdi)
    movq %r12, R12_OFFSET(%rdi)
    movq %rbp, RBP_OFFSET(%rdi)
    movq %rbx, RBX_OFFSET(%rdi)
    # r11 was clobbered
    movq %r10, R10_OFFSET(%rdi)
    movq %r9,  R9_OFFSET(%rdi)
    movq %r8,  R8_OFFSET(%rdi)
    movq %rax, RAX_OFFSET(%rdi)
    # rcx was clobbered
    movq %rdx, RDX_OFFSET(%rdi)
    movq %rsi, RSI_OFFSET(%rdi)
    movq %rcx, RIP_OFFSET(%rdi)
    movq %r11, RFLAGS_OFFSET(%rdi)

    # rsp
    movq %gs:USER_RSP_OFFSET, %rsi
    movq %rsi, RSP_OFFSET(%rdi)
    
    # rdi
    popq %rdi
    movq %gs:CURRENT_TASK_OFFSET, %rsi
    movq %rdi, RDI_OFFSET(%rsi)
    movq RSI_OFFSET(%rsi), %rsi

    movq %r10, %rcx            # So that syscall_handler_c works with SystemV
    xorq %rbp, %rbp            # Mark the start of stack frames
    call syscall_handler_c

    # Restore the user registers
    movq %gs:CURRENT_TASK_OFFSET, %rdi
    movq R15_OFFSET(%rdi), %r15
    movq R14_OFFSET(%rdi), %r14
    movq R13_OFFSET(%rdi), %r13
    movq R12_OFFSET(%rdi), %r12
    movq RBP_OFFSET(%rdi), %rbp
    movq RBX_OFFSET(%rdi), %rbx
    # r11 was clobbered
    movq R10_OFFSET(%rdi), %r10
    movq R9_OFFSET(%rdi), %r9
    movq R8_OFFSET(%rdi), %r8
    movq RAX_OFFSET(%rdi), %rax
    # rcx was clobbered
    movq RDX_OFFSET(%rdi), %rdx
    movq RSI_OFFSET(%rdi), %rsi
    movq RIP_OFFSET(%rdi), %rcx
    movq RFLAGS_OFFSET(%rdi), %r11
    movq RSP_OFFSET(%rdi), %rsp
    movq RDI_OFFSET(%rdi), %rdi
    swapgs
    sysretq

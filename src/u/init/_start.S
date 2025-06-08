.section .text
.global _start

_start:
    movq $0, %rdi
    movq $1, %rsi
    movq $2, %rdx
    movq $3, %r10
    movq $4, %r8
    movq $5, %r9
    syscall
    jmp _start

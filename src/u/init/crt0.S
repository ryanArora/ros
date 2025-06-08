.section .text
.global _start

_start:
    xorq %rbp, %rbp
    call main
    xorq %rdi, %rdi
    movq %rax, %rsi
    syscall

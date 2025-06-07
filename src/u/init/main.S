.section .text
.global _start

_start:
    syscall
    jmp _start

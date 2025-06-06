.global gdt_reload_segments
gdt_reload_segments:
    pushq $0x08
    leaq .reload(%rip), %rax
    pushq %rax
    lretq
.reload:
    movw $0x10, %ax
    movw %ax, %ss
    movw $0x00, %ax
    movw %ax, %ds
    movw %ax, %es
    ret

    pushq $0x10
    pushq %rbp
    pushfq
    pushq $0x08
    pushq %rax
    iretq

.global gdt_flush_tss
gdt_flush_tss:
    movw $((5 * 8) | 0), %ax   # Load segment selector (TSS descriptor) into AX
    ltr %ax                    # Load Task Register
    ret

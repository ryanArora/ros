.global timer_interrupt_handler
timer_interrupt_handler:
    pushq %rax
    movb $0x20, %al
    outb %al, $0x20
    outb %al, $0xA0
    popq %rax
    iretq

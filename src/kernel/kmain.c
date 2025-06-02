[[noreturn]] void
kmain(void)
{
    while (1)
        __asm__("cli\n"
                "hlt");
}

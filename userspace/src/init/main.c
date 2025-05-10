void
_start(void)
{
    while (1)
        __asm__("cli"); // Just issue a general protection fault for now
}

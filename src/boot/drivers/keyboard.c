#include "keyboard.h"
#include "../lib/io.h"
#include <stdint.h>

static bool lshift;
static bool rshift;
static bool caps_lock;

__attribute__((interrupt)) void
keyboard_interrupt_handler(void* frame)
{
    (void)(frame);
    uint8_t scancode = inb(0x60);

    bool is_shift = lshift || rshift;
    bool use_uppercase = is_shift != caps_lock;

    // clang-format off
    switch (scancode) {
    case 0x02: kputchar(is_shift ? '!' : '1'); break;
    case 0x03: kputchar(is_shift ? '@' : '2'); break;
    case 0x04: kputchar(is_shift ? '#' : '3'); break;
    case 0x05: kputchar(is_shift ? '$' : '4'); break;
    case 0x06: kputchar(is_shift ? '%' : '5'); break;
    case 0x07: kputchar(is_shift ? '^' : '6'); break;
    case 0x08: kputchar(is_shift ? '&' : '7'); break;
    case 0x09: kputchar(is_shift ? '*' : '8'); break;
    case 0x0A: kputchar(is_shift ? '(' : '9'); break;
    case 0x0B: kputchar(is_shift ? ')' : '0'); break;
    case 0x10: kputchar(use_uppercase ? 'Q' : 'q'); break;
    case 0x11: kputchar(use_uppercase ? 'W' : 'w'); break;
    case 0x12: kputchar(use_uppercase ? 'E' : 'e'); break;
    case 0x13: kputchar(use_uppercase ? 'R' : 'r'); break;
    case 0x14: kputchar(use_uppercase ? 'T' : 't'); break;
    case 0x15: kputchar(use_uppercase ? 'Y' : 'y'); break;
    case 0x16: kputchar(use_uppercase ? 'U' : 'u'); break;
    case 0x17: kputchar(use_uppercase ? 'I' : 'i'); break;
    case 0x18: kputchar(use_uppercase ? 'O' : 'o'); break;
    case 0x19: kputchar(use_uppercase ? 'P' : 'p'); break;
    case 0x1A: kputchar(use_uppercase ? '{' : '['); break;
    case 0x1B: kputchar(use_uppercase ? '}' : ']'); break;
    case 0x1E: kputchar(use_uppercase ? 'A' : 'a'); break;
    case 0x1F: kputchar(use_uppercase ? 'S' : 's'); break;
    case 0x20: kputchar(use_uppercase ? 'D' : 'd'); break;
    case 0x21: kputchar(use_uppercase ? 'F' : 'f'); break;
    case 0x22: kputchar(use_uppercase ? 'G' : 'g'); break;
    case 0x23: kputchar(use_uppercase ? 'H' : 'h'); break;
    case 0x24: kputchar(use_uppercase ? 'J' : 'j'); break;
    case 0x25: kputchar(use_uppercase ? 'K' : 'k'); break;
    case 0x26: kputchar(use_uppercase ? 'L' : 'l'); break;
    case 0x2C: kputchar(use_uppercase ? 'Z' : 'z'); break;
    case 0x2D: kputchar(use_uppercase ? 'X' : 'x'); break;
    case 0x2E: kputchar(use_uppercase ? 'C' : 'c'); break;
    case 0x2F: kputchar(use_uppercase ? 'V' : 'v'); break;
    case 0x30: kputchar(use_uppercase ? 'B' : 'b'); break;
    case 0x31: kputchar(use_uppercase ? 'N' : 'n'); break;
    case 0x32: kputchar(use_uppercase ? 'M' : 'm'); break;
    case 0x27: kputchar(is_shift ? ':' : ';'); break;
    case 0x28: kputchar(is_shift ? '"' : '\''); break;
    case 0x29: kputchar(is_shift ? '~' : '`'); break;
    case 0x2B: kputchar(is_shift ? '|' : '\\'); break;
    case 0x33: kputchar(is_shift ? '<' : ','); break;
    case 0x34: kputchar(is_shift ? '>' : '.'); break;
    case 0x35: kputchar(is_shift ? '?' : '/'); break;
    case 0x39: kputchar(' '); break;
    case 0x1C: kputchar('\n'); break;
    case 0x0C: kputchar(is_shift ? '_' : '-'); break;
    case 0x0D: kputchar(is_shift ? '+' : '='); break;
    case 0x0E: kputchar('\b'); break;
    case 0x2A: lshift = true; break;
    case 0x36: rshift = true; break;
    case 0xAA: lshift = false; break;
    case 0xB6: rshift = false; break;
    case 0x3A: caps_lock = !caps_lock; break;
    default: break;
    }
    // clang-format on

    outb(0x20, 0x20); // End of interrupt (EOI) for master PIC
    outb(0xA0, 0x20); // End of interrupt (EOI) for slave PIC
}

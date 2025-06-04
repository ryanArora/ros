#include <mm/mm.h>
#include <mm/pfa.h>
#include <mm/slab.h>

void
mm_init(void)
{
    pfa_init();
    slab_init();
}

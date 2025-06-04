#pragma once

#include <mm/pfa.h>
#include <mm/slab.h>

struct mm {
    struct pfa pfa;
    struct slab slab;
};

void mm_init(void);

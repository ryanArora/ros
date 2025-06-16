#pragma once

#include <stdio.h>

#define assert(cond)                                                           \
    do {                                                                       \
        if (!(cond)) {                                                         \
            fprintf(STDERR, "Assertion failed: %s\n", #cond);                  \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

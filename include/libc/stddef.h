#pragma once

#define NULL ((void*)0)

typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __WCHAR_TYPE__ wchar_t;

#define offsetof(st, m) __builtin_offsetof(st, m)

#define container_of(ptr, type, member)                                        \
    ({                                                                         \
        const typeof(((type*)0)->member)* __mptr = (ptr);                      \
        (type*)((char*)__mptr - offsetof(type, member));                       \
    })

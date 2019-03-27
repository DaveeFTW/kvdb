#pragma once

#include <string.h>

template<typename T>
bool begins_with(const char *a, T &&str)
{
    return __builtin_memcmp(a, str, sizeof(str)-1) == 0;
}

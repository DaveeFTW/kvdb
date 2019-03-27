#pragma once

#include <cstdint>

namespace hex
{
    unsigned int from_char(int ch);
    int to_char(unsigned int hex);
    void to_string(char *dst, const char *src, std::size_t length);
    void from_string(char *dst, const char *src);
}
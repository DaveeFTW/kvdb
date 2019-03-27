#pragma once

#include <cstdint>

namespace rsp
{
    int read(char *out, std::size_t size);
    int write(const char *pkt, std::size_t size);
}

#pragma once

#include <cstdint>

namespace pipe
{
    void init();
    void use();
    int copyin(std::uintptr_t udata, std::size_t size);
    int copyout(std::uintptr_t udata, std::size_t max_size, int timeout);
}

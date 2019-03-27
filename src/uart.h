#pragma once

namespace uart
{
    void init();
    void use();

    int printf(const char *fmt, ...);
}

#include "hex.h"
#include <string.h>

#include "log.h"

namespace hex
{
    unsigned int from_char(int ch)
    {
        if (ch >= '0' && ch <= '9')
        {
            return ch - '0';
        }

        else if (ch >= 'a' && ch <= 'f')
        {
            return (ch - 'a') + 0xA;
        }

        else if (ch >= 'A' && ch <= 'F')
        {
            return (ch - 'A') + 0xA;
        }

        // TODO: handle this?
        return -1;
    }

    int to_char(unsigned int hex)
    {
        auto digits = "0123456789abcdef";
        return digits[hex & 0xF];
    }

    void to_string(char *dst, const char *src, std::size_t length)
    {
        for (auto i = 0u; i < length; ++i)
        {
            *dst++ = to_char(src[i] >> 4);
            *dst++ = to_char(src[i] & 0xF);
        }

        *dst = 0;
    }

    void from_string(char *dst, const char *src)
    {
        auto length = strlen(src);

        for (auto i = 0u; i < length; ++i)
        {
            dst[i] = (from_char(src[2*i]) << 4) | from_char(src[2*i+1]);
        }
    }
}
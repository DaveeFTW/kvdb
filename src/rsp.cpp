#include "rsp.h"
#include "serial.h"
#include "hex.h"
#include "log.h"

#include <psp2kern/kernel/threadmgr.h>

namespace
{
    static constexpr auto RSP_START_TOKEN      = '$';
    static constexpr auto RSP_END_TOKEN        = '#';
    static constexpr auto RSP_ESCAPE_TOKEN     = '}';
    static constexpr auto RSP_RUNLENGTH_TOKEN  = '*';
    static constexpr auto RSP_ACCEPT           = '+';
    static constexpr auto RSP_DECLINE          = '-';


    int unescape(int token)
    {
        if (token != RSP_ESCAPE_TOKEN)
        {
            return token;
        }

        return serial::get() ^ 0x20;
    }

    int escape(int ch)
    {
        return ch ^ 0x20;
    }

    int is_token(int ch)
    {
        return (ch == RSP_START_TOKEN
            || ch == RSP_END_TOKEN
            || ch == RSP_ESCAPE_TOKEN
            || ch == RSP_RUNLENGTH_TOKEN);
    }
}

namespace rsp
{
    int read(char *out, std::size_t size)
    {
        while (serial::get() != RSP_START_TOKEN)
        {
            // we need to yield so other services can run still
            ksceKernelDelayThread(100);
        }

        auto checksum = 0u;

        for (auto i = 0u; i < size; ++i)
        {
            auto token = serial::get();

            if (token == RSP_END_TOKEN)
            {
                auto upper = hex::from_char(serial::get()) << 4;
                auto lower = hex::from_char(serial::get());

                // checksum is mod 256 of sum of all data
                if ((checksum & 0xFF) != (upper | lower))
                {
                    // TODO: handle error
                    LOG("rsp: bad checksum: 0x%02X != 0x%02X\n", checksum & 0xFF, upper | lower);
                    break;
                }

                // null terminate cstring
                out[i] = '\0';
                serial::put(RSP_ACCEPT);

                LOG("rsp < (\"%s\"): 0x%08X\n", out, i);
                return 0;
            }

            checksum += token;
            out[i] = unescape(token);
        }

        serial::put(RSP_DECLINE);
        return -1;
    }

    int write(const char *data, std::size_t size)
    {
        LOG("rsp > (\"%s\"): 0x%08X\n", data, size);

        serial::put(RSP_START_TOKEN);

        auto checksum = 0u;

        for (auto i = 0u; i < size; ++i)
        {
            auto ch = data[i];

            // check for any reserved tokens
            if (is_token(ch))
            {
                serial::put(RSP_ESCAPE_TOKEN);
                checksum += RSP_ESCAPE_TOKEN;
                ch = escape(ch);
            }

            serial::put(ch);
            checksum += ch;
        }

        serial::put(RSP_END_TOKEN);

        // checksum is mod 256
        checksum &= 0xFF;
        serial::put(hex::to_char((checksum >> 4) & 0xF));
        serial::put(hex::to_char(checksum & 0xF));
        return 0;
    }
}
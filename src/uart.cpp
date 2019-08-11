#include "uart.h"
#include "serial.h"
#include "serial_impl.h"

#include <psp2kern/uart.h>
#include <psp2kern/lowio/pervasive.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace
{
    template<int port>
    class Uart : public SerialImpl
    {
    public:
        Uart()
        {
            kscePervasiveUartClockEnable(port);
            kscePervasiveUartResetDisable(port);
            ksceUartInit(port);
        }

        int get() override
        {
            if (!ksceUartReadAvailable(port))
            {
                return -1;
            }

            return ksceUartRead(port);
        }

        void put(int ch) override
        {
            if (ch == '\n')
            {
                ksceUartWrite(port, '\r');
            }

            ksceUartWrite(port, ch);
        }
    };

    template <int port>
    Uart<port> *get()
    {
        static Uart<port> uart;
        return &uart;
    }
}

namespace uart
{
    void init()
    {
        // we just use get to init port 0
        get<0>();
    }

    void use()
    {
        serial::set_device(get<0>());
    }

    int printf(const char *fmt, ...)
    {
        va_list args;
        static char buffer[0x1000];

        va_start(args, fmt);
        vsprintf(buffer, fmt, args);
        va_end(args);

        auto len = strlen(buffer);
        auto uart = get<0>();

        for (auto i = 0u; i < len; ++i)
        {
            uart->put(buffer[i]);
        }

        return 0;
    }
}

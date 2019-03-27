#include "serial.h"
#include "serial_impl.h"
#include "log.h"
#include <psp2kern/kernel/threadmgr.h>

namespace
{
    SerialImpl **device_ptr()
    {
        static SerialImpl *device = nullptr;
        return &device;
    }

    SerialImpl *device()
    {
        return *device_ptr();
    }
}

namespace serial
{
    void set_device(SerialImpl *device)
    {
        *device_ptr() = device;
    }

    int get()
    {
        while (1)
        {
            auto ch = device()->get();

            if (ch >= 0)
            {
                return ch;
            }

            ksceKernelDelayThread(10);
        }
    }

    int poll()
    {
        return device()->get();
    }

    void put(int ch)
    {
        device()->put(ch);
    }
}

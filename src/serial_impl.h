#pragma once

class SerialImpl
{
public:
    virtual int get() = 0;
    virtual void put(int ch) = 0;
};

namespace serial
{
    namespace impl
    {
        extern SerialImpl *g_serial_device;
    }
}

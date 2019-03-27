#pragma once

class SerialImpl;

namespace serial
{
    void set_device(SerialImpl *device);
    int get();
    void put(int ch);
    int poll();
}

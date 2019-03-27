#pragma once

#include <cstdint>
#include <string.h>

struct Packet
{
private:
    static constexpr std::size_t m_packet_size = 0x1000;

public:
    Packet()
    {
        reset();
    }

    void reset()
    {
        recv_buf[0] = '\0';
        send_buf[0] = '\0';
    }

    void send(const char *str)
    {
        strncat(send_buf, str, size());
    }

    constexpr std::size_t size() const { return m_packet_size; }

    char recv_buf[m_packet_size+1];
    char send_buf[m_packet_size+1]; 
};

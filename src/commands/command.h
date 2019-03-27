#pragma once

struct Packet;

class Command
{
public:
    virtual bool is(const Packet *packet) const = 0;
    virtual int execute(Packet *packet) = 0;
};

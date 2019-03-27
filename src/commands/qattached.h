#pragma once

#include "command.h"

class qAttachedCommand : public Command
{
public:
    bool is(const Packet *packet) const override;
    int execute(Packet *packet) override;
};

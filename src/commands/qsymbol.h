#pragma once

#include "command.h"

class qSymbolCommand : public Command
{
public:
    bool is(const Packet *packet) const override;
    int execute(Packet *packet) override;
};

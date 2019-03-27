#pragma once

#include "command.h"

class qXferCommand : public Command
{
public:
    bool is(const Packet *packet) const override;
    int execute(Packet *packet) override;

private:
    void analyseFeatures(Packet *packet);
};

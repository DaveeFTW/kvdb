#pragma once

#include "command.h"

class qSupportedCommand : public Command
{
public:
    bool is(const Packet *packet) const override;
    int execute(Packet *packet) override;

private:
    void analyseFeatures(Packet *packet);
    void xmlRegistersHandler();
};

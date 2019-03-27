#pragma once

#include "command.h"

class Debugger;

class StepCommand : public Command
{
public:
    StepCommand(Debugger *debugger);
    bool is(const Packet *packet) const override;
    int execute(Packet *packet) override;

private:
    unsigned int next_address(unsigned int current_address);
    
private:
    Debugger * const m_debugger = nullptr;
};

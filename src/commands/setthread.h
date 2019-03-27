#pragma once

#include "command.h"

class Debugger;

class SetThreadCommand : public Command
{
public:
    SetThreadCommand(Debugger *debugger);
    bool is(const Packet *packet) const override;
    int execute(Packet *packet) override;

private:
    Debugger * const m_debugger = nullptr;
};

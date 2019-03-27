#pragma once

#include "command.h"

class Debugger;

class ReadRegisterCommand : public Command
{
public:
    ReadRegisterCommand(Debugger *debugger);
    bool is(const Packet *packet) const override;
    int execute(Packet *packet) override;

private:
    Debugger * const m_debugger = nullptr;
};

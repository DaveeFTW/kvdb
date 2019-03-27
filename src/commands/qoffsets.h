#pragma once

#include "command.h"

class Debugger;

class qOffsetsCommand : public Command
{
public:
    qOffsetsCommand(Debugger *debugger);
    bool is(const Packet *packet) const override;
    int execute(Packet *packet) override;

private:
    Debugger * const m_debugger = nullptr;
};

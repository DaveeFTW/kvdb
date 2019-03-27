#include "readregister.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"

#include <psp2kern/kernel/threadmgr.h>

#include <stdio.h>
#include <stdlib.h>

ReadRegisterCommand::ReadRegisterCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool ReadRegisterCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == 'p';
}

int ReadRegisterCommand::execute(Packet *packet)
{
    auto target = m_debugger->target();

    // TODO: check for errors
    ThreadCpuRegisters register_sets;
    ksceKernelGetThreadCpuRegisters(target->excpt_tid, &register_sets);

    auto registers = (register_sets.user.cpsr & 0x1F) == 0x10 ? (&register_sets.user) : (&register_sets.kernel);
    auto reg_number = strtoul(packet->recv_buf+1, nullptr, 16);
    auto value = 0;

    switch (reg_number)
    {
        // TODO: find out enum
    case 25:
        // CPSR
        value = registers->cpsr;
        break;
    default:
        value = 0;
        break;
    }

    snprintf(packet->send_buf, packet->size(), "%08x", __builtin_bswap32(value));
    return 0;
}

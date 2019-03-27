#include "writememory.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"
#include "hex.h"

#include <psp2kern/kernel/sysmem.h>

#include <stdlib.h>

WriteMemoryCommand::WriteMemoryCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool WriteMemoryCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == 'M';
}

int WriteMemoryCommand::execute(Packet *packet)
{
    auto target = m_debugger->target();

    // TODO: probably should check these
    auto addr = strtoul(packet->recv_buf+1, nullptr, 16);
    auto length = strtoul(strchr(packet->recv_buf, ',')+1, nullptr, 16);
    auto hex = strchr(packet->recv_buf, ':') + 1;

    hex::from_string(packet->recv_buf, hex);

    // TODO: check if dest write location is valid
    // TODO: check result
    ksceKernelRxMemcpyKernelToUserForPid(target->pid, addr, packet->recv_buf, length);

    packet->send("OK");
    return 0;
}

#include "setthread.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"

#include <stdlib.h>

SetThreadCommand::SetThreadCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool SetThreadCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == 'H';
}

int SetThreadCommand::execute(Packet *packet)
{
    auto target = m_debugger->target();
    auto op = packet->recv_buf[1];
    auto thread_id = strtol(packet->recv_buf+2, nullptr, 16);

    if (thread_id != -1 && thread_id != 0)
    {
        target->excpt_tid = thread_id;
    }

    packet->send("OK");
    return 0;
}

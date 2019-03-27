#include "getthread.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"
#include "utils.h"

#include <stdio.h>

GetThreadCommand::GetThreadCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool GetThreadCommand::is(const Packet *packet) const
{
    return begins_with(packet->recv_buf, "qC");
}

int GetThreadCommand::execute(Packet *packet)
{
    auto target = m_debugger->target();
    snprintf(packet->send_buf, packet->size(), "QC%x", target->excpt_tid);
    return 0;
}

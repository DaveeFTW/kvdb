#include "kill.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"

#include <psp2kern/appmgr.h>
#include <psp2kern/kernel/processmgr.h>
#include <psp2kern/kernel/threadmgr.h>

KillCommand::KillCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool KillCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == 'k';
}

int KillCommand::execute(Packet *packet)
{
    auto target = m_debugger->target();

    // TODO: handle errors
    ksceAppMgrKillProcess(target->pid);

    // TODO: signal vdb to notify debugger that connection is closed
    return 0;
}

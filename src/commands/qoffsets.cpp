#include "qoffsets.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"
#include "utils.h"

#include <psp2kern/kernel/modulemgr.h>

#include <stdio.h>

qOffsetsCommand::qOffsetsCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool qOffsetsCommand::is(const Packet *packet) const
{
    return begins_with(packet->recv_buf, "qOffsets");
}

int qOffsetsCommand::execute(Packet *packet)
{
    auto target = m_debugger->target();

    SceKernelModuleInfo info;
    info.size = sizeof(info);

    // TODO: check for errors
    ksceKernelGetModuleInfo(target->pid, target->main_module_id, &info);

    snprintf(packet->send_buf, packet->size(), "TextSeg=%08x;DataSeg=%08x", info.segments[0].vaddr, info.segments[1].vaddr);
    return 0;
}

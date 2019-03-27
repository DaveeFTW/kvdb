#include "threadinfo.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"
#include "utils.h"

#include <psp2kern/kernel/threadmgr.h>

#include <stdio.h>

ThreadInfoCommand::ThreadInfoCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool ThreadInfoCommand::is(const Packet *packet) const
{
    return begins_with(packet->recv_buf, "qfThreadInfo");
}

int ThreadInfoCommand::execute(Packet *packet)
{
    int thread_ids[128];
    auto target = m_debugger->target();

    // TODO: check for errors
    // TODO: check for < 0 and > 128
    auto count = ksceKernelGetThreadIdList(target->pid, nullptr, 0, nullptr);

    auto copied = count;
    ksceKernelGetThreadIdList(target->pid, thread_ids, count*sizeof(int), &copied);

    packet->send("m");

    for (auto i = 0; i < count; ++i)
    {
        char thread_str[10];
        snprintf(thread_str, sizeof(thread_str), (i == 0) ? ("%x") : (",%x"), thread_ids[i]);
        packet->send(thread_str);
    }

    packet->send("l");
    return 0;
}

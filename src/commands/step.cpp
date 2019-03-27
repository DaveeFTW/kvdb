#include "step.h"
#include "packet.h"
#include "debugger.h"

#include <psp2kern/kernel/threadmgr.h>

#include <stdlib.h>
#include <string.h>

StepCommand::StepCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool StepCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == 's';
}

int StepCommand::execute(Packet *packet)
{
    auto addr = 0u;

    if (strlen(packet->recv_buf) > 1)
    {
        addr = strtoul(packet->recv_buf+1, nullptr, 16);
    }

    auto target = m_debugger->target();
    ThreadCpuRegisters register_sets;
    ksceKernelGetThreadCpuRegisters(target->excpt_tid, &register_sets);

    auto registers = (register_sets.user.cpsr & 0x1F) == 0x10 ? (&register_sets.user) : (&register_sets.kernel);

    auto pc_addr = registers->pc;
    auto is_thumb = (registers->cpsr & (1 << 5)) != 0;

    if (is_thumb)
    {
        // TODO: error check better
        auto instruction = 0u;
        auto res = ksceKernelMemcpyUserToKernelForPid(target->pid, &instruction, pc_addr, 2);

        if (res < 0)
        {
            packet->send("E0B");
            return res;
        }
    }
    else
    {
        // TODO: error check better
        auto instruction = 0u;
        auto res = ksceKernelMemcpyUserToKernelForPid(target->pid, &instruction, pc_addr, 4);

        if (res < 0)
        {
            packet->send("E0B");
            return res;
        }


    }
    

    return 0;
}

unsigned int StepCommand::next_address(unsigned int current_address)
{
    return 0; 
}
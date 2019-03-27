#include "readregisters.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"
#include <psp2kern/kernel/threadmgr.h>

#include <stdio.h>

ReadRegistersCommand::ReadRegistersCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool ReadRegistersCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == 'g';
}

int ReadRegistersCommand::execute(Packet *packet)
{
    ThreadCpuRegisters register_set;
    auto target = m_debugger->target();

    ksceKernelGetThreadCpuRegisters(target->excpt_tid, &register_set); // TODO check returns

    auto registers = (register_set.user.cpsr & 0x1F) == 0x10 ? (&register_set.user) : (&register_set.kernel);

    sprintf(packet->send_buf, "%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x%08x"
            , (unsigned int)__builtin_bswap32(registers->r0)
            , (unsigned int)__builtin_bswap32(registers->r1)
            , (unsigned int)__builtin_bswap32(registers->r2)
            , (unsigned int)__builtin_bswap32(registers->r3)
            , (unsigned int)__builtin_bswap32(registers->r4)
            , (unsigned int)__builtin_bswap32(registers->r5)
            , (unsigned int)__builtin_bswap32(registers->r6)
            , (unsigned int)__builtin_bswap32(registers->r7)
            , (unsigned int)__builtin_bswap32(registers->r8)
            , (unsigned int)__builtin_bswap32(registers->r9)
            , (unsigned int)__builtin_bswap32(registers->r10)
            , (unsigned int)__builtin_bswap32(registers->r11)
            , (unsigned int)__builtin_bswap32(registers->r12)
            , (unsigned int)__builtin_bswap32(registers->sp)
            , (unsigned int)__builtin_bswap32(registers->lr)
            , (unsigned int)__builtin_bswap32(registers->pc)
    );

    return 0;
}

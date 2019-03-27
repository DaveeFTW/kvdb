#include "resume.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"
#include "serial.h"

#include "hex.h"
#include "rsp.h"
#include "log.h"

#include <psp2kern/kernel/processmgr.h>
#include <psp2kern/kernel/threadmgr.h>

ResumeCommand::ResumeCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool ResumeCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == 'c';
}

int ResumeCommand::execute(Packet *packet)
{
    auto target = m_debugger->target();

    m_debugger->resume(); // TODO: CHECK ERRORS
    
    // we're NOT non-stop, we need to wait for signal
    while (m_debugger->pollSignal(Debugger::Signal::BKPT) < 0)
    {
        // check for CTRL+C
        if (serial::poll() == 0x03)
        {
            m_debugger->halt();
            break;
        }

        ksceKernelDelayThread(100);
    }

    LOG("sending exit resume msg\n");
    packet->send("T05");
    return 0;
}

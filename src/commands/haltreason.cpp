#include "haltreason.h"
#include "packet.h"
#include "debugger.h"

HaltReasonCommand::HaltReasonCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool HaltReasonCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == '?';
}

int HaltReasonCommand::execute(Packet *packet)
{
    // we're NOT non-stop, so we halt the device for this
    m_debugger->halt(); // TODO: check errors

    // TODO: provide proper information here...
    packet->send("T00");
    return 0;
}

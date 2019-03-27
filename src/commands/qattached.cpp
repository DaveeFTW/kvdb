#include "qattached.h"
#include "packet.h"
#include "utils.h"

bool qAttachedCommand::is(const Packet *packet) const
{
    return begins_with(packet->recv_buf, "qAttached");
}

int qAttachedCommand::execute(Packet *packet)
{
    packet->send("0");
    return 0;
}

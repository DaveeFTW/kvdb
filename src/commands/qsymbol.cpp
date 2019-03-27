#include "qsymbol.h"
#include "packet.h"
#include "utils.h"

bool qSymbolCommand::is(const Packet *packet) const
{
    return begins_with(packet->recv_buf, "qSymbol");
}

int qSymbolCommand::execute(Packet *packet)
{
    packet->send("OK");
    return 0;
}

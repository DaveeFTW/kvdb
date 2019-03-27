#include "qxfer.h"
#include "packet.h"
#include "utils.h"
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


bool qXferCommand::is(const Packet *packet) const
{
    return begins_with(packet->recv_buf, "qXfer");
}

int qXferCommand::execute(Packet *packet)
{
    analyseFeatures(packet);
    return 0;
}

void qXferCommand::analyseFeatures(Packet *packet)
{
    static constexpr auto max_tokens = 4;
    char *tokens[max_tokens];

    if (strlen(packet->recv_buf) < strlen("qXfer:"))
    {
        return;
    }

    auto next = strtok(packet->recv_buf+strlen("qXfer:"), ":");

    auto token_n = 0;
    while (token_n < max_tokens && next)
    {
        tokens[token_n++] = next;
        next = strtok(nullptr, ":");
    }

    // TODO: in the future we can do something more dynamic
    // for now just hardcode the values we're interested in
    if (token_n != 4)
    {
        LOG("got token count: %i\n", token_n);
        packet->send("E00");
        return;
    }

    if (strcmp(tokens[0], "features")  == 0 && strcmp(tokens[1], "read") == 0)
    {
        auto annex = "target.xml";
        auto data = "<?xml version=\"1.0\"?><!DOCTYPE target SYSTEM \"gdb-target.dtd\"><target version=\"1.0\"><architecture>arm</architecture><abi>AAPCS</abi></target>";
        
        // we only handle this one annex
        if (strcmp(annex, tokens[2]) != 0)
        {
            return;
        }

        // TODO: if no ',' then crash. please fix
        auto offset = strtoul(tokens[3], nullptr, 16);
        auto size = strtoul(strchr(tokens[3], ',')+1, nullptr, 16);
        auto data_size = strlen(data);
        auto response = 'l';

        if (offset > data_size)
        {
            // EINVAL
            packet->send("E16");
            return;
        }

        if (offset+size >= data_size)
        {
            size = data_size - offset;
            response = 'l';
        }
        else
        {
            // more data to be read
            response = 'm';
        }
        
        snprintf(packet->send_buf, packet->size(), "%c%s", response, data+offset);
    }
}

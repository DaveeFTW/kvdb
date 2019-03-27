#include "qsupported.h"
#include "packet.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

namespace 
{
    template <typename T = const char *>
    struct Feature
    {
        enum Type
        {
            SUPPORTED_VALUE,
            SUPPORTED,
            NOT_SUPPORTED,
            MAYBE_SUPPORTED
        };

        Feature(char *feature)
        {
            auto assignment = strchr(feature, '=');
            auto len = strlen(feature);

            if (assignment)
            {
                // split the string
                *assignment = '\0';
                type = SUPPORTED_VALUE;
                name = feature;
                value = assignment+1;
            }

            else if (feature[len-1] == '+')
            {
                type = SUPPORTED;
                name = feature;
                feature[len] = '\0';
            }

            else if (feature[len-1] == '-')
            {
                type = NOT_SUPPORTED;
                name = feature;
                feature[len] = '\0';
            }

            else if (feature[len-1] == '?')
            {
                type = MAYBE_SUPPORTED;
                name = feature;
                feature[len] = '\0';
            }
        }

        Feature(const char *feature, T val)
        {
            type = SUPPORTED_VALUE;
            name = feature;
            value = val;
        }

        Feature(const char *feature, Type typ = SUPPORTED)
        {
            type = typ;
            name = feature;
        }

        void toString(char *out, std::size_t len)
        {
            if (type == SUPPORTED_VALUE)
            {
                convert_value(out, len);
            }
            else
            {
                char type_ch = '-';

                switch (type)
                {
                case SUPPORTED:
                    type_ch = '+';
                    break;
                case NOT_SUPPORTED:
                    type_ch = '-';
                    break;
                case MAYBE_SUPPORTED:
                    type_ch = '?';
                    break;
                default:
                    // TODO: handle?
                    break;
                }

                snprintf(out, len, "%s%c", name, type_ch);
            }
        }

        Type type;
        const char *name;
        T value;

    private:
        void convert_value(char *out, std::size_t len);
    };

    template <>
    void Feature<const char *>::convert_value(char *out, std::size_t len)
    {
        snprintf(out, len, "%s=%s", name, value);
    }

    template <>
    void Feature<std::size_t>::convert_value(char *out, std::size_t len)
    {
        snprintf(out, len, "%s=%u", name, value);
    }
}

bool qSupportedCommand::is(const Packet *packet) const
{
    return begins_with(packet->recv_buf, "qSupported");
}

int qSupportedCommand::execute(Packet *packet)
{
    char work[128];
    Feature<std::size_t> packet_size("PacketSize", packet->size());
    packet_size.toString(work, sizeof(work));
    packet->send(work);

    analyseFeatures(packet);

    Feature<const char *> qxfer_read("qXfer:features:read");
    qxfer_read.toString(work, sizeof(work));
    packet->send(";");
    packet->send(work);
    return 0;
}

void qSupportedCommand::analyseFeatures(Packet *packet)
{
    if (strlen(packet->recv_buf) < strlen("qSupported:"))
    {
        return;
    }

    auto next = strtok(packet->recv_buf+strlen("qSupported:"), ";");

    while (next)
    {
        Feature<const char *> feature(next);

        if (strcmp(feature.name, "xmlRegisters") == 0)
        {
            xmlRegistersHandler();
        }

        next = strtok(nullptr, ";");
    }
}

void qSupportedCommand::xmlRegistersHandler()
{

}

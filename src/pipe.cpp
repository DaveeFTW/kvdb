#include "pipe.h"
#include "serial.h"
#include "serial_impl.h"
#include "eventflag.h"
#include "pipecache.h"

#include "log.h"

#include <psp2kern/kernel/threadmgr.h>

namespace
{
    class Pipe : public SerialImpl
    {
        enum Op
        {
            Get = (1u << 0),
            Put = (1u << 1),
            GetComplete = (1u << 2),
            PutComplete = (1u << 3),
        };

    public:
        Pipe()
            : m_flag("vdb-pipe-flag")
            , m_tx("vdb-pipe-tx")
            , m_rx("vdb-pipe-rx")
        {
        }

        int get() override
        {
            if (!m_rx_len)
            {
                MsgPipeRecvData data;
                data.message = m_rx_buf;
                data.size = sizeof(m_rx_buf);
                m_rx_ptr = m_rx_buf;

                // timeout of 1 to peek
                m_rx_msg = m_rx.read(m_rx_buf, sizeof(m_rx_buf), 1);

                if (m_rx_msg < 0)
                {
                    return m_rx_msg;
                }

                if (!m_rx_len)
                {
                    return -1;
                }
            }

            --m_rx_len;
            auto r = *m_rx_ptr++;
            return r;
        }

        void put(int ch) override
        {
            m_tx_msg = ch;
            //LOG("req put\n");
            MsgPipeSendData data;
            data.message = &m_tx_msg;
            data.size = 1;

            // TODO: check return
            m_tx.write(reinterpret_cast<const char *>(&m_tx_msg), 1);
            //LOG("wait put done\n");
        }

        int copyin(std::uintptr_t udata, std::size_t size)
        {
            return m_rx.copyin(udata, size);
        }

        int copyout(std::uintptr_t udata, std::size_t max_size, int timeout)
        {
            return m_tx.copyout(udata, max_size, timeout);
        }

    private:
        char m_rx_buf[0x2000];
        int m_rx_len = 0;
        char *m_rx_ptr = m_rx_buf;
        int m_tx_msg = 0;
        int m_rx_msg = 0;
        EventFlag m_flag;
        PipeCache m_tx;
        PipeCache m_rx;
    };

    Pipe *get()
    {
        static Pipe pipe;
        return &pipe;
    }
}

namespace pipe
{
    void init()
    {
        // just call this to init
        get();
    }

    void use()
    {
        serial::set_device(get());
    }

    int copyin(std::uintptr_t udata, std::size_t size)
    {
        return get()->copyin(udata, size);
    }

    int copyout(std::uintptr_t udata, std::size_t max_size, int timeout)
    {
        return get()->copyout(udata, max_size, timeout);
    }
}

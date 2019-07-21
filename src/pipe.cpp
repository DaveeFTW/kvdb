#include "pipe.h"
#include "serial.h"
#include "serial_impl.h"
#include "eventflag.h"

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
        {
            // open a shared pipe for receiving data
            m_rx = ksceKernelCreateMsgPipe("vdb-pipe-rx", 0, 4 | 8 | 0x80, 0x2000, nullptr);

            // open a shared pipe for sending data
            m_tx = ksceKernelCreateMsgPipe("vdb-pipe-tx", 0, 4 | 8 | 0x80, 0x2000, nullptr);

            // create a thread to call the msg pipe API
            auto thid = ksceKernelCreateThread("kvdb", pipe_thread_entry, 0x40, 0x1000, 0, 0, nullptr);
            auto class_ptr = this;
            //ksceKernelStartThread(thid, sizeof(class_ptr), &class_ptr);
        }

        int get() override
        {
            if (!m_rx_len)
            {
                MsgPipeRecvData data;
                data.message = m_rx_buf;
                data.size = sizeof(m_rx_buf);
                m_rx_ptr = m_rx_buf;

                // TODO: check return
                auto timeout = 0u;
                m_rx_msg = ksceKernelReceiveMsgPipeVector(m_rx, &data, 1, 0, &m_rx_len, &timeout);

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
            auto res = ksceKernelSendMsgPipeVector(m_tx, &data, 1, 1, nullptr, nullptr);
            
            //LOG("wait put done\n");
        }

    private:
        void io_loop()
        {
            while (1)
            {
                auto op = 0u;
                m_flag.waitForAny(Op::Get | Op::Put, &op);

                if (op & Op::Get)
                {
                    MsgPipeRecvData data;
                    data.message = m_rx_buf;
                    data.size = sizeof(m_rx_buf);
                    m_rx_ptr = m_rx_buf;

                    // TODO: check return
                    auto timeout = 0u;
                    m_rx_msg = ksceKernelReceiveMsgPipeVector(m_rx, &data, 1, 0, &m_rx_len, &timeout);
                    m_flag.set(Op::GetComplete);
                }
                
                if (op & Op::Put)
                {
                    MsgPipeSendData data;
                    data.message = &m_tx_msg;
                    data.size = 1;

                    // TODO: check return
                    auto res = ksceKernelSendMsgPipeVector(m_tx, &data, 1, 1, nullptr, nullptr);
                    //LOG("writing pipe data: 0x%08X\n", res);
                    m_flag.set(Op::PutComplete);
                }
            }
        }

        static int pipe_thread_entry(SceSize args, void *argp)
        {
            auto class_ptr = reinterpret_cast<Pipe **>(argp);
            (*class_ptr)->io_loop();
            return 0;
        }

        SceUID m_rx = -1;
        SceUID m_tx = -1;
        SceUID m_rx_mtx = -1;
        SceUID m_tx_mtx = -1;
        char m_rx_buf[0x2000];
        int m_rx_len = 0;
        char *m_rx_ptr = m_rx_buf;
        int m_tx_msg = 0;
        int m_rx_msg = 0;
        EventFlag m_flag;
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
}

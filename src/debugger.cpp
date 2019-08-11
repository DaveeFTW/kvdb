#include "debugger.h"
#include "packet.h"
#include "rsp.h"
#include "hex.h"
#include "log.h"

// commands
#include "commands/getthread.h"
#include "commands/haltreason.h"
#include "commands/kill.h"
#include "commands/qattached.h"
#include "commands/qoffsets.h"
#include "commands/qsupported.h"
#include "commands/qsymbol.h"
#include "commands/qxfer.h"
#include "commands/readmemory.h"
#include "commands/readregister.h"
#include "commands/readregisters.h"
#include "commands/resume.h"
#include "commands/setthread.h"
#include "commands/step.h"
#include "commands/threadinfo.h"
#include "commands/writememory.h"

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/processmgr.h>

namespace
{
    template <typename T>
    Command *static_command()
    {
        static T command;
        return &command;
    }

    template <typename T>
    Command *static_command(Debugger *debugger)
    {
        static T command(debugger);
        return &command;
    }

    int dummy_handler()
    {
        return 0;
    }
}

Debugger *Debugger::instance()
{
    static Debugger Debugger;
    return &Debugger;
}

Debugger::Debugger()
    : m_state_flag("kvdb-state-flag")
{
    static Command * const g_cmd_list[] =
    {
        static_command<GetThreadCommand>(this),
        static_command<HaltReasonCommand>(this),
        static_command<KillCommand>(this),
        static_command<qAttachedCommand>(),
        static_command<qOffsetsCommand>(this),
        static_command<qSupportedCommand>(),
        static_command<qSymbolCommand>(),
        static_command<qXferCommand>(),
        static_command<ReadMemoryCommand>(this),
        static_command<ReadRegisterCommand>(this),
        static_command<ReadRegistersCommand>(this),
        static_command<ResumeCommand>(this),
        static_command<SetThreadCommand>(this),
        //static_command<StepCommand>(this),
        static_command<ThreadInfoCommand>(this),
        static_command<WriteMemoryCommand>(this)
    };

    constexpr std::size_t g_cmd_n = sizeof(g_cmd_list)/sizeof(Command *);

    m_cmd_list = g_cmd_list;
    m_cmd_list_n = g_cmd_n;

    m_proc_handler = 
    {
        sizeof(SceSysrootProcessHandler),
        ::dummy_handler,
        ::dummy_handler,
        ::dummy_handler,
        ::dummy_handler,
        ::dummy_handler,
        ::dummy_handler,
        on_process_created_handler,
        ::dummy_handler,
        ::dummy_handler
    };

    memset(m_stdout_cache, 0, sizeof(m_stdout_cache));

    ksceKernelSysrootSetProcessHandler(&m_proc_handler);
    ksceDebugRegisterPutcharHandler(&Debugger::putchar_handler, this);
}

void Debugger::signal(Signal signal)
{
    m_state_flag.set(static_cast<unsigned int>(signal));
}

int Debugger::waitForSignal(Signal signal, unsigned int *pattern)
{
    return m_state_flag.waitFor(static_cast<unsigned int>(signal), pattern);    
}

int Debugger::pollSignal(Signal signal, unsigned int *pattern)
{
    return m_state_flag.pollFor(static_cast<unsigned int>(signal), pattern);    
}

int Debugger::start()
{
    Packet packet;

    while (1)
    {
        packet.reset();
        while (rsp::read(packet.recv_buf, packet.size()) < 0);

        for (auto i = 0u; i < m_cmd_list_n; ++i)
        {
            if (m_cmd_list[i]->is(&packet))
            {
                m_cmd_list[i]->execute(&packet);
                break;
            }
        }

        rsp::write(packet.send_buf, strlen(packet.send_buf));
    }

    return 0;
}

void Debugger::attach(int pid)
{
    m_target.pid = pid;
    m_target.main_module_id = ksceKernelGetProcessMainModule(pid);
    m_target.main_thread_id = ksceKernelGetProcessMainThread(pid);
    m_target.excpt_tid = m_target.main_thread_id;
    m_attached = true;

    // we should probably check whether or not the target is halted
    // for now we assume that
    m_state = State::Halted;
}

bool Debugger::attached() const
{
    return m_attached;
}

bool Debugger::halt(int reason)
{
    auto status = 0;
    ksceKernelGetProcessStatus(m_target.pid, &status); // TODO: check returns

    if (status & 0x10)
    {
        // the process is already halted, if flags are set then return false to indicate
        // that state is consistent
        if (m_state == State::Halted)
        {
            return false;
        }
    }
    else
    {
        // process is not suspended, so we will suspend it
        ksceKernelSuspendProcess(m_target.pid, reason); // TODO: check returns
    }

    m_state = State::Halted;
    return true;
}

bool Debugger::isHalted() const
{
    return m_state == State::Halted;
}

bool Debugger::resume()
{
    if (m_state != State::Halted)
    {
        // we can only resume from the halted state
        return false;
    }

    // TODO: handle errors
    ksceKernelChangeThreadSuspendStatus(m_target.excpt_tid, 2);

    // TODO: handle errors
    ksceKernelResumeProcess(m_target.pid);

    m_state = State::Running;
    return true;
}

bool Debugger::isRunning() const
{
    return m_state == State::Running;
}

Target *Debugger::target()
{
    return &m_target;
}

int Debugger::on_process_created()
{
    signal(Signal::PROCESS_CREATED);
    return 0;
}

int Debugger::on_process_created_handler()
{
    return instance()->on_process_created();
}

int Debugger::on_putchar(char ch)
{
    if (m_state != State::Running)
    {
        return 0;
    }

    char hex_data[sizeof(m_stdout_cache)*2 + 2];

    m_stdout_cache[m_stdout_cache_indx++] = ch;

    // we do an output once cache is full or we meet a newline
    if (m_stdout_cache_indx == sizeof(m_stdout_cache) || ch == '\n')
    {
        hex_data[0] = 'O';
        hex::to_string(hex_data+1, m_stdout_cache, m_stdout_cache_indx);
        rsp::write(hex_data, strlen(hex_data));
        LOG("got text: \"%s\"\n", hex_data);
        m_stdout_cache_indx = 0;
    }

    return 0;
}

int Debugger::putchar_handler(void *arg, char ch)
{
    Debugger *debugger = reinterpret_cast<Debugger *>(arg);
    return debugger->on_putchar(ch);
}

namespace debugger
{
    Debugger *get()
    {
        return Debugger::instance();
    }
}

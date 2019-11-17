#include "exception.h"
#include "exception_asm.h"
#include "debugger.h"
#include "target.h"

#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/excpmgr.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/processmgr.h>

#include <cstdint>

#include "log.h"

namespace
{
    struct excp_saved_ctx 
    {
        std::uint32_t    r0;
        std::uint32_t    r1;
        std::uint32_t    r2;
        std::uint32_t    r3;
        std::uint32_t    r4;
        std::uint32_t    r5;
        std::uint32_t    r6;
        std::uint32_t    r7;
        std::uint32_t    r8;
        std::uint32_t    r9;
        std::uint32_t    r10;
        std::uint32_t    r11;
        std::uint32_t    r12;
        std::uint32_t    sp;
        std::uint32_t    lr;
        std::uint32_t    lr_abt;
        std::uint32_t    save_fp_regs;
        std::uint32_t    spsr;
        std::uint32_t    cpacr;
        std::uint32_t    fpscr;
        std::uint32_t    fpexc;
        std::uint32_t    contextidr;
        std::uint32_t    tpidrurw;
        std::uint32_t    tpidruro;
        std::uint32_t    tpidrprw;
        std::uint32_t    ttbr1;
        std::uint32_t    field_68;
        std::uint32_t    dacr;
        std::uint32_t    dfsr;
        std::uint32_t    ifsr;
        std::uint32_t    dfar;
        std::uint32_t    ifar;
        std::uint32_t    par;
        std::uint32_t    sys_p14_c1_c0_0;
        std::uint32_t    pmcr;
        std::uint32_t    pmcntenset;
        std::uint32_t    pmcntenset_2;
        std::uint32_t    pmselr;
        std::uint32_t    pmccntr;
        std::uint32_t    pmuserenr;
        std::uint32_t    pmxevtyper_0;
        std::uint32_t    pmxevcntr_0;
        std::uint32_t    pmxevtyper_1;
        std::uint32_t    pmxevcntr_1;
        std::uint32_t    pmxevtyper_2;
        std::uint32_t    pmxevcntr_2;
        std::uint32_t    pmxevtyper_3;
        std::uint32_t    pmxevcntr_3;
        std::uint32_t    pmxevtyper_4;
        std::uint32_t    pmxevcntr_4;
        std::uint32_t    pmxevtyper_5;
        std::uint32_t    pmxevcntr_5;
        std::uint32_t    field_D0;
        std::uint32_t    field_D4;
        std::uint32_t    dbgdscrint;
        std::uint32_t    unused[9];
        std::uint32_t    fp_regs[64];
        std::uint32_t    thread_ctx[128];
    };

    constexpr int EXCEPTION_HANDLED = 0;
    constexpr int EXCEPTION_NOT_HANDLED = 2;

    constexpr int IFSR_DEBUG_EVENT = 0x002;
}

namespace exception
{
    void register_handlers()
    {
        ksceExcpmgrRegisterHandler(SCE_EXCP_PABT, 5, reinterpret_cast<void *>(debugger_pabt_handler));
        ksceExcpmgrRegisterHandler(SCE_EXCP_DABT, 5, reinterpret_cast<void *>(debugger_dabt_handler));
        ksceExcpmgrRegisterHandler(SCE_EXCP_UNDEF_INSTRUCTION, 5, reinterpret_cast<void *>(debugger_und_handler));
    }
}

extern "C"
{
    std::uint32_t *g_abt_excp_count = nullptr;

    int insn_fault_triggered(excp_saved_ctx *ctx, std::uintptr_t lr, std::uintptr_t spsr)
    {
        LOG("insn_fault_triggered\n");
        auto debugger = debugger::get();

        if (!debugger->attached())
        {
            return EXCEPTION_NOT_HANDLED;
        }

        SceKernelFaultingProcessInfo info;
        ksceKernelGetFaultingProcess(&info); // TODO: check result
        g_abt_excp_count = reinterpret_cast<std::uint32_t *>(ksceExcpmgrGetData());

        auto code = ((ctx->ifsr & 0x400) >> 6) | (ctx->ifsr & 0xF);

        if (code == IFSR_DEBUG_EVENT)
        {
            auto target = debugger->target();

            // check if its our process
            if (target->pid != info.pid)
            {
                return EXCEPTION_NOT_HANDLED;
            }

            target->excpt_tid = info.unk;

            int status = 0;
            ksceKernelGetProcessStatus(target->pid, &status); // TODO: check result

            ksceKernelChangeThreadSuspendStatus(info.unk, 0x1002); // TODO: check result

            // lets suspend the process for further processing by gdb
            debugger->halt(0x1C);
            //ksceKernelSuspendProcess(target->pid, 0x1C); // TODO: check result

            // signal GDB that we have halted
            debugger->signal(Debugger::Signal::BKPT);

            //ksceKernelSetEventFlag(g_gdb_evid, GDB_SIGNAL_BKPT);

            return EXCEPTION_NOT_HANDLED;
        }
     
        return EXCEPTION_NOT_HANDLED;
    }

    int mem_fault_triggered(excp_saved_ctx *ctx, std::uintptr_t lr, std::uintptr_t spsr)
    {
        LOG("mem_fault_triggered\n");
        return 0;
    }

    int insn_undef_triggered(excp_saved_ctx *ctx, std::uintptr_t lr, std::uintptr_t spsr)
    {
        LOG("insn_undef_triggered\n");
        auto debugger = debugger::get();

        if (!debugger->attached())
        {
            return EXCEPTION_NOT_HANDLED;
        }

        SceKernelFaultingProcessInfo info;
        ksceKernelGetFaultingProcess(&info); // TODO: check result
        g_abt_excp_count = reinterpret_cast<std::uint32_t *>(ksceExcpmgrGetData());

        //auto code = ((ctx->ifsr & 0x400) >> 6) | (ctx->ifsr & 0xF);

       // if (code == IFSR_DEBUG_EVENT)
        {
            auto target = debugger->target();

            // check if its our process
            if (target->pid != info.pid)
            {
                return EXCEPTION_NOT_HANDLED;
            }

            target->excpt_tid = info.unk;

            int status = 0;
            ksceKernelGetProcessStatus(target->pid, &status); // TODO: check result

            ksceKernelChangeThreadSuspendStatus(info.unk, 0x1002); // TODO: check result

            // lets suspend the process for further processing by gdb
            debugger->halt(0x1C);
            //ksceKernelSuspendProcess(target->pid, 0x1C); // TODO: check result

            // signal GDB that we have halted
            debugger->signal(Debugger::Signal::BKPT);

            //ksceKernelSetEventFlag(g_gdb_evid, GDB_SIGNAL_BKPT);

            return EXCEPTION_NOT_HANDLED;
        }
    }
}

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

/* exceptions.c -- software breakpoint handler
 *
 * Copyright (C) 2018 molecule
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 
#include "exceptions.h"
#include "gdb.h"
#include "src/process.h"
#include "log.h"

#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/excpmgr.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>

#define IFSR_DEBUG_EVENT (0x002)
#define EXCEPTION_HANDLED (0)
#define EXCEPTION_NOT_HANDLED (2)

std::uint32_t *g_abt_excp_count;
extern void debugger_pabt_handler(excp_saved_ctx_t *ctx);
extern void debugger_dabt_handler(excp_saved_ctx_t *ctx);
extern void debugger_und_handler(excp_saved_ctx_t *ctx);
extern int g_kuser_evid;
extern int g_gdb_evid;

std::uint32_t debug_asm(std::uint32_t val) {
  LOG("val: 0x%08X", val);
  return val;
}

int ksceKernelProcessDebugSuspend(int pid, int reason);
int ksceKernelGetProcessStatus(int pid, int *status);
int ksceKernelChangeThreadDebugSuspendReason(int tid, int reason);

int insn_fault_triggered(excp_saved_ctx_t *ctx, uintptr_t lr, uintptr_t spsr)
{
    if (!is_attached())
    {
        return EXCEPTION_NOT_HANDLED;
    }

    SceKernelFaultingProcessInfo info;
    int code;
    int ret;
    int cpu;

    ret = ksceKernelGetFaultingProcess(&info);
    LOG("faulting process (ret:%x): 0x%x, 0x%x\n", ret, info.pid, info.unk);
    cpu = ksceKernelCpuGetCpuId();
    LOG("faulting cpu: %d\n", cpu);
    LOG("triggered, spsr: 0x%08X, lr: 0x%08X\n", spsr, lr);
    g_abt_excp_count = ksceExcpmgrGetData();
    LOG("unhandled abt count: %d\n", g_abt_excp_count[cpu]);

    code = ((ctx->ifsr & 0x400) >> 6) | (ctx->ifsr & 0xF);
    LOG("ifsr code: 0x%X\n", code);
    if (code == IFSR_DEBUG_EVENT)
    {
        TargetUnderDebug *target = get_target();

        // check if its our process
        if (target->pid != info.pid)
        {
            return EXCEPTION_NOT_HANDLED;
        }

        target->excpt_tid = info.unk;

        int status = 0;
        int res = ksceKernelGetProcessStatus(target->pid, &status);
        LOG("ksceKernelGetProcessStatus res: 0x%08X, status: 0x%08X\n", res, status);

        res = ksceKernelChangeThreadDebugSuspendReason(info.unk, 0x1002);
        LOG("ksceKernelChangeThreadDebugSuspendReason res: 0x%08X\n", res);

        // lets suspend the process for further processing by gdb
        res = ksceKernelProcessDebugSuspend(target->pid, 0x1C);
        LOG("suspending process 0x%08X\n", res);

        // signal GDB that we have halted
        ksceKernelSetEventFlag(g_gdb_evid, GDB_SIGNAL_BKPT);

        return EXCEPTION_NOT_HANDLED;
    }

    return EXCEPTION_NOT_HANDLED;
}

int mem_fault_triggered(excp_saved_ctx_t *ctx, uintptr_t lr, uintptr_t spsr) {
    SceKernelFaultingProcessInfo info;
    int code;
    int ret;
    int cpu;

    ret = ksceKernelGetFaultingProcess(&info);
    LOG("faulting process (ret:%x): 0x%x, 0x%x\n", ret, info.pid, info.unk);
    cpu = ksceKernelCpuGetCpuId();
    LOG("faulting cpu: %d\n", cpu);
    LOG("triggered, spsr: 0x%08X, lr: 0x%08X\n", spsr, lr);
    g_abt_excp_count = ksceExcpmgrGetData();
    LOG("unhandled abt count: %d\n", g_abt_excp_count[cpu]);

    code = ((ctx->ifsr & 0x400) >> 6) | (ctx->ifsr & 0xF);
    LOG("ifsr code: 0x%X\n", code);
  return EXCEPTION_NOT_HANDLED;
}

int insn_undef_triggered(excp_saved_ctx_t *ctx, uintptr_t lr, uintptr_t spsr) {
    SceKernelFaultingProcessInfo info;
    int code;
    int ret;
    int cpu;

    ret = ksceKernelGetFaultingProcess(&info);
    LOG("faulting process (ret:%x): 0x%x, 0x%x\n", ret, info.pid, info.unk);
    cpu = ksceKernelCpuGetCpuId();
    LOG("faulting cpu: %d\n", cpu);
    LOG("triggered, spsr: 0x%08X, lr: 0x%08X\n", spsr, lr);
    g_abt_excp_count = ksceExcpmgrGetData();
    LOG("unhandled abt count: %d\n", g_abt_excp_count[cpu]);

    code = ((ctx->ifsr & 0x400) >> 6) | (ctx->ifsr & 0xF);
    LOG("ifsr code: 0x%X\n", code);
    if (code == 0) {
      LOG("is undef event!\n");
      TargetUnderDebug *target = get_target();

      // check if its our process
      if (target->pid != info.pid)
      {
          return EXCEPTION_NOT_HANDLED;
      }

      target->excpt_tid = info.unk;

      int status = 0;
      int res = ksceKernelGetProcessStatus(target->pid, &status);
      LOG("ksceKernelGetProcessStatus res: 0x%08X, status: 0x%08X\n", res, status);

      res = ksceKernelChangeThreadDebugSuspendReason(info.unk, 0x1002);
      LOG("ksceKernelChangeThreadDebugSuspendReason res: 0x%08X\n", res);

      // lets suspend the process for further processing by gdb
      res = ksceKernelProcessDebugSuspend(target->pid, 0x1C);
      LOG("suspending process 0x%08X\n", res);

      // signal GDB that we have halted
      ksceKernelSetEventFlag(g_gdb_evid, GDB_SIGNAL_BKPT);
      return EXCEPTION_NOT_HANDLED;
    }

  return EXCEPTION_NOT_HANDLED;
}

void register_exceptions(void)
{

}
*/
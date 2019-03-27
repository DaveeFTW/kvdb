#include "cache.h"

#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/sysmem.h>

namespace cache
{
    void flush(int pid, std::uintptr_t vma, std::size_t len)
    {
        uintptr_t vma_align;
        int flags;
        int my_context[3];
        int ret;
        int *other_context = nullptr;
        int dacr;

        vma_align = vma & ~0x1F;
        len = ((vma + len + 0x1F) & ~0x1F) - vma_align;

        if (pid == 0x10005) 
        {
            ksceKernelCpuDcacheWritebackInvalidateRange((void *)vma_align, len);
            ksceKernelCpuIcacheAndL2WritebackInvalidateRange((void *)vma_align, len);
        } 
        else 
        {
            // TODO: Take care of SHARED_PID
            flags = ksceKernelCpuDisableInterrupts();
            ksceKernelCpuSaveContext(my_context);

            // TODO: fix up sdk inconsistencies...
            ret = ksceKernelGetPidContext(pid, reinterpret_cast<SceKernelProcessContext**>(&other_context));
            if (ret >= 0) 
            {
                ksceKernelCpuRestoreContext(other_context);
                asm volatile ("mrc p15, 0, %0, c3, c0, 0" : "=r" (dacr));
                asm volatile ("mcr p15, 0, %0, c3, c0, 0" :: "r" (0x15450FC3));
                ksceKernelCpuDcacheWritebackInvalidateRange((void *)vma_align, len);
                ksceKernelCpuIcacheAndL2WritebackInvalidateRange((void *)vma_align, len);
                asm volatile ("mcr p15, 0, %0, c3, c0, 0" :: "r" (dacr));
            }

            ksceKernelCpuRestoreContext(my_context);
            ksceKernelCpuEnableInterrupts(flags);
        }

        asm volatile ("isb" ::: "memory");
    }
}

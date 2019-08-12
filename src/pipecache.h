#pragma once

#include "eventflag.h"

#include <psp2kern/kernel/threadmgr.h>

#include <cstdint>

class PipeCache
{
public:
    PipeCache(const char *name, std::size_t size = 0x4000);
    
    // kernel mode access
    int read(char *data, std::size_t size, unsigned int timeout = 0);
    int write(const char *data, std::size_t size);
    
    // user mode access
    int copyout(std::uintptr_t udata, std::size_t size, unsigned int timeout = 0);
    int copyin(std::uintptr_t udata, std::size_t size);
    
    std::size_t size() const;

private:
    template <typename F, typename Ptr>
    int do_read(F f, Ptr data, std::size_t size, unsigned int timeout);

    template <typename F, typename Ptr>
    int do_write(F f, Ptr data, std::size_t size);

    template <typename F, typename Ptr>
    void read_cache(F f, Ptr data, std::size_t size);

    template <typename F, typename Ptr>
    void write_cache(F f, Ptr data, std::size_t size);

    EventFlag m_flag;
    std::size_t m_size = 0u;
    std::size_t m_start_ptr = 0u;
    std::size_t m_end_ptr = 0u;
    SceKernelLwMutexWork m_mutex;
    int m_memid = -1;
    char *m_base = nullptr;
};

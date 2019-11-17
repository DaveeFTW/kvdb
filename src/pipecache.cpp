#include "pipecache.h"
#include "log.h"

#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>

#include <string.h>

namespace
{
    enum PipeEvents
    {
        DataAvailable = (1 << 0)
    };
}

PipeCache::PipeCache(const char *name, std::size_t size)
    : m_flag(name)
    , m_size(size)
{
    m_mutex = ksceKernelCreateMutex(name, 0, 0, nullptr);
    m_memid = ksceKernelAllocMemBlock(name, SCE_KERNEL_MEMBLOCK_TYPE_KERNEL_RW, size, nullptr);

    if (m_memid < 0)
    {
        LOG("error allocating PipeCache memory: 0x%08X\n", m_memid);
        return;
    }

    ksceKernelGetMemBlockBase(m_memid, reinterpret_cast<void **>(&m_base));
}

int PipeCache::read(char *data, std::size_t max_size, unsigned int timeout)
{
    return do_read(memcpy, data, max_size, timeout);
}

int PipeCache::write(const char *data, std::size_t size)
{
    return do_write(memcpy, data, size);
}

int PipeCache::copyout(std::uintptr_t udata, std::size_t max_size, unsigned int timeout)
{
    return do_read(ksceKernelMemcpyKernelToUser, udata, max_size, timeout);
}

int PipeCache::copyin(std::uintptr_t udata, size_t size)
{
    return do_write(ksceKernelMemcpyUserToKernel, udata, size);
}

std::size_t PipeCache::size() const
{
    if (m_start_ptr <= m_end_ptr)
    {
        return m_end_ptr - m_start_ptr;
    }

    return (m_size - m_start_ptr) + m_end_ptr;
}

template <typename F, typename Ptr>
int PipeCache::do_read(F f, Ptr data, std::size_t max_size, unsigned int timeout)
{
    unsigned int *eventTimeout = &timeout;

    // we wait indefinitely
    if (!timeout)
    {
        eventTimeout = nullptr;
    }
    
    auto res = m_flag.waitFor(DataAvailable, false, nullptr, eventTimeout);
         
    if (res < 0)
    {
        return res;
    }

    ksceKernelLockMutex(m_mutex, 1, nullptr);

    auto size = this->size();

    // min size is the max we can copy
    size = (size > max_size) ? max_size : size;

    read_cache(f, data, size);

    if (!this->size())
    {
        m_flag.clear(DataAvailable);
    }

    ksceKernelUnlockMutex(m_mutex, 1);  
    return size;  
}

template <typename F, typename Ptr>
int PipeCache::do_write(F f, Ptr data, std::size_t size)
{
    ksceKernelLockMutex(m_mutex, 1, nullptr);

    // check if this new data will fit in our cache
    if (this->size() + size > m_size)
    {
        ksceKernelUnlockMutex(m_mutex, 1);
        return -1;
    }

    write_cache(f, data, size);

    if (this->size()) 
    {
        m_flag.set(DataAvailable);
    }

    ksceKernelUnlockMutex(m_mutex, 1);
    return 0;
}

template <typename F, typename Ptr>
void PipeCache::read_cache(F f, Ptr data, std::size_t size)
{
    auto end_copy_size = m_size - m_start_ptr;

    // check if need to do copy in two parts
    // this is where start_ptr will exceed buffer end
    if (size > end_copy_size) 
    {
        f(data, &m_base[m_start_ptr], end_copy_size);
        f(data + end_copy_size, m_base, size - end_copy_size);
    }

    // no overlap with end of buffer, just copy straight
    else
    {
        f(data, &m_base[m_start_ptr], size);
    }

    m_start_ptr = (m_start_ptr + size) % m_size;
}

template <typename F, typename Ptr>
void PipeCache::write_cache(F f, Ptr data, std::size_t size)
{
    auto end_copy_size = m_size - m_end_ptr;

    // check if need to do copy in two parts
    // this is where end_ptr will exceed buffer end
    if (size > end_copy_size) 
    {
        f(&m_base[m_end_ptr], data, end_copy_size);
        f(m_base, data + end_copy_size, size - end_copy_size);
    }

    // no overlap with end of buffer, just copy straight
    else
    {
        f(&m_base[m_end_ptr], data, size);
    }

    m_end_ptr = (m_end_ptr + size) % m_size;
}

#pragma once

#include <cstdint>

namespace cache
{
    /**
     * @brief      Flush L1 and L2 cache for an address
     *
     *             For thread safety, interrupts may be disabled for the duration of
     *             this call. That plus the act of cache flushing itself makes this
     *             an expensive operation.
     *
     * @param[in]  pid   The pid
     * @param[in]  vma   The vma
     * @param[in]  len   The length
     */
    void flush(int pid, std::uintptr_t vma, std::size_t len);
}

#include "eventflag.h"
#include "log.h"

#include <psp2kern/kernel/threadmgr.h>

EventFlag::EventFlag(const char * &&name)
{
    // TODO: handle error
    m_evid = ksceKernelCreateEventFlag(name, 0, 0, nullptr);
}

int EventFlag::waitFor(unsigned int value, unsigned int *pattern)
{
    return waitFor(value, true, pattern);
}

int EventFlag::waitFor(unsigned int value, bool clear, unsigned int *pattern)
{
    unsigned int flags = SCE_EVENT_WAITAND;

    if (clear)
    {
        flags |= SCE_EVENT_WAITCLEAR;
    }

    auto patternInt = 0u;
    auto res = ksceKernelWaitEventFlag(m_evid, value, flags, &patternInt, nullptr);

    if (res < 0)
    {
        return res;
    }

    if (pattern)
    {
        *pattern = patternInt;
    }

    return 0;
}

int EventFlag::waitForAny(unsigned int value, unsigned int *pattern)
{
    unsigned int patternInt = 0;
    auto res = ksceKernelWaitEventFlag(m_evid, value, SCE_EVENT_WAITOR | SCE_EVENT_WAITCLEAR, &patternInt, nullptr);

    if (res < 0)
    {
        return res;
    }

    if (pattern)
    {
        *pattern = patternInt;
    }

    return 0;
}

int EventFlag::pollFor(unsigned int value, bool clear, unsigned int *pattern)
{
    unsigned int flags = SCE_EVENT_WAITAND;

    if (clear)
    {
        flags |= SCE_EVENT_WAITCLEAR;
    }

    auto patternInt = 0u;
    auto timeout = 0u;
    LOG("begin poll\n");
    auto res = ksceKernelWaitEventFlag(m_evid, value, SCE_EVENT_WAITAND | SCE_EVENT_WAITCLEAR, &patternInt, &timeout);

    LOG("end poll 0x%08X\n", res);
    if (res < 0)
    {
        return res;
    }

    if (pattern)
    {
        *pattern = patternInt;
    }

    return 0;
}

int EventFlag::set(unsigned int value)
{
    return ksceKernelSetEventFlag(m_evid, value);
}

int EventFlag::clear(unsigned int value)
{
    return ksceKernelClearEventFlag(m_evid, value);
}

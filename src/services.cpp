#include "pipe.h"
#include "uart.h"
#include "launch.h"

#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/sysmem.h>

extern "C" void vdb_serial_uart() __attribute__((used));
extern "C" void vdb_serial_pipe() __attribute__((used));
extern "C" int vdb_send_serial_pipe(const char *udata, std::size_t size) __attribute__((used));
extern "C" int vdb_recv_serial_pipe(char *udata, std::size_t max_size, int timeout) __attribute__((used));
extern "C" int vdb_launch_debug(const char *executable) __attribute__((used));

void vdb_serial_uart() 
{
    uart::use();
}

void vdb_serial_pipe()
{
    pipe::use();
}

int vdb_send_serial_pipe(const char *udata, std::size_t size)
{
    int state = 0;
    ENTER_SYSCALL(state);
    auto res = pipe::copyin(reinterpret_cast<std::uintptr_t>(udata), size);
    EXIT_SYSCALL(state);
    return res;
}

int vdb_recv_serial_pipe(char *udata, std::size_t max_size, int timeout)
{
    int state = 0;
    ENTER_SYSCALL(state);
    auto res = pipe::copyout(reinterpret_cast<std::uintptr_t>(udata), max_size, timeout);
    EXIT_SYSCALL(state);
    return res;
}

int vdb_launch_debug(const char *executable)
{
    constexpr size_t maxStringLength = 255;
    char path[maxStringLength];
    int state = 0;
    ENTER_SYSCALL(state);
    
    auto res = ksceKernelStrncpyUserToKernel(path, reinterpret_cast<std::uintptr_t>(executable), maxStringLength);

    if (res < 0)
    {
        EXIT_SYSCALL(state);
        return res;
    }

    path[maxStringLength] = '\0';

    res = launch::for_debug(path);
    EXIT_SYSCALL(state);
    return res;
}

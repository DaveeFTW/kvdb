#include "pipe.h"
#include "uart.h"

#include <psp2kern/kernel/cpu.h>

extern "C" void vdb_serial_uart() __attribute__((used));
extern "C" void vdb_serial_pipe() __attribute__((used));
extern "C" void vdb_send_serial_pipe() __attribute__((used));
extern "C" void vdb_recv_serial_pipe() __attribute__((used));

void vdb_serial_uart() 
{
    uart::use();
}

void vdb_serial_pipe()
{
    pipe::use();
}

int vdb_send_serial_pipe(std::uintptr_t udata, std::size_t size)
{
    int state = 0;
    ENTER_SYSCALL(state);
    auto res = pipe::copyin(udata, size);
    EXIT_SYSCALL(state);
    return res;
}

int vdb_recv_serial_pipe(std::uintptr_t udata, std::size_t max_size, int timeout)
{
    int state = 0;
    ENTER_SYSCALL(state);
    auto res = pipe::copyout(udata, max_size, timeout);
    EXIT_SYSCALL(state);
    return res;
}
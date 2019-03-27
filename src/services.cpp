#include "pipe.h"
#include "uart.h"

extern "C" void vdb_serial_uart() __attribute__((used));
extern "C" void vdb_serial_pipe() __attribute__((used));

void vdb_serial_uart() 
{
    uart::use();
}

void vdb_serial_pipe()
{
    pipe::use();
}

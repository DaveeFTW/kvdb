#include "exception.h"
#include "pipe.h"
#include "uart.h"
#include "debugger.h"

#include "launch.h"

#include <psp2kern/io/fcntl.h>
#include <psp2kern/kernel/threadmgr.h>

namespace 
{
    int init(SceSize args, void *argp)
    {
        ksceIoRemove("ux0:tai/kvdb.skprx");

        pipe::init();
        uart::init();
        uart::use();

        exception::register_handlers();

        debugger::get()->start();
        return 0;
    }

    int test(SceSize args, void *argp)
    {
        ksceKernelDelayThread(20*1000*1000);
        launch::for_debug("ux0:data/eboot.bin");
        return 0;
    }
}

extern "C" void __cxa_pure_virtual() { while (1); }

extern "C" int module_start()
{
    // TODO: handle returns for this
    auto thid = ksceKernelCreateThread("kvdb", init, 0x40, 0x8000, 0, 0, nullptr);
    ksceKernelStartThread(thid, 0, nullptr);

    // test launcher
    auto thid2 = ksceKernelCreateThread("kvdb2", test, 0x40, 0x8000, 0, 0, nullptr);
    ksceKernelStartThread(thid2, 0, nullptr);
    return 0;
}

extern "C" int _start() __attribute__ ((weak, alias ("module_start")));

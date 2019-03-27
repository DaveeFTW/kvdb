#include "launch.h"
#include "debugger.h"
#include "log.h"

#include <psp2kern/appmgr.h>

#include <string.h>

namespace
{
    struct AppMgrLaunchParam : public SceAppMgrLaunchParam
    {
        AppMgrLaunchParam()
        {
            memset(this, 0, sizeof(AppMgrLaunchParam));
            size = sizeof(AppMgrLaunchParam);
        }
    };
}

namespace launch
{
    int for_debug(const char *path)
    {
        const char launch_args[] = "\0\0-titleid\0MLCL05505\0";

        AppMgrLaunchParam param;
        param.unk_4 = 0x80000000; // launch application but suspend immediately

        // TODO: check results
        auto pid = ksceAppMgrLaunchAppByPath(path, launch_args, sizeof(launch_args), 0x80000000, &param, nullptr);
        LOG("ksceAppMgrLaunchAppByPath(\"%s\"): 0x%08X\n", path, pid);

        // wait for debugger to know that the app is launched
        auto debugger = debugger::get();
        debugger->waitForSignal(Debugger::Signal::PROCESS_CREATED);

        // okay, tell debugger to attach to this process
        debugger->attach(pid);
        return 0;
    }
}

#pragma once

extern "C"
{
    void debugger_pabt_handler(void);
    void debugger_dabt_handler(void);
    void debugger_und_handler(void);
}

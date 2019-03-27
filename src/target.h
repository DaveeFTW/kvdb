#pragma once

class Target
{
public:
    int pid = -1;
    int main_module_id = -1;
    int main_thread_id = -1;
    int excpt_tid = -1;
};
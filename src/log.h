#pragma once

#define LOGGING_SCEd
#define LOGGING_UART

#ifdef LOGGING
extern "C" int ksceDebugPrintf(const char *fmt, ...);
#define LOG(fmt, ...) ksceDebugPrintf("[kvdb] " fmt, ##__VA_ARGS__)
#elif defined(LOGGING_UART)
#include "uart.h"
#define LOG(fmt, ...) uart::printf("[kvdb] " fmt, ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif
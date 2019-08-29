#ifndef HOOKS_H
#define HOOKS_H

#include "tinyos.h"

// 目前添加四个钩子函数
void hooksCpuIdle(void);                        // idle task hook
void hookSysTick(void);                         // sys tick hook
void hooksTaskSwitch(tTask* from, tTask* to);   // task switch hook
void hooksTaskInit(tTask* task);                // task init hook

#endif

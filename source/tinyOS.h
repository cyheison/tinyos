#ifndef TINYOS_H
#define TINYOS_H

#include "stdint.h"
#include "ARMCM3.h"

#define NVIC_INT_CTRL               0xE000Ed04
#define NVIC_PENDSVSET              0x10000000
#define NVIC_SYSPRI2                0xE000ED22
#define NVIC_PENDSV_PRI             0x000000FF

#define mem32(addr)                 *(volatile uint32_t *)(addr)
//#define mem8(addr)                  *(unsigned char *)(addr)

typedef struct
{
    uint32_t * stack;
    uint32_t systemTickCount;
}tTask;

extern tTask * nextTask;
extern tTask * currentTask;

void tTaskRunFirst(void);
void taskSwitch(void);
uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);

#endif

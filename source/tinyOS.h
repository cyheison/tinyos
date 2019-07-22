#ifndef TINYOS_H
#define TINYOS_H

#include "ARMCM3.h"
#include "config.h"
#include "tLib.h"
#include "task.h"
#include "event.h"
#include "sem.h"
#include "mBox.h"

#define NVIC_INT_CTRL               0xE000Ed04
#define NVIC_PENDSVSET              0x10000000
#define NVIC_SYSPRI2                0xE000ED22
#define NVIC_PENDSV_PRI             0x000000FF

#define mem32(addr)                 *(volatile uint32_t *)(addr)
//#define mem8(addr)                  *(unsigned char *)(addr)

typedef enum
{   
    ERROR_NOERROR = 0,
    ERROR_TIMEOUT = 1,
    ERROR_RSCNOTAVALIABLE = 2, 
    ERROR_NORESOURCE,
}errorNum;

extern tTask * nextTask;
extern tTask * currentTask;

void tTaskRunFirst(void);
void taskSwitch(void);
uint32_t tTaskEnterCritical(void);
void tTaskExitCritical(uint32_t status);
void tTaskSchedInit(void);

void tTaskSchedDisable(void);

void tTaskSchedEnable(void);

// This schedual function can be invoked by irq or task, so needs to to be protected.
void tTaskSched(void);
void tTaskSystemTickHandler(void);
void timedTaskWait(tTask* task, uint32_t tick);

void tSetSysTickPeriod(uint32_t ms);
void initApp(void);
void taskSchedUnReady(tTask* task);
void taskSchedReady(tTask* task);
void setTaskDelay(uint32_t delay);
void taskWakeupFromSuspend(tTask* task);
void taskDelayedDelete(tTask* task);
void taskSchedDelete(tTask* task);


#endif

#ifndef TINYOS_H
#define TINYOS_H

#include "stdint.h"
#include "ARMCM3.h"
#include "config.h"
#include "tLib.h"

#define NVIC_INT_CTRL               0xE000Ed04
#define NVIC_PENDSVSET              0x10000000
#define NVIC_SYSPRI2                0xE000ED22
#define NVIC_PENDSV_PRI             0x000000FF

#define mem32(addr)                 *(volatile uint32_t *)(addr)
//#define mem8(addr)                  *(unsigned char *)(addr)

#define TINYOS_TASK_STATE_RDY           0
#define TINYOS_TASK_STATE_DELAY         (1<<1)
#define TINYOS_TASK_STATE_SUSPEND       (1<<2)

typedef struct
{
    uint32_t*   stack;
    uint32_t    systemTickCount;
    tNode       linkNode; // tasks shoule be added into a list when they have same priority
    uint32_t    slice;    // When tasks have the same priority, slice number shows that how long they can occupy the CPU. Now the default time is 100ms  
    uint32_t    pri;
    tNode       delayNode;
    uint32_t    suspendCount; // Used to record how many times that this task has been suspended. Usually we only suspend a task once.
    uint32_t    state;
    
    // Used for task clean
    void (*clean)(void*);
    void* cleanParam;
    uint8_t requestDeleteFlag; // Flag for the request of deleting task
}tTask;

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
void tTaskSchedual(void);
void tTaskSystemTickHandler(void);
void timedTaskWakeUp(tTask* task);
void timedTaskWait(tTask* task, uint32_t tick);
void tTaskInit(tTask *task, void (*entry)(void*), void* param, uint32_t pri, uint32_t *stack);
void tSetSysTickPeriod(uint32_t ms);
void initApp(void);
void taskSchedUnReady(tTask* task);
void taskSchedReady(tTask* task);
void setTaskDelay(uint32_t delay);
void taskSuspend(tTask* task);
void taskWakeupFromSuspend(tTask* task);
void taskDelayedDelete(tTask* task);
void taskSchedDelete(tTask* task);
void taskRegisterCleanFunc(tTask* task, void (*clean)(void*), void* param);
void taskForceDelete(tTask* task);
void taskRequestDelete(tTask* task);
uint8_t taskRequestDeleteFlag(tTask* task);
void taskDeleteSelf(tTask* task);


#endif

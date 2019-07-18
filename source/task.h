#ifndef TASK_H
#define TASK_H

#define TINYOS_TASK_STATE_RDY           0
#define TINYOS_TASK_STATE_DESTROYED     (1<<1)
#define TINYOS_TASK_STATE_DELAY         (1<<2)
#define TINYOS_TASK_STATE_SUSPEND       (1<<3)

// Reserve high 16bis to event
#define TINYOS_TASK_WAIT_MASK           (0xff<<16)

struct _tEvent;

typedef struct
{
    uint32_t*   stack;
    uint32_t    systemTickCount; // Delay ticks for this task
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
    
    // Task event
    struct _tEvent* waitEvent; // This task is waiting for what event
    void* eventMsg; // Used in mail
    uint32_t eventWaitResult;
}tTask;

typedef struct 
{
    uint32_t delayTicks;
    uint32_t pri;
    uint32_t state;
    uint32_t slice;
    uint32_t suspendCount;
}tTaskInfo;

void tTaskInit(tTask *task, void (*entry)(void*), void* param, uint32_t pri, uint32_t *stack);
void taskSuspend(tTask* task);
void timedTaskWakeUp(tTask* task);
void taskRegisterCleanFunc(tTask* task, void (*clean)(void*), void* param);
void taskForceDelete(tTask* task);
void taskRequestDelete(tTask* task);
uint8_t taskRequestDeleteFlag(tTask* task);
void taskDeleteSelf(tTask* task);
void taskInfoGet(tTask* task, tTaskInfo* taskInfo);

#endif

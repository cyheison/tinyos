#include "tinyos.h"

//task point; entry function; param; stack space.
// Init all the components in a task
void tTaskInit(tTask *task, void (*entry)(void*), void* param, uint32_t pri, uint32_t *stack, uint32_t size)//size表示的是多少个byte
{   
    uint32_t* stackTop;
    
    task->stackBase = stack;
    task->stackSize = size;
    stackTop = stack + size / sizeof(uint32_t); // 传入的是数组的首地址，但是是stack的末尾地址（stack是向下生长的）。这里除以的是堆栈单元的大小
    
    //init the stack. when converting task, these values will pop into registers
    //will be stored automaticaly
    *(--stackTop) = (uint32_t)(1<<24);//xPSR
    *(--stackTop) = (uint32_t)entry;//PC
    *(--stackTop) = (uint32_t)0x14;//LR
    *(--stackTop) = (uint32_t)0x12;
    *(--stackTop) = (uint32_t)0x3;
    *(--stackTop) = (uint32_t)0x2;
    *(--stackTop) = (uint32_t)0x1;
    *(--stackTop) = (uint32_t)param;
    
    // need to store manually
    *(--stackTop) = (uint32_t)0x11;
    *(--stackTop) = (uint32_t)0x10;
    *(--stackTop) = (uint32_t)0x9;
    *(--stackTop) = (uint32_t)0x8;
    *(--stackTop) = (uint32_t)0x7;
    *(--stackTop) = (uint32_t)0x6;
    *(--stackTop) = (uint32_t)0x5;
    *(--stackTop) = (uint32_t)0x4;
    
    // now stack addr is &tTask->stack[1024] - 4 * 16. So when switching task, we need to use LDMIA to restore these registers' value
    task->stack = stackTop;
    task->systemTickCount = 0;
    task->slice = TINYOS_TASK_CLICE_COUNT;
    task->pri = pri;
    task->state = TINYOS_TASK_STATE_RDY;
    task->suspendCount = 0;
    task->clean = (void (*)(void*))0;
    task->cleanParam = (void*)0;
    task->requestDeleteFlag = 0;
    
    // Initialize the nodes in this task
    nodeInit(&task->delayNode);
    nodeInit(&task->linkNode);
        
    taskSchedReady(task);
}

void taskSuspend(tTask* task)
{
    uint32_t status = tTaskEnterCritical();
    
    // When task in on timed delay state, we shouldn't suspend it
    if (!(task->state & TINYOS_TASK_STATE_DELAY))
    {
        // We suspend the task only once
        if (++task->suspendCount <= 1)
        {
            taskSchedUnReady(task);
            task->state = TINYOS_TASK_STATE_SUSPEND;
            
            // We need to switch to another task asap
            if (task == currentTask)
            {
                tTaskSched();
            }
        }
    }
    
    tTaskExitCritical(status);
}

void taskWakeupFromSuspend(tTask* task)
{
    uint32_t status = tTaskEnterCritical();
    
    if (task->state & TINYOS_TASK_STATE_SUSPEND)
    {
        // When this count reduced to 0, then we can wake up it
        if (--task->suspendCount == 0)
        {
            taskSchedReady(task);
            task->state = TINYOS_TASK_STATE_RDY;
            tTaskSched();
        }
    }
    
    tTaskExitCritical(status);
}

// Provide interface for app layer to register clean function
void taskRegisterCleanFunc(tTask* task, void (*clean)(void*), void* param)
{
    task->clean = clean;
    task->cleanParam = param;
}

// task force delete
void taskForceDelete(tTask* task)
{
    uint32_t status = tTaskEnterCritical();
    
    if (task->state == TINYOS_TASK_STATE_DELAY)
    {
        taskDelayedDelete(task);
    }
    else if (!(task->state == TINYOS_TASK_STATE_SUSPEND))
    {
        taskSchedDelete(task);
    }
    
    if (task->clean)
    {
        task->clean(task->cleanParam);
    }
    
    if (task == currentTask)
    {
        tTaskSched();
    }
    
    tTaskExitCritical(status);
}

// Task request delete
void taskRequestDelete(tTask* task)
{
    uint32_t status = tTaskEnterCritical();
    
    task->requestDeleteFlag = 1;
    
    tTaskExitCritical(status);
}

// Check the task request delete flag
uint8_t taskRequestDeleteFlag(tTask* task)
{
    uint8_t delete;
    
    uint32_t status = tTaskEnterCritical();
    
    delete = task->requestDeleteFlag;
    
    tTaskExitCritical(status);
    
    return delete;
}

void taskDeleteSelf(tTask* task)
{
    uint32_t status = tTaskEnterCritical();
    
    // Because task in delayList won't be going here, so only need to delete the tasks that are in sched list.
    taskSchedDelete(task);
    
    if (task->clean)
    {
        task->clean(task->cleanParam);
    }
    
    if (task == currentTask)
    {
        tTaskSched();
    }
    
    tTaskExitCritical(status);
}

void taskInfoGet(tTask* task, tTaskInfo* taskInfo)
{
    uint32_t* stackEnd; // 数组的首地址，即stack的最低地址
    uint32_t stats = tTaskEnterCritical();
    
    taskInfo->delayTicks    = task->systemTickCount;
    taskInfo->pri           = task->pri;
    taskInfo->slice         = task->slice;
    taskInfo->state         = task->state;
    taskInfo->suspendCount  = task->suspendCount;
    taskInfo->stackSize     = task->stackSize;
    
    // calculate the stack free size
    taskInfo->stackFree = 0;
    stackEnd = task->stackBase;
    
    //这里可以看出还是单独定义一个stackSize的宏比较好，因为不一定是uint32_t的。
    while ((*stackEnd++ == 0) && (stackEnd <= task->stackBase + task->stackSize / sizeof(uint32_t))) //地址的一个单元是4个byte
    {
        taskInfo->stackFree++;
    }
    //这里只是单元数，需要转成真正的byte数
    taskInfo->stackFree *= sizeof(uint32_t);
    
    tTaskExitCritical(stats);
}



#include "tinyos.h"

//task point; entry function; param; stack space.
// Init all the components in a task
void tTaskInit(tTask *task, void (*entry)(void*), void* param, uint32_t pri, uint32_t *stack)
{   
    //init the stack. when converting task, these values will pop into registers
    //will be stored automaticaly
    *(--stack) = (uint32_t)(1<<24);//xPSR
    *(--stack) = (uint32_t)entry;//PC
    *(--stack) = (uint32_t)0x14;//LR
    *(--stack) = (uint32_t)0x12;
    *(--stack) = (uint32_t)0x3;
    *(--stack) = (uint32_t)0x2;
    *(--stack) = (uint32_t)0x1;
    *(--stack) = (uint32_t)param;
    
    // need to store manually
    *(--stack) = (uint32_t)0x11;
    *(--stack) = (uint32_t)0x10;
    *(--stack) = (uint32_t)0x9;
    *(--stack) = (uint32_t)0x8;
    *(--stack) = (uint32_t)0x7;
    *(--stack) = (uint32_t)0x6;
    *(--stack) = (uint32_t)0x5;
    *(--stack) = (uint32_t)0x4;
    
    // now stack addr is &tTask->stack[1024] - 4 * 16. So when switching task, we need to use LDMIA to restore these registers' value
    task->stack = stack;
    task->systemTickCount = 0;
    task->slice = TINYOS_TASK_CLICE_COUNT;
    task->pri = pri;
    task->state = TINYOS_TASK_STATE_RDY;
    
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
                tTaskSchedual();
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
            tTaskSchedual();
        }
    }
    
    tTaskExitCritical(status);
}

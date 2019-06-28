#include "tinyos.h"

void eventInit(tEvent* event, tEventType eventType)
{
    event->eventType = eventType;
    listInit(&event->waitList);
}

// The last three params are used for mail and semaphore
void eventAddWait(tEvent* event, tTask* task, void* msg, uint32_t state, uint32_t timeOut)
{
    uint32_t status = tTaskEnterCritical();
    
    task->eventMsg = msg;
    task->waitEvent = event;
    task->state |= state;// For the future use, use |
    task->eventResult = ERROR_NOERROR;
    
    // Remove this task from the sched list
    taskSchedUnReady(task);
    
    // Add this task into the waiting list
    listAddLast(&event->waitList, &task->linkNode);
    
    // If set the timeout, then this task should also be added into the timeDelay list
    if (timeOut)
    {
        timedTaskWait(task, timeOut);
    }
    
    tTaskExitCritical(status);
}

// Wake up the first task in the waiting list
tTask* eventWakeUp(tEvent* event, void* msg, uint32_t result)
{
    tNode* node;
    tTask* task = (tTask*)0;
    
    uint32_t status = tTaskEnterCritical();
    
    // Waiting list may be null
    if ((node = listRemoveFirst(&event->waitList)) != (tNode*)0)
    {
        task = tNodeParent(node, tTask, linkNode);
        task->eventMsg = msg;
        task->eventResult = result;
        task->waitEvent = (tEvent*)0; // Don't forget to clean the waitEvent
        task->state &= ~TINYOS_TASK_WAIT_MASK; //
        
        // Check whether this task has delay
        if (task->systemTickCount > 0)
        {
            timedTaskWakeUp(task);
        }
        
        // Add task into sched list
        taskSchedReady(task);
    }
    
    // Here don't need to remove from the delay list because in systick semaphore will do this operation.
        
    tTaskExitCritical(status);
    
    return task;
}

void eventRemoveTask(tTask* task, void* msg, uint32_t result)
{
    uint32_t status = tTaskEnterCritical();
    task->eventMsg = msg;
    task->waitEvent = (tEvent*)0;
    task->state &= TINYOS_TASK_WAIT_MASK;// For the future use, use |
    task->eventResult = result; // error number
    listRemove(&task->waitEvent->waitList, &task->linkNode);
    
    tTaskExitCritical(status);
}

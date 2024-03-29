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
    task->waitEvent = event;// combine event and task
    task->state |= state << 16;// Which event, sem or mbox. only high 16bits is left for event in task.h
    task->eventWaitResult = ERROR_NOERROR;
    
    // Remove this task from the sched list
    taskSchedUnReady(task);
    
    // Add this task into the waiting list
    listAddLast(&event->waitList, &task->linkNode);//注意waitList中仅保存linkNode，所以查看waitList是看不到具体是哪个task的。
                                                    //除非查看该linkNode归属于哪个task
    
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
        task->eventWaitResult = result;
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

// Wake up detailed task
tTask* eventWakeUpTask(tEvent* event, tTask* task, void* msg, uint32_t result)
{   
    uint32_t status = tTaskEnterCritical();
    
    // Critical step: from event waiting list delete this task
    listRemove(&event->waitList, &task->linkNode);
    
    task->eventMsg = msg;
    task->eventWaitResult = result;
    task->waitEvent = (tEvent*)0; // Don't forget to clean the waitEvent
    task->state &= ~TINYOS_TASK_WAIT_MASK; //
        
    // Check whether this task has delay
    if (task->systemTickCount > 0)
    {
        timedTaskWakeUp(task);
    }
    
    // Add task into sched list
    taskSchedReady(task);
    
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
    task->eventWaitResult = result; // error number
    listRemove(&task->waitEvent->waitList, &task->linkNode);
    
    tTaskExitCritical(status);
}

// Remove every task from the event list to sched list and timed delay list
uint32_t eventRemoveAll(tEvent* event, void* msg, uint32_t result)
{
    uint32_t count;
    tNode* node;
    tTask* task;
    
    uint32_t status = tTaskEnterCritical();
    
    count = listCount(&event->waitList);
    
    while ((node=listRemoveFirst(&event->waitList)) != (tNode*)0)
    {
        task = tNodeParent(node, tTask, linkNode);
        task->eventMsg = msg;
        task->waitEvent = (tEvent*)0;
        task->eventWaitResult = result;
        task->state &= ~TINYOS_TASK_WAIT_MASK;
        
        if (task->systemTickCount != 0)
        {
            // If this task is in the delay list, we should wake up it
            timedTaskWakeUp(task);
        }

        // Put this task into sched list
        taskSchedReady(task);
    }
    
    tTaskExitCritical(status);
    
    return count;
}

// How many tasks waiting in the list
uint32_t eventWaitCount(tEvent* event)
{
    uint32_t count;
    uint32_t status = tTaskEnterCritical();
    
    count = listCount(&event->waitList);
    
    tTaskExitCritical(status);
    
    return count;
}

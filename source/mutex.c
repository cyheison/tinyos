#include "tinyos.h"

void mutexInit(tMuxte* mutex)
{
    eventInit(&mutex->event, eventTypeMutex);
    
    mutex->lockedCount = 0;
    mutex->owner = (tTask*)0;
    mutex->ownerOriPri = TINYOS_PRI_COUNT;
}

uint32_t mutexWait(tMuxte* mutex, uint32_t waitTicks)
{
    uint32_t stats = tTaskEnterCritical();
    
    if (mutex->lockedCount <= 0)
    {
        // Mutex is avaliable now
        mutex->lockedCount++;
        mutex->ownerOriPri = currentTask->pri;
        mutex->owner = currentTask;
        
        tTaskExitCritical(stats);
        return ERROR_NOERROR;
    }
    else
    {
        // Mutex is not avaliable now
        if (mutex->owner == currentTask)
        {
            mutex->lockedCount++;
            
            tTaskExitCritical(stats);
            return ERROR_NOERROR;
        }
        else
        {
            // Other tasks has been occupyed this mutex, so we need to check its prio first
            if (currentTask->pri < mutex->owner->pri)
            {
                // This means low prio task has been occupied this mutex, so we need upgrade its prio to high
                tTask* owner = mutex->owner;
                
                //因为currentTask在运行，所以owner不可能是running状态，有可能是RDY，或者delay等状态。
                if (owner->state == TINYOS_TASK_STATE_RDY)
                {
                    // If task is on ready state, it's in the taskPriTable so it will be invoked at any time, so we need to remove it from taskPriTable first
                    taskSchedUnReady(owner);
                    owner->pri = currentTask->pri;
                    taskSchedReady(owner);
                }
                else
                {
                    // If owner task is in running/delayed/pending state, we can change its prio directly
                    // But why we need to remove it first if it's in taskPriTable???????????????
                    owner->pri = currentTask->pri;
                }
            }
            
            //  Then we need to add current task into waiting list
            eventAddWait(&mutex->event, currentTask, (void*)0, eventTypeMutex, waitTicks);
            tTaskExitCritical(stats);

            tTaskSched();
            return currentTask->eventWaitResult;
        }
    }
}

uint32_t mutexNoWaitGet(tMuxte* mutex)
{
    uint32_t stats = tTaskEnterCritical();
    
    if (mutex->lockedCount <= 0)
    {
        // Mutex is avaliable now
        mutex->lockedCount++;
        mutex->ownerOriPri = currentTask->pri;
        mutex->owner = currentTask;
        
        tTaskExitCritical(stats);
        return ERROR_NOERROR;
    }
    else
    {
        // Mutex is not avaliable now
        if (mutex->owner == currentTask)
        {
            mutex->lockedCount++;
            
            tTaskExitCritical(stats);
            return ERROR_NOERROR;
        }
        else
        {
            tTaskExitCritical(stats);
            return ERROR_NORESOURCE;
        }
    }
}
    
uint32_t mutexNotify(tMuxte* mutex)
{
    uint32_t stats = tTaskEnterCritical();
    
    if (mutex->lockedCount <= 0)
    {
        tTaskExitCritical(stats);
        return ERROR_NOERROR;
    }
    
    // Only owner can notify this mutex, other task can't do it
    if (mutex->owner != currentTask)
    {
        tTaskExitCritical(stats);
        return ERROR_OWNER;
    }
    
    if (--mutex->lockedCount > 0)
    {
        tTaskExitCritical(stats);
        return ERROR_NOERROR; 
    }
    
    // Check if we need to restore the prio
    // Before we change to another task, we need to restore current task's pri
    if (mutex->owner->pri != mutex->ownerOriPri)
    {
        if (mutex->owner->state == TINYOS_TASK_STATE_RDY)
        {
            taskSchedUnReady(mutex->owner);
            currentTask->pri = mutex->ownerOriPri;// Can we use mutex->owner->pri ????
            taskSchedReady(mutex->owner);
        }
        else
        {
            currentTask->pri = mutex->ownerOriPri;
        }
    }
    
    // Notify a waiting task in this mutex's waitling list
    if (eventWaitCount(&mutex->event))
    {
        tTask* task = eventWakeUp(&mutex->event, (void*)0, ERROR_NOERROR);
        
        // When task has been waked up, we need to update mutex element
        mutex->owner = task;
        mutex->ownerOriPri = task->pri;
        mutex->lockedCount++;
        
        if (task->pri < currentTask->pri)
        {
            tTaskSched();
        }
    }
    
    tTaskExitCritical(stats);
    return ERROR_NOERROR;
}



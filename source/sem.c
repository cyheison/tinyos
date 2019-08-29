#include "sem.h"
#include "tinyos.h"

#if TINYOS_ENABLE_SEM 
void semInit(tSem* sem, uint32_t semCount, uint32_t maxCount)
{
    eventInit(&sem->event, eventTypeSem);
    sem->maxCount = maxCount;
    
    if (maxCount == 0)
    {
        // When maxCount = 0 means that no limit to the semCount
        sem->semCount = semCount;
    }
    else
    {
        sem->semCount = (semCount < maxCount)? semCount : maxCount;
    }
}

// Check whether current task needs to wait for sema
uint32_t semWait(tSem* sem, uint32_t waitTicks )
{
    uint32_t status = tTaskEnterCritical();
    
    if (sem->semCount > 0)
    {
        // Why here need to -- ???????
        sem->semCount--;
        tTaskExitCritical(status);
        return ERROR_NOERROR;
    }
    else
    {
        eventAddWait(&sem->event, currentTask, (void*)0, 0, waitTicks);//waitTicks表示最多能等待多长时间，为0表示无限等待下去，无超时。
        tTaskExitCritical(status);
        
        tTaskSched();
        
        return currentTask->eventWaitResult;
    }
}

uint32_t semNoWaitGet(tSem* sem)
{
    uint32_t status = tTaskEnterCritical();
    
    if (sem->semCount > 0)
    {
        // Why every time when we check the sem waiting status, we need to -- ??????
        sem->semCount--;
        tTaskExitCritical(status);
        return ERROR_NOERROR;
    }
    else
    {
        // We don't need to insert into event waiting list
        tTaskExitCritical(status);
        return ERROR_RSCNOTAVALIABLE;
    }    
}

void semNotify(tSem* sem)
{
    tTask* task;
    
    uint32_t status = tTaskEnterCritical();
    
    if (sem->semCount > 0)
    {
        // Wake up a task
        task = eventWakeUp(&sem->event, (void*)0, ERROR_NOERROR);
        
        if (task->pri < currentTask->pri)
        {
            tTaskSched();
        }
    }
    else
    {
        // Indicates that currently we have resource and we need to wake up a task. So if new task is trying to be added into waitlist, we will wake it up immediately.
        sem->semCount++;// Why if now waiting task, we need to ++ ???????
        
        // If sem->maxCount == 0, it means that we can ++ semCount forever.
        if (sem->maxCount != 0 && sem->semCount > sem->maxCount)
        {
            sem->semCount = sem->maxCount;
        }
    }
    
    tTaskExitCritical(status);
}

void semInfoGet(tSem* sem, tSemInfo* semInfo)
{
    uint32_t stats = tTaskEnterCritical();
    
    semInfo->semCount = sem->semCount;
    semInfo->maxCount = sem->maxCount;
    semInfo->waitTaskCount = eventWaitCount(&sem->event);
    
    tTaskExitCritical(stats);
}

// Remove all the waiting tasks which are waiting for this semaphore.
uint32_t semRemoveAll(tSem* sem)
{
    uint32_t count;
    
    uint32_t stats = tTaskEnterCritical();
    
    count = eventRemoveAll(&sem->event, (void*)0, ERROR_NOERROR);
    sem->semCount = 0; // We have remove all the waiting tasks for this semaphore/event.
    
    if (count > 0)// It means we have removed some tasks, so it might need to switch the task
    {
        tTaskSched();
    }
    
    tTaskExitCritical(stats);
    
    return count;
}
#endif

#include "sem.h"
#include "tinyos.h"

void semInit(tSem* sem, uint32_t startCount, uint32_t maxCount)
{
    eventInit(&sem->event, eventTypeSem);
    sem->maxCount = maxCount;
    
    if (maxCount == 0)
    {
        // When maxCount = 0 means that no limit to the startCount
        sem->startCount = startCount;
    }
    else
    {
        sem->startCount = (startCount < maxCount)? startCount : maxCount;
    }
}

// Check whether current task needs to wait for sema
uint32_t semWait(tSem* sem, uint32_t waitTicks )
{
    uint32_t status = tTaskEnterCritical();
    
    if (sem->startCount > 0)
    {
        // Why here need to -- ???????
        sem->startCount--;
        tTaskExitCritical(status);
        return ERROR_NOERROR;
    }
    else
    {
        eventAddWait(&sem->event, currentTask, (void*)0, 0, waitTicks);
        tTaskExitCritical(status);
        
        tTaskSched();
        
        return currentTask->eventWaitResult;
    }
}

uint32_t semNoWaitGet(tSem* sem)
{
    uint32_t status = tTaskEnterCritical();
    
    if (sem->startCount > 0)
    {
        // Why every time when we check the sem waiting status, we need to -- ??????
        sem->startCount--;
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
    
    if (sem->startCount > 0)
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
        sem->startCount++;
        
        // If sem->maxCount == 0, it means that we can ++ startCount forever.
        if (sem->maxCount != 0 && sem->startCount > sem->maxCount)
        {
            sem->startCount = sem->maxCount;
        }
    }
    
    tTaskExitCritical(status);
}


// Why if now waiting task, we need to ++ ???????


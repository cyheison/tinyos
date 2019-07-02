#include "sem.h"

void semInit(tSem* sem, uint32_t startCount, uint32_t maxCount)
{
    eventInit(&sem->event, eventTypeSem);
    sem->maxCount = maxCount;
    
    if (maxCount == 0)
    {
        // Error?
        sem->startCount = startCount;
    }
    else
    {
        sem->startCount = (startCount < maxCount)? startCount : maxCount;
    }
}

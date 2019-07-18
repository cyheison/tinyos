#ifndef SEM_H
#define SEM_H

#include "event.h"

typedef struct _tSem
{
    tEvent event;
    uint32_t startCount; // If adding a new waiting task, we need to ++. This value shows that how many tasks are waiting now.
    uint32_t maxCount;
}tSem;

void semInit(tSem* sem, uint32_t startCount, uint32_t maxCount);
uint32_t semWait(tSem* sem, uint32_t waitTicks );
uint32_t semNoWaitGet(tSem* sem);
void semNotify(tSem* sem);

#endif

#ifndef SEM_H
#define SEM_H

#include "event.h"

typedef struct _tSem
{
    tEvent event;// One sem only points to one event. But one event can have multi waiting tasks.
    uint32_t semCount; // If adding a new waiting task, we need to ++. This value shows that how many tasks are waiting now.
    uint32_t maxCount;
}tSem;

typedef struct _tSemInfo
{
    uint32_t semCount;
    uint32_t maxCount;
    uint32_t waitTaskCount;
}tSemInfo;

void semInit(tSem* sem, uint32_t startCount, uint32_t maxCount);
uint32_t semWait(tSem* sem, uint32_t waitTicks );
uint32_t semNoWaitGet(tSem* sem);
void semNotify(tSem* sem);
void semInfoGet(tSem* sem, tSemInfo* semInfo);
uint32_t semRemoveAll(tSem* sem);

#endif

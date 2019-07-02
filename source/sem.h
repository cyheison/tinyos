#ifndef SEM_H
#define SEM_H

#include "event.h"

typedef struct _tSem
{
    tEvent event;
    uint32_t startCount;
    uint32_t maxCount;
}tSem;

void semInit(tSem* sem, uint32_t startCount, uint32_t maxCount);

#endif

#ifndef MUTEX_H
#define MUTEX_H

#include "tinyos.h"

typedef struct _tMutex
{   
    tEvent event;
    uint32_t lockedCount;
    tTask* task;
    uint32_t ownerOriPri;
}tMuxte;

void mutexInit(tMuxte* mutex);

#endif

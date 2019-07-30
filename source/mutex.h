#ifndef MUTEX_H
#define MUTEX_H

#include "tinyos.h"

typedef struct _tMutex
{   
    tEvent      event;
    uint32_t    lockedCount;
    tTask*      owner;
    uint32_t    ownerOriPri;
}tMuxte;

void mutexInit(tMuxte* mutex);
uint32_t mutexWait(tMuxte* mutex, uint32_t waitTicks);
uint32_t mutexNoWaitGet(tMuxte* mutex);
uint32_t mutexNotify(tMuxte* mutex);

#endif

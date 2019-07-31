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

typedef struct _tMutexInfo
{
    uint32_t waitCount;
    uint32_t ownerOriPri;
    uint32_t inheritedPri;
    tTask*   owner;
    uint32_t lockedCount;
}tMutexInfo;

void mutexInit(tMuxte* mutex);
uint32_t mutexWait(tMuxte* mutex, uint32_t waitTicks);
uint32_t mutexNoWaitGet(tMuxte* mutex);
uint32_t mutexNotify(tMuxte* mutex);

uint32_t mutexDestroy(tMuxte* mutex);
void mutexInfoGet(tMuxte* mutex, tMutexInfo* mutexInfo);

#endif

#include "tinyos.h"

void mutexInit(tMuxte* mutex)
{
    eventInit(&mutex->event, eventTypeMutex);
    
    mutex->lockedCount = 0;
    mutex->task = (tTask*)0;
    mutex->ownerOriPri = TINYOS_PRI_COUNT;
}

#ifndef EVENT_H
#define EVENT_H

#include "tLib.h"
#include "task.h"

typedef enum
{
    eventTypeUnKnown,
    eventTypeSem,
    eventTypeMbox,
    eventTypeMemBlock,
}tEventType;

// Waiting task list
typedef struct _tEvent
{
    tEventType  eventType;
    tList       waitList; // How many tasks are waiting for this eventType. We will also define an event in the task structure to indicate that which event the task is waiting for.
}tEvent;

void eventInit(tEvent* event, tEventType eventType);
void eventAddWait(tEvent* event, tTask* task, void* msg, uint32_t state, uint32_t timeOut);
tTask* eventWakeUp(tEvent* event, void* msg, uint32_t result);
void eventRemoveTask(tTask* task, void* msg, uint32_t result);
uint32_t eventRemoveAll(tEvent* event, void* msg, uint32_t result);
uint32_t eventWaitCount(tEvent* event);

#endif

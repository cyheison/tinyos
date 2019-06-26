#ifndef EVENT_H
#define EVENT_H

#include "tLib.h"

typedef enum
{
    eventTypeUnKnown,
}tEventType;

typedef struct
{
    tEventType  eventType;
    tList       waitList; // Put the waiting tasks into a list
}tEvent;

void eventInit(tEvent* event, tEventType eventType);

#endif

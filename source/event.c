#include "tinyos.h"

void eventInit(tEvent* event, tEventType eventType)
{
    event->eventType = eventType;
    listInit(&event->waitList);
}

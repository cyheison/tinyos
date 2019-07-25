#ifndef FLAGGROUP_H
#define FLAGGROUP_H

#include "event.h"

typedef struct _flagGroup
{
    tEvent event;
    uint32_t flag;
}tFlagGroup;

void flagGroupInit(tFlagGroup* flagGrp, uint32_t flag);

#endif

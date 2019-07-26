#ifndef FLAGGROUP_H
#define FLAGGROUP_H

#include "event.h"

typedef struct _flagGroup
{
    tEvent event;
    uint32_t flag;
}tFlagGroup;


#define FLAGGROUP_CLEAR     (0x0 << 0)
#define FLAGGROUP_SET       (0x1 << 0)
#define FLAGGROUP_ANY       (0x0 << 1)
#define FLAGGROUP_ALL       (0x1 << 1)// all bis will be set or clean

#define FLAGGROUP_SET_ALL       (FLAGGROUP_SET | FLAGGROUP_ALL)
#define FLAGGROUP_SET_ANAY      (FLAGGROUP_SET | FLAGGROUP_ANAY)
#define FLAGGROUP_CLEAR_ALL     (FLAGGROUP_CLEAR | FLAGGROUP_ALL)
#define FLAGGROUP_CLEAR_ANAY    (FLAGGROUP_CLEAR | FLAGGROUP_ANAY)

#define FLAGGROUP_CONSUME   (1 << 7)

void flagGroupInit(tFlagGroup* flagGrp, uint32_t flag);

// requestFlag is the flags it wishes for. 
// resultFlag is the result that points out which flags it has been waited for
uint32_t flagGroupWait(tFlagGroup* flagGrp, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag, uint32_t waitTicks);
uint32_t flagGroupNoWaitGet(tFlagGroup* flagGrp, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag);
void flagGroupNotify(tFlagGroup* flagGrp, uint8_t isSet, uint32_t flag);


#endif

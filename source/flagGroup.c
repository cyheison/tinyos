#include "tinyos.h"

void flagGroupInit(tFlagGroup* flagGrp, uint32_t flag)
{
    eventInit(&flagGrp->event, eventTypeFlagGroup);
    flagGrp->flag = flag;
}

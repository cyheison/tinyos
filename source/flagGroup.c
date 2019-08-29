#include "tinyos.h"

#if TINYOS_ENABLE_FLAGGROUP
void flagGroupInit(tFlagGroup* flagGrp, uint32_t flag)
{
    eventInit(&flagGrp->event, eventTypeFlagGroup);
    flagGrp->flag = flag;
}

// Assistant function
// Returen is the queried flag result
static uint32_t flagGroupCheckAndConsume(tFlagGroup* flagGrp, uint32_t type, uint32_t* flags)
{
    uint32_t waitingFlags = *flags;
    uint32_t isSet = type & FLAGGROUP_SET;
    uint32_t isAll = type & FLAGGROUP_ALL;
    uint32_t isConsume = type & FLAGGROUP_CONSUME;
    
    //不管是置1还是清零，将实际发生的bit和需要的bit进行对比，然后得到这个calFlags
    uint32_t calFlags = isSet ? (flagGrp->flag & waitingFlags) : (~flagGrp->flag & waitingFlags);
    
    //第一个表明：我要等待全部的bit被置位或清零，并且这已经发生了；
    //第二个表明：我不需要等待所有的bit被置位或清零，然后计算的得到calFlags不为0，表明已经有bit置位了，那就可以了
    //如果不满足该if， 则表明还没有发生相应的flag
    if ( ((isAll != 0) && (calFlags == waitingFlags)) || ((isAll == 0) &&(calFlags != 0 )))
    {
        // event has been triggered
        if (isConsume)
        {
            if (isSet)
            {
                //将对应bit清零
                flagGrp->flag &= ~waitingFlags;
            }
            else
            {
                //将对应bit置1
                flagGrp->flag |= waitingFlags;
            }
        }
        
        // Store the calFlags back to caller to make him knowing that which bit is he wants and whether it has happened.
        *flags = calFlags;
        return ERROR_NOERROR;
    }
    
    *flags = calFlags;
    return ERROR_NORESOURCE;
}

// requestFlag is the flag the task is waiting for
uint32_t flagGroupWait(tFlagGroup* flagGrp, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag, uint32_t waitTicks)
{
    uint32_t waitingFlags = requestFlag;
    uint32_t result;
    
    uint32_t stats = tTaskEnterCritical();
    
    // Get current task's flags and check these bits already setted or cleaned. If not so, puting them into waiting list
    result = flagGroupCheckAndConsume(flagGrp, waitType, &waitingFlags);
    
    if (result != ERROR_NOERROR)
    {
        // First time i forget to set these two elements of currentTask then in notify function can't find this task's right info.
        // These two elemetns will be used in nofity function
        currentTask->waitFlagsType = waitType;
        currentTask->eventFlags = requestFlag;
        // We need to put current task into flag waiting list
        eventAddWait(&flagGrp->event, currentTask, (void*)0, eventTypeFlagGroup, waitTicks);
        tTaskExitCritical(stats);
        
        tTaskSched();
        
        // When back to here, it means that some bits has been setted or cleaned, so get its flags
        *resultFlag = currentTask->eventFlags;
        result = currentTask->eventWaitResult;
    }
    else
    {
        // current task is the one we want, so go on executing 
        *resultFlag = currentTask->eventFlags;
        tTaskExitCritical(stats);
    }
    
    return result;
}


uint32_t flagGroupNoWaitGet(tFlagGroup* flagGrp, uint32_t waitType, uint32_t requestFlag, uint32_t* resultFlag)
{
    uint32_t waitingFlags = requestFlag;
    uint32_t result;
    
    uint32_t stats = tTaskEnterCritical();
    
    // Get current task's flags and check these bits already setted or cleaned. If not so, puting them into waiting list
    result = flagGroupCheckAndConsume(flagGrp, waitType, &waitingFlags);
    tTaskExitCritical(stats);
    
    *resultFlag = waitingFlags;
    
    return result;
}

// flags is the already happened flag, so write this flags into the flagGrp
void flagGroupNotify(tFlagGroup* flagGrp, uint8_t isSet, uint32_t flags)
{
    uint32_t stats = tTaskEnterCritical();
    tList* waitList;
    tNode* node;
    uint32_t waitingFlags;
    uint32_t result;
    uint8_t sched = 0;
    
    if (isSet)
    {
        flagGrp->flag |= flags;
    }
    else
    {
        flagGrp->flag &= ~flags;
    }
    
    waitList = &flagGrp->event.waitList;
    
    for (node = waitList->node.nextNode; node != &waitList->node; node = node->nextNode)
    {
        tTask* task = tNodeParent(node, tTask, linkNode);
        // Get the flags that this task is waiting for
        waitingFlags = task->eventFlags;
        
        result = flagGroupCheckAndConsume(flagGrp, task->waitFlagsType, &waitingFlags);
        
        // All the flags we want happened, so this task is the one we want 
        if (result == ERROR_NOERROR)
        {
            // Store its remainder flags for furture use
            task->eventFlags = flags;
            eventWakeUpTask(&flagGrp->event, task, (void*)0, ERROR_NOERROR);
            
            sched = 1;
        }
    }
    
    tTaskExitCritical(stats);
    
    if (sched == 1)
    {
        tTaskSched();    
    }
}


void flagGroupInfoGet(tFlagGroup* flagGrp, flagGroupInfo* flagInfo)
{
    uint32_t stats = tTaskEnterCritical();
    
    flagInfo->flags = flagGrp->flag;
    flagInfo->taskCount = eventWaitCount(&flagGrp->event);
    
    tTaskExitCritical(stats);
}

uint32_t flagGroupDestroy(tFlagGroup* flagGrp)
{
    uint32_t stats = tTaskEnterCritical();
    
    uint32_t count = eventRemoveAll(&flagGrp->event, (void*)0, ERROR_NOERROR);
    
    tTaskExitCritical(stats);
    
    if (count > 0)
    {
        tTaskSched();
    }
    
    return count;
}
#endif

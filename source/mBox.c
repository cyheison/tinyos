#include "tinyos.h"

#if TINYOS_ENABLE_MBOX
void mBoxInit(tMbox* mBox, void** msgBuffer, uint32_t maxCount)
{
    eventInit(&mBox->event, eventTypeMbox);
    mBox->msgBuffer = msgBuffer;
    mBox->read = 0;
    mBox->write = 0;
    mBox->msgCount = 0;
    mBox->maxCount = maxCount;
}

// Get msg from this mBox
// Task asks for msg and if no msg is avaliable, task will be waiting
uint8_t mBoxWait(tMbox* mBox, void** msg, uint32_t waitTicks)
{
    uint32_t stats = tTaskEnterCritical();
    
    if (mBox->msgCount > 0)
    {
        // msg is avaliable
        --mBox->msgCount;
        *msg = mBox->msgBuffer[mBox->read++];
        
        // We should check read pointer here
        if (mBox->read >= mBox->maxCount)
        {
            mBox->read = 0;
        }
        
        tTaskExitCritical(stats);
        return ERROR_NOERROR;
    }
    else
    {
        // msg is not avaliable, so adding the task into waiting list
        eventAddWait(&mBox->event, currentTask, (void*)0, eventTypeMbox, waitTicks);
        // This exit critical must be before task sched, otherwise it will cause the next task working abnormally
        tTaskExitCritical(stats);
        
        tTaskSched();
        
        // When task backs to here, then it should carry the msg it needs
        *msg = currentTask->eventMsg;
        
        // Because we get the msg from task, so not from mail box, so we don't need to reduce.
        //mBox->msgCount--;
        
        return currentTask->eventWaitResult;
        
    }
}

uint8_t mBoxNoWait(tMbox* mBox, void** msg)
{
    uint32_t stats = tTaskEnterCritical();
    
    if (mBox->msgCount > 0)
    {
        mBox->msgCount--;
        *msg = mBox->msgBuffer[mBox->read++];
        
        // We should check read pointer here
        if (mBox->read >= mBox->maxCount)
        {
            mBox->read = 0;
        }
        
        tTaskExitCritical(stats);
        return ERROR_NOERROR;
    }
    else
    {
        tTaskExitCritical(stats);
        return ERROR_NORESOURCE;
    }   
}

// Write msg to this mBox
// Msg is avaliable, then should find a task
uint8_t mBoxNotify(tMbox* mBox, void* msg, uint8_t notifyOption)
{
    // Why here uses void* msg, but upper uses void** msg??
    // That's because here msg is using array, but upper stores pointer in a array.
    // void** 认为就是指针的地址,该地址中保存了一个指针，这个指针指向了一片内存区域的首地址。*指针就是取出了该地址中的值。
    
    uint32_t stats = tTaskEnterCritical();
    
    // There are tasks waiting in the list
    if (eventWaitCount(&mBox->event) > 0)
    {
        // When waking up a task, pass in the msg it needs
        tTask* task = eventWakeUp(&mBox->event, msg, ERROR_NOERROR);
        if (task->pri < currentTask->pri)
        {
            tTaskSched();
        }
    }
    else
    {
        // No task is waiting, so we need to store this msg in buffer
        if (notifyOption & tMOXSendFront)// The last inserted msg is higher priority
        {
            // Send high priority msg first
            if (mBox->read <= 0)
            {
                // Current msg is already the highest priority, so will back to the tail
                mBox->read = mBox->maxCount - 1;
            }
            else
            {
                --mBox->read;
            }
            mBox->msgBuffer[mBox->read] = msg;
        }
        else
        {
            // Normal read
            mBox->msgBuffer[mBox->write++] = msg;// The last inserted msg is lowest priority
            
            if (mBox->write >= mBox->maxCount)
            {
                mBox->write = 0;
            }
        }
        
        // Dont forget to add msg count
        mBox->msgCount++;
    }
    
    tTaskExitCritical(stats);
    return ERROR_NOERROR;
}

// Clean up all the msgs in the mail box
void mBoxFlush(tMbox* mBox)
{
    uint32_t stats = tTaskEnterCritical();
    
    // When there are tasks waiting in the list, mail box must be cleaned. So when tasks are empty, msg box may not empty.
    if (eventWaitCount(&mBox->event) == 0)
    {
        mBox->read = 0;
        mBox->write = 0;
        mBox->msgCount = 0;
    }
    
    tTaskExitCritical(stats);
}

uint32_t mBoxRemoveTask(tMbox* mBox)
{
    uint32_t stats = tTaskEnterCritical();
    
    uint32_t count = eventRemoveAll(&mBox->event, (void*)0, ERROR_NOERROR);
    
    tTaskExitCritical(stats);
    
    // The deleted task may be the running task
    if (count > 0)
    {
        tTaskSched();
    }
    
    return count;
}

void mBoxInfoGet(tMbox* mBox, tMboxInfo* mBoxInfo)
{
    uint32_t stats = tTaskEnterCritical();
    
    mBoxInfo->msgCount = mBox->msgCount;
    mBoxInfo->maxCount = mBox->maxCount;
    mBoxInfo->taskCount = eventWaitCount(&mBox->event);
    
    tTaskExitCritical(stats);
}
#endif

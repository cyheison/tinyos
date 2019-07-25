#include "tinyos.h"

void memBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCount)
{
    // 这里的memStart只需要一个数组的首地址，所以有三种方式可以传进来。
    uint8_t* memBlockStart = memStart;
    uint8_t* memBlockEnd = memStart + blockCount * blockSize;
    
    // If blockSize is too small, then we will reject this apply
    if (blockSize < sizeof(tNode))
    {
        return;
    }
    
    eventInit(&memBlock->event, eventTypeMemBlock);
    memBlock->memStart = memStart;
    memBlock->blockCount = blockCount;
    memBlock->blockSize = blockSize;
    
    listInit(&memBlock->blockList);
    
    while (memBlockStart < memBlockEnd)
    {
        //这里不可以使用memStart，因为memStart不是一个正统的数组首地址，其实它是一个二维数组的地址做了类型变换来的。而memBlockStart是一个正统的地址。
        nodeInit((tNode*)memBlockStart);
        // 用node*来修饰是为了在每个block的开头放两个指针，构成链表，后面的是数据，数据的大小是blockSize - 8（2个指针）
        listAddLast(&memBlock->blockList, (tNode*)memBlockStart);
        
        memBlockStart += blockSize;
    }
}

// Try to get the mem block, if no block is avaliable, then add this task into waiting list
uint32_t memBlockWait(tMemBlock* memBlock, uint8_t** mem, uint32_t waitTicks)
{
    // 这里之所以传入二维指针，传入的是指针的地址，直接将mem的地址赋给指针
    uint32_t stats = tTaskEnterCritical();
    
    if (listCount(&memBlock->blockList) > 0)
    {
        *mem = (uint8_t*)listRemoveFirst(&memBlock->blockList);
        tTaskExitCritical(stats);
        return ERROR_NOERROR;
    }
    else
    {
        // No mem is avaliable, add task into waiting list
        eventAddWait(&memBlock->event, currentTask, (void*)0, eventTypeMemBlock, waitTicks);
        tTaskExitCritical(stats);
        
        tTaskSched();
        
        //这里eventMsg需要传一个指针过来
        *mem = currentTask->eventMsg;
        return currentTask->eventWaitResult;
    }
}

uint32_t memBlockNoWaitGet(tMemBlock* memBlock, uint8_t** mem)
{
        // 这里之所以传入二维指针，传入的是指针的地址，直接将mem的地址赋给指针
    uint32_t stats = tTaskEnterCritical();
    
    if (listCount(&memBlock->blockList) > 0)
    {
        *mem = (uint8_t*)listRemoveFirst(&memBlock->blockList);
        tTaskExitCritical(stats);
        return ERROR_NOERROR;
    }
    else
    {
        tTaskExitCritical(stats);
        return ERROR_NORESOURCE;
    }    
}

void memBlockNotify(tMemBlock* memBlock, uint8_t* mem)
{
    uint32_t stats = tTaskEnterCritical();
    
    if (eventWaitCount(&memBlock->event) > 0)
    {
        //msg传入指针，因为wait那里是在等指针，只要是指针就行，不限定什么类型的指针。
        tTask* task = eventWakeUp(&memBlock->event, (void*)mem, ERROR_NOERROR);
        if (task->pri < currentTask->pri)
        {
            tTaskSched();
        }
    }
    else
    {
        listAddLast(&memBlock->blockList, (tNode*)mem);
    }
    
    tTaskExitCritical(stats);
}



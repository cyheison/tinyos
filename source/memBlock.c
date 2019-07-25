#include "tinyos.h"

void memBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCount)
{
    // �����memStartֻ��Ҫһ��������׵�ַ�����������ַ�ʽ���Դ�������
    uint8_t* memBlockStart = memStart;
    uint8_t* memBlockEnd = memStart + blockCount * blockSize;
    
    // If blockSize is too small, then we will reject this apply
    if (blockSize < sizeof(tNode))
    {
        return;
    }
    
    eventInit(&memBlock->event, eventTypeMemBlock);
    memBlock->memStart = memStart;
    memBlock->maxBlockCount = blockCount;
    memBlock->blockSize = blockSize;
    
    listInit(&memBlock->blockList);
    
    while (memBlockStart < memBlockEnd)
    {
        //���ﲻ����ʹ��memStart����ΪmemStart����һ����ͳ�������׵�ַ����ʵ����һ����ά����ĵ�ַ�������ͱ任���ġ���memBlockStart��һ����ͳ�ĵ�ַ��
        nodeInit((tNode*)memBlockStart);
        // ��node*��������Ϊ����ÿ��block�Ŀ�ͷ������ָ�룬������������������ݣ����ݵĴ�С��blockSize - 8��2��ָ�룩
        listAddLast(&memBlock->blockList, (tNode*)memBlockStart);
        
        memBlockStart += blockSize;
    }
}

// Try to get the mem block, if no block is avaliable, then add this task into waiting list
uint32_t memBlockWait(tMemBlock* memBlock, uint8_t** mem, uint32_t waitTicks)
{
    // ����֮���Դ����άָ�룬�������ָ��ĵ�ַ��ֱ�ӽ�mem�ĵ�ַ����ָ��
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
        
        //����eventMsg��Ҫ��һ��ָ�����
        *mem = currentTask->eventMsg;
        return currentTask->eventWaitResult;
    }
}

uint32_t memBlockNoWaitGet(tMemBlock* memBlock, uint8_t** mem)
{
        // ����֮���Դ����άָ�룬�������ָ��ĵ�ַ��ֱ�ӽ�mem�ĵ�ַ����ָ��
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
        //msg����ָ�룬��Ϊwait�������ڵ�ָ�룬ֻҪ��ָ����У����޶�ʲô���͵�ָ�롣
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

void memInfoGet(tMemBlock* memBlock, memBlockInfo* memBlockInfo)
{
    uint32_t stats = tTaskEnterCritical();
    
    memBlockInfo->blockSize = memBlock->blockSize;
    memBlockInfo->maxBlockCount = memBlock->maxBlockCount;
    memBlockInfo->blockCountUsed = listCount(&memBlock->blockList);
    memBlockInfo->taskCount = eventWaitCount(&memBlock->event);
    
    tTaskExitCritical(stats);
}

uint32_t memBlockDestroy(tMemBlock* memBlock)
{
    uint32_t stats = tTaskEnterCritical();
    
    // event �ṩ���Լ���ɾ���ȴ�event��task�ĺ���
    uint32_t count = eventRemoveAll(&memBlock->event, (void*)0, ERROR_NOERROR);
    
    if (count > 0)
    {
        tTaskExitCritical(stats);
        tTaskSched();
    }
    
    return count;
}


#include "tinyos.h"

void memBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCount)
{
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
        //���ﲻ����ʹ��memStart����ΪmemStart����һ����ͳ�������׵�ַ����ʵ����һ����ά����ĵ�ַ�������ͱ任���ġ���memBlockStart��һ����ͳ�ĵ�ַ��
        nodeInit((tNode*)memBlockStart);
        listAddLast(&memBlock->blockList, (tNode*)memBlockStart);
        
        memBlockStart += blockSize;
    }
}

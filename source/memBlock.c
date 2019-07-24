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
        //这里不可以使用memStart，因为memStart不是一个正统的数组首地址，其实它是一个二维数组的地址做了类型变换来的。而memBlockStart是一个正统的地址。
        nodeInit((tNode*)memBlockStart);
        listAddLast(&memBlock->blockList, (tNode*)memBlockStart);
        
        memBlockStart += blockSize;
    }
}

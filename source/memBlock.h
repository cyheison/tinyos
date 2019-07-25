#ifndef MEMBLOCK_H
#define MEMBLOCK_H

#include "event.h"

typedef struct _memBlock
{
    tEvent      event;
    void*       memStart;
    uint32_t    blockSize;
    uint32_t    blockCount;
    tList       blockList;
}tMemBlock;

void memBlockInit(tMemBlock* memBlock, uint8_t* memStart, uint32_t blockSize, uint32_t blockCount);
uint32_t memBlockWait(tMemBlock* memBlock, uint8_t** mem, uint32_t waitTicks);
uint32_t memBlockNoWaitGet(tMemBlock* memBlock, uint8_t** mem);
void memBlockNotify(tMemBlock* memBlock, uint8_t* mem);

#endif

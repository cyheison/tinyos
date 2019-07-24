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


#endif

#ifndef TLIB_H
#define TLIB_H

#include "stdint.h"

typedef struct
{
    uint32_t bitMap;
}tBitMap;

void bitMapInit(tBitMap * pBitMap);
void bitMapSet(tBitMap * pBitMap, uint8_t pos);
void bitMapClear(tBitMap * pBitMap, uint8_t pos);
uint8_t bitMapGetFirstSet(tBitMap * pBitMap);
uint8_t bitMapGetPosCount(void);

#endif

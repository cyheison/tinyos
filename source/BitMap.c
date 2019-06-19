#include "tLib.h"

uint8_t bitMapGetPosCount()
{
    return 32;
}

void bitMapInit(tBitMap * pBitMap)
{
    pBitMap->bitMap = 0;
}

void bitMapSet(tBitMap * pBitMap, uint8_t pos)
{
    pBitMap->bitMap |= 1 << pos;
}

void bitMapClear(tBitMap * pBitMap, uint8_t pos)
{
    pBitMap->bitMap &= ~(1 << pos);
}

uint8_t bitMapGetFirstSet(tBitMap * pBitMap)
{
    static const uint8_t quickFindTable[] = 
    {
	    /* 00 */ 0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 10 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 20 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 30 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 40 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 50 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 60 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 70 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 80 */ 7,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 90 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* A0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* B0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* C0 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* D0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* E0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
        /* F0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
    };
    
    if (pBitMap->bitMap & 0xff)
    {
        return quickFindTable[pBitMap->bitMap & 0xff];
    }
    else if (pBitMap->bitMap & 0xff00)
    {
        return quickFindTable[(pBitMap->bitMap) >> 8 & 0xff] + 8;
    }
    else if (pBitMap->bitMap & 0xff0000)
    {
        return quickFindTable[pBitMap->bitMap >> 16 & 0xff] + 16;
    }
    else if (pBitMap->bitMap & 0xff000000)
    {
        return quickFindTable[pBitMap->bitMap >> 24 & 0xff] + 24;
    }
    else
    {
        return bitMapGetPosCount();
    }
    
}

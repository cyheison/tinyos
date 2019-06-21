#ifndef TLIB_H
#define TLIB_H

#include "stdint.h"

typedef struct
{
    uint32_t bitMap;
}tBitMap;

typedef struct _tNode
{
    struct _tNode * preNode;
    struct _tNode * nextNode;
}tNode;

typedef struct
{
    tNode node;
    uint32_t nodeCount;
}tList;

#define tNodeParent(node, parent, name)     (parent*)((uint32_t)node - (uint32_t)&((parent*)0)->name)

void bitMapInit(tBitMap * pBitMap);
void bitMapSet(tBitMap * pBitMap, uint8_t pos);
void bitMapClear(tBitMap * pBitMap, uint8_t pos);
uint8_t bitMapGetFirstSet(tBitMap * pBitMap);
uint8_t bitMapGetPosCount(void); 

void nodeInit(tNode* node);  
uint32_t listCount(tList * list);
// Assign preNode and nextNode to point to its struct addr
void listInit(tList * list);
// Returen this list's first node 
tNode* listFirstNode(tList * list);
// Return this list's last node
tNode* listLastNode(tList * list);
tNode* listPreNode(tList * list, tNode* node);
tNode* listNextNode(tList * list, tNode* node);
void listRemoveAll(tList* list);
void listAddFirst(tList * list, tNode* node);
void listAddLast(tList * list, tNode* node);
tNode* listRemoveFirst(tList* list);
void listInsertAfter(tList* list, tNode* nodeAfter, tNode* nodeToInsert);
void listRemove(tList* list, tNode* node);



#endif

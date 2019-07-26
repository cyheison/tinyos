 #include "tLib.h"
 
 // When firstNode is being used, then lastNode won't be used. it means that these two macros will only exist one at the same time.
 // If use addfirst, then firstNode will always points to the first node
 // If use addlast, then lastNode will always points to the last node
 // That's because we use circle bidirectional linked list.
 #define firstNode node.nextNode // Why nextNode is the first? Because 
 #define lastNode  node.preNode
 
 // Node init means put this node's pre/next to point to itself
 void nodeInit(tNode* node)  
 {
    node->preNode = node;
    node->nextNode = node;
 }
 
 uint32_t listCount(tList * list)
 {
    return list->nodeCount;
 }
 
 // Assign preNode and nextNode to point to its struct addr
 void listInit(tList * list)
 {
    list->lastNode = &(list->node);
    list->firstNode = &(list->node);
    list->nodeCount = 0;
 }
 
// Returen this list's first node 
tNode* listFirstNode(tList * list)
{    
    tNode* node = (tNode*)0;
    if (list->nodeCount != 0)
    {
        node = list->firstNode;
    }
    
    return node;
}
 
// Return this list's last node
tNode* listLastNode(tList * list)
{
    tNode* node = (tNode*)0;
    if (list->nodeCount != 0)
    {
        node = list->lastNode;
    }
    
    return node;
}

tNode* listPreNode(tList * list, tNode* node)
{
    if (node->preNode == node)
    {
        return (tNode*)0;
    }
    else
    {
        return node->preNode;
    }
}

tNode* listNextNode(tList * list, tNode* node)
{
    if (node->nextNode == node)
    {
        return (tNode*)0;
    }
    else
    {
        return node->nextNode;
    }
}

void listRemoveAll(tList* list)
{
    uint32_t i;
    tNode* nextNode = list->firstNode;
    
    for(i=0; i<list->nodeCount; i++)
    {
        tNode* currentNode = nextNode;
        nextNode = nextNode->nextNode;
        
        currentNode->preNode = currentNode;
        currentNode->nextNode = currentNode;
    }
    
    // Because list is a global var, so its lastNode and firstNode should alse be cleaned up.
    list->lastNode = &(list->node);
    list->firstNode = &(list->node);
    list->nodeCount = 0;
}

void listAddFirst(tList * list, tNode* node)
{
    node->preNode = list->firstNode->preNode;
    node->nextNode = list->firstNode;
    
    list->firstNode->preNode = node;
    list->firstNode = node;
    list->nodeCount++;
}

void listAddLast(tList * list, tNode* node)
{
    node->nextNode = &(list->node);
    node->preNode = list->lastNode;
    
    list->lastNode->nextNode = node;
    list->lastNode = node;
    list->nodeCount++;
}

tNode* listRemoveFirst(tList* list)
{
    tNode* node = (tNode*)0;
    
    if (list->nodeCount != 0)
    {
        node = list->firstNode;
        node->nextNode->preNode = &(list->node);
        list->firstNode = node->nextNode;
        list->nodeCount--;
    }
    return node;
}

void listInsertAfter(tList* list, tNode* nodeAfter, tNode* nodeToInsert)
{
    nodeToInsert->preNode = nodeAfter;
    nodeToInsert->nextNode = nodeAfter->nextNode;
    
    nodeAfter->nextNode->preNode = nodeToInsert;
    nodeAfter->nextNode = nodeToInsert;
    
    list->nodeCount++;
}

void listRemove(tList* list, tNode* node)
{
    node->preNode->nextNode = node->nextNode;
    node->nextNode->preNode = node->preNode;
    list->nodeCount--;
}

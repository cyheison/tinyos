#ifndef MBOX_H
#define MBOX_H

#include "event.h"

#define tMOXSendNormal  0x00
#define tMOXSendFront   0x01

typedef struct _mBox
{
    tEvent event;
    uint32_t msgCount;
    uint32_t read;// Read position index
    uint32_t write;// Write position index
    uint32_t maxCount;
    void** msgBuffer; // In this array, every element we store a pointer
}tMbox;

void mBoxInit(tMbox* mBox, void** msgBuffer, uint32_t maxMsgCount);
uint8_t mBoxWait(tMbox* mBox, void** msg, uint32_t waitTicks);
uint8_t mBoxNoWait(tMbox* mBox, void** msg);
uint8_t mBoxNotify(tMbox* mBox, void* msg, uint8_t notifyOption);

#endif
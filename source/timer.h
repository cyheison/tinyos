#ifndef TIMER_H
#define TIMER_H

#include "tinyos.h"

typedef enum _tTimerState
{
    tTimerCreated,
    tTimerStarted,
    tTimerRunning,
    tTimerStopped,
    tTimerDestroyed
}tTimerState;

typedef struct _tTimer
{
    tNode linkNode;
    uint32_t startDelayTicks; // initial value
    uint32_t durationTicks; // timer duration ticks
    uint32_t delayTicks; // current ticks
    void (*timerFuc)(void* arg);
    void* arg;//param wants to pass into timerFunc
    uint32_t config;
    tTimerState state;
}tTimer;

// 软件定时器的cb函数，一个放在system tick中，一个放在timer task中
#define TIMER_CONFIG_TYPE_HARD          (1<<0)  // handled in interrupt
#define TIMER_CONFIG_TYPE_SOFT          (0<<0)  // handled in timer task

void timerInit(tTimer* timer, uint32_t startTicks, uint32_t durationTicks,
                void (*timerFuc)(void* arg), void* arg, uint32_t config);

#endif

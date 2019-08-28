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
    uint32_t startDelayTicks; // 这个参数是有用的。在start timer时，根据该值决定是启动delayTicks还是durationTicks，即先等待还是直接启动真正的timer
    uint32_t durationTicks; // timer duration ticks
    uint32_t delayTicks; // current ticks
    void (*timerFuc)(void* arg);
    void* arg;//param wants to pass into timerFunc
    uint32_t config; // Hard or soft timer
    tTimerState state;
}tTimer;

// 软件定时器的cb函数，一个放在system tick中，一个放在timer task中。注意这里都是软件定时器
#define TIMER_CONFIG_TYPE_HARD          (1<<0)  // handled in interrupt
#define TIMER_CONFIG_TYPE_SOFT          (0<<0)  // handled in timer task

void timerInit(tTimer* timer, uint32_t startTicks, uint32_t durationTicks,
                void (*timerFuc)(void* arg), void* arg, uint32_t config);
void timerModuleInit(void);
void timerSoftTask(void* param);
void timerStop(tTimer* timer);
void timerStart(tTimer* timer);
void timerHandleList(tList* timerList);
void timerModuleNotify(void);


#endif

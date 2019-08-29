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
    uint32_t startDelayTicks; // ������������õġ���start timerʱ�����ݸ�ֵ����������delayTicks����durationTicks�����ȵȴ�����ֱ������������timer
    uint32_t durationTicks; // timer duration ticks
    uint32_t delayTicks; // current ticks
    void (*timerFuc)(void* arg);
    void* arg;//param wants to pass into timerFunc
    uint32_t config; // Hard or soft timer
    tTimerState state;
}tTimer;

typedef struct _tTimerInfo
{
    uint32_t startDelayTicks; // ������������õġ���start timerʱ�����ݸ�ֵ����������delayTicks����durationTicks�����ȵȴ�����ֱ������������timer
    uint32_t durationTicks; // timer duration ticks
    void (*timerFuc)(void* arg);
    void* arg;//param wants to pass into timerFunc
    uint32_t config; // Hard or soft timer
    tTimerState state; 
}tTimerInfo;

// �����ʱ����cb������һ������system tick�У�һ������timer task�С�ע�����ﶼ�������ʱ����֮����Ū����list����Ϊ���������һ��list�У���Щtimer��
// ����ʱ�ͻ�ܴ���soft list�е�timer������ʱ����ܻ����Ӻ󣬵������Ĵ�����������΢��һЩ������hard list�е�timer�������������С������
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
void timerInfoGet(tTimer* timer, tTimerInfo* info);
void timerDestroy(tTimer* timer);
void timerModuleTaskInit(void);


#endif

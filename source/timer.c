#include "timer.h"

// after delayTicks, timer will be started, its cycle is durationTicks
void timerInit(tTimer* timer, uint32_t delayTicks, uint32_t durationTicks,
                void (*timerFuc)(void* arg), void* arg, uint32_t config)
{
    nodeInit(&timer->linkNode);
    timer->startDelayTicks = delayTicks;//��ʾ��Ҫ���ٸ�tick֮����ö�ʱ������
    timer->durationTicks = durationTicks;//timer
    timer->timerFuc = timerFuc;
    timer->arg = arg;
    timer->config = config;
    
    if (delayTicks == 0)//�������ö�ʱ������
    {
        timer->delayTicks = durationTicks;//���ֺͶ�ʱ����ʱ��һ�£������ڶ�ʱ������ʱ��delayTicks���Ա���һ���ɾ���ֵ
    }
    else
    {
        // already have delay at the beggining
        // this delayTicks will be --
        timer->delayTicks = timer->startDelayTicks;//�ȴ�delayTicks֮�󣬶�ʱ���ٿ�ʼ����
    }
    
    timer->state = tTimerCreated;
}                    

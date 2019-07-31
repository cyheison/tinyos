#include "timer.h"

// after delayTicks, timer will be started, its cycle is durationTicks
void timerInit(tTimer* timer, uint32_t delayTicks, uint32_t durationTicks,
                void (*timerFuc)(void* arg), void* arg, uint32_t config)
{
    nodeInit(&timer->linkNode);
    timer->startDelayTicks = delayTicks;//表示想要多少个tick之后才让定时器工作
    timer->durationTicks = durationTicks;//timer
    timer->timerFuc = timerFuc;
    timer->arg = arg;
    timer->config = config;
    
    if (delayTicks == 0)//上来就让定时器工作
    {
        timer->delayTicks = durationTicks;//保持和定时器的时钟一致，这样在定时器到期时，delayTicks可以保持一个干净的值
    }
    else
    {
        // already have delay at the beggining
        // this delayTicks will be --
        timer->delayTicks = timer->startDelayTicks;//等待delayTicks之后，定时器再开始工作
    }
    
    timer->state = tTimerCreated;
}                    

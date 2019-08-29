#include "timer.h"

static tList timerHardList;     // Hard timer list, only accessed in systick irq
static tList timerSoftList;     // Soft timer list, accessed by soft timer task and other tasks, but in systick irq can't access this list
static tSem  timerProtectSem;   // Used by timer task and other tasks to access timerSoftList
static tSem  timerTickSem;      // In systick irq, notify timer task that there is some timer timeout, now timer task can access timerHardList 
static tTask timerTask;         // Task used in soft timer
static uint32_t timerTaskStack[TINYOS_TIMERTASK_STACK_SIZE]; // Timer task stack

// after delayTicks, timer will be started, its cycle is durationTicks
// This function is invoked by customer
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
        timer->delayTicks = durationTicks;//保持和定时器的时钟一致，也将相当于保存了之前的定时值。感觉可有可无的
    }
    else
    {
        // already have delay at the beggining
        // this delayTicks will be --
        timer->delayTicks = timer->startDelayTicks;//等待delayTicks之后，定时器再开始工作
    }
    
    timer->state = tTimerCreated;
}                    

// Task execute function。其它所有task的timer需求都由这个任务去完成。
static void timerSoftTask(void* param)
{
    for (;;)
    {
        // This sem means that there is some timer timeout. Then here should check softList
        // 最终我们还是通过sysTick来定时的，HARD和Soft只是区别在systick中还是softTask中扫描list
        semWait(&timerTickSem, 0); // Wait forever
        
        semWait(&timerProtectSem, 0);
        
        // Handle softList, every cycle will check whether has timer timeout
        timerHandleList(&timerSoftList);
        
        // After using softList, release this ProtectSem soon.
        // For this ProtectSem only have 1 count, then 
        semNotify(&timerProtectSem);
    }
}

// Will be called in the sysTick irq
void timerModuleNotify(void)
{
    uint32_t stats = tTaskEnterCritical();
    
    // Two lists, one is checking in systick, the other one is checking in task loop
    // This list must be short and small
    timerHandleList(&timerHardList);
    
    tTaskExitCritical(stats);
    
    // wake up soft timer task, because it's waiting for this sem
    semNotify(&timerTickSem);
}

void timerStart(tTimer* timer)
{
    switch(timer->state)
    {
        case tTimerCreated:
        case tTimerStopped:
            // update this timer's state
            // judge if this timer needs to delay start
            timer->delayTicks = timer->startDelayTicks ? timer->startDelayTicks : timer->durationTicks;
            timer->state = tTimerStarted;
            
            if (timer->config & TIMER_CONFIG_TYPE_HARD)
            {
                // Here we only need critical section, not sem, because it's easy and fast
                // This hardList will also be accessed by sysTick
                uint32_t stats = tTaskEnterCritical();
                listAddFirst(&timerHardList, &timer->linkNode);
                tTaskExitCritical(stats);
            }
            else
            {
                // Here we use sem, not mutex, because sem is easy, normal count sem will be better.
                // softList will be accessed by this timer task and other tasks, so sem is better, not critical section
                semWait(&timerProtectSem, 0);
                listAddFirst(&timerSoftList, &timer->linkNode);
                semNotify(&timerProtectSem);
            }
        break;
        
        default:
            break;
    }
}

// These two start, stop function, assume only invoked by task
// If we want to start/stop timer in irq, then we should use critical section instead of sem
void timerStop(tTimer* timer)
{
    switch(timer->state)
    {
        case tTimerCreated:
        case tTimerStarted:
            if (timer->config & TIMER_CONFIG_TYPE_HARD)
            {
                uint32_t stats = tTaskEnterCritical();
                listRemove(&timerHardList, &timer->linkNode);
                tTaskExitCritical(stats);
            }
            else
            {
                // 只有一个信号量，用完必须立即释放
                semWait(&timerProtectSem, 0);
                listRemove(&timerHardList, &timer->linkNode);
                semNotify(&timerProtectSem);
            }
            
            timer->state = tTimerStopped;
            
            break;  
        default:
            break;
    }
}

// 删除和停止的效果是一样的，这是一个简化的操作。
void timerDestroy(tTimer* timer)
{
    timerStop(timer);
    timer->state = tTimerDestroyed;
}

void timerInfoGet(tTimer* timer, tTimerInfo* info)
{
    uint32_t stats = tTaskEnterCritical();
    
    info->startDelayTicks = timer->startDelayTicks;
    info->durationTicks = timer->durationTicks;
    info->timerFuc = timer->timerFuc;
    info->config = timer->config;
    info->state = timer->state;
    info->arg = timer->arg;
    
    tTaskExitCritical(stats);
}

// 遍历list(分为hard和soft的list)，查看该list中是否有timer到期了
void timerHandleList(tList* timerList)
{
    tNode* node;
    for (node = timerList->node.nextNode; node != &(timerList->node); node = node->nextNode)
    {
        tTimer* timer = tNodeParent(node, tTimer, linkNode);
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0))
        {
            timer->state = tTimerRunning;
            timer->timerFuc(timer->arg); // execute timer cb function
            timer->state = tTimerStarted;
            
            // check if timer is only running for once or running for cycle
            if (timer->durationTicks > 0) // this means this timer still needs to run
            {
                timer->delayTicks = timer->durationTicks;//表明下一个timer要启动时，至少得等durationTicks之后。
            }
            else
            {
                // 一次性的timer，从list中删掉，而不是删除timer本身。timer本身还是存在的。
                listRemove(timerList, &timer->linkNode);
                timer->state = tTimerStopped;
            }
        }
    }
}

// Only used by RTOS os, not for user
void timerModuleInit(void)
{
    listInit(&timerHardList);
    listInit(&timerSoftList);
    semInit(&timerProtectSem, 1, 1); // Only can get once
    semInit(&timerTickSem, 0, 0); // No count limit
}

void timerModuleTaskInit()
{
// Add judgment for the timer stack pri must higher than idle task
#if   TINYOS_TIMERTASK_PRI >= TINYOS_PRI_COUNT - 1
    #error "Timer task priority is not right"   // 这里只会在编译时报错。这样在编译时就可以发现代码错了
#endif    
    
    // 才开始我只用timerTaskStack，task没run的起来，现在改为&timerTaskStack[TINYOS_TIMERTASK_STACK_SIZE]则运行正常
    // 因为stask要从尾部传入！不能从头部传入，从尾部传入即大地址传入是一个向下生长的满栈
    tTaskInit(&timerTask, timerSoftTask, (void*)0, TINYOS_TIMERTASK_PRI, timerTaskStack, TINYOS_TIMERTASK_STACK_SIZE);
}


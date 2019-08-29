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
    timer->startDelayTicks = delayTicks;//��ʾ��Ҫ���ٸ�tick֮����ö�ʱ������
    timer->durationTicks = durationTicks;//timer
    timer->timerFuc = timerFuc;
    timer->arg = arg;
    timer->config = config;
    
    if (delayTicks == 0)//�������ö�ʱ������
    {
        timer->delayTicks = durationTicks;//���ֺͶ�ʱ����ʱ��һ�£�Ҳ���൱�ڱ�����֮ǰ�Ķ�ʱֵ���о����п��޵�
    }
    else
    {
        // already have delay at the beggining
        // this delayTicks will be --
        timer->delayTicks = timer->startDelayTicks;//�ȴ�delayTicks֮�󣬶�ʱ���ٿ�ʼ����
    }
    
    timer->state = tTimerCreated;
}                    

// Task execute function����������task��timer�������������ȥ��ɡ�
static void timerSoftTask(void* param)
{
    for (;;)
    {
        // This sem means that there is some timer timeout. Then here should check softList
        // �������ǻ���ͨ��sysTick����ʱ�ģ�HARD��Softֻ��������systick�л���softTask��ɨ��list
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
                // ֻ��һ���ź�����������������ͷ�
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

// ɾ����ֹͣ��Ч����һ���ģ�����һ���򻯵Ĳ�����
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

// ����list(��Ϊhard��soft��list)���鿴��list���Ƿ���timer������
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
                timer->delayTicks = timer->durationTicks;//������һ��timerҪ����ʱ�����ٵõ�durationTicks֮��
            }
            else
            {
                // һ���Ե�timer����list��ɾ����������ɾ��timer����timer�����Ǵ��ڵġ�
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
    #error "Timer task priority is not right"   // ����ֻ���ڱ���ʱ���������ڱ���ʱ�Ϳ��Է��ִ������
#endif    
    
    // �ſ�ʼ��ֻ��timerTaskStack��taskûrun�����������ڸ�Ϊ&timerTaskStack[TINYOS_TIMERTASK_STACK_SIZE]����������
    // ��ΪstaskҪ��β�����룡���ܴ�ͷ�����룬��β�����뼴���ַ������һ��������������ջ
    tTaskInit(&timerTask, timerSoftTask, (void*)0, TINYOS_TIMERTASK_PRI, timerTaskStack, TINYOS_TIMERTASK_STACK_SIZE);
}


#include "tinyOS.h"
#include "tLib.h"

tTask * currentTask;
tTask * nextTask;
tList   taskPriTable[TINYOS_PRI_COUNT]; // This list only store the tasks that is active
tList   delayList; // This list only store the tasks which are sleeping
tBitMap taskBitMap;


tTask tIdleTask;

uint32_t idleTaskEnv[TINYOS_STACK_SIZE];

uint32_t tickCount;
uint32_t idleCount;
uint32_t idleMaxCount;
uint8_t schedLockCount;
uint8_t enableCpuUsageState;
float cpuUsage; // 统计CPU的使用率

#if TINYOS_ENABLE_CPUUSAGE_STAT
void cpuCheckUsage(void);
#endif

void tTaskSchedInit()
{
    int i;
    schedLockCount = 0;
    bitMapInit(&taskBitMap);
    
    // Iniliaze the delay list, tasks in this list are all sleeping.
    listInit(&delayList);
    
    // Initialize all the lists in priority table. Every index has a list which used to store all the tasks that have same priority
    // Tasks in this list are all active.
    for (i=0; i<TINYOS_PRI_COUNT; i++)
    {
        listInit(&taskPriTable[i]);
    }
}

// All delayed tasks should be put into a list
//void taskDelayInit(void)
//{
//    listInit(&delayList);
//}


void tTaskSchedDisable()
{
    uint32_t status = tTaskEnterCritical();
    
    if (schedLockCount < 255)
    {
        schedLockCount++;
    }
    
    tTaskExitCritical(status);
}

void tTaskSchedEnable()
{
    uint32_t status = tTaskEnterCritical();
    
    if (schedLockCount > 0)
    {
        if (--schedLockCount == 0)
        {
            tTaskSched();
        }
    }
    
    tTaskExitCritical(status);
}

tTask* findHighestPriTask()
{
    uint8_t pos = bitMapGetFirstSet(&taskBitMap);
    tNode* node = listFirstNode(&taskPriTable[pos]);
    return tNodeParent(node, tTask, linkNode);
}

void taskSchedReady(tTask* task)
{
    // Then we need to set taskPriTable to mark this task as ready 
    // We can delete this sentance because we only use bitMap per the priority to search the task. We don't use taskPriTable to search task.
    listAddLast(&(taskPriTable[task->pri]), &(task->linkNode));
    bitMapSet(&taskBitMap, task->pri);
}

// When task is unready(for the reason of sleeping or something else), we need to remove it from the taskPriTable list
void taskSchedUnReady(tTask* task)
{
    // Then clear this task in the taskPriTable
    listRemove(&(taskPriTable[task->pri]), &(task->linkNode));
    if (listCount(&taskPriTable[task->pri]) == 0)
    {
        bitMapClear(&taskBitMap, task->pri);
    }
}

// Delete task from the sched task table
void taskSchedDelete(tTask* task)
{
    listRemove(&taskPriTable[task->pri], &task->linkNode);
    if (listCount(&taskPriTable[task->pri]) == 0)
    {
        bitMapClear(&taskBitMap, task->pri);
    }
}

// Delete task from the delay list
void taskDelayedDelete(tTask* task)
{
    listRemove(&delayList, &task->delayNode);
}


// When task is in sleep state, we need to add them into delayList
void timedTaskWait(tTask* task, uint32_t tick)
{
    // Add the delay task into delayList
    task->systemTickCount = tick;
    listAddLast(&delayList, &task->delayNode);
    task->state = TINYOS_TASK_STATE_DELAY;
}

// When task has waked up, then we need to remove it from the delayList
void timedTaskWakeUp(tTask* task)
{
    // First we need to remove the timeout task from the delayList
    listRemove(&delayList, &task->delayNode);
    task->state &= ~TINYOS_TASK_STATE_DELAY; 
}


// This schedual function can be invoked by irq or task, so needs to to be protected.
void tTaskSched()
{
    tTask* tempTask;
    
    // Make sure taskTable won't be chagned. Rule is global variable should be protected.
    uint32_t status = tTaskEnterCritical();
    
    // If task in locked, then we should never permit the task schedual
    if (schedLockCount > 0)
    {
        tTaskExitCritical(status);
        return;
    }

    // All tasks will be collected into array that every time core will find the highest priority task to execute
    tempTask = findHighestPriTask();
    
    if (currentTask != tempTask)
    {
        nextTask = tempTask;
        // In this function, currentTask will be assigned.
        taskSwitch();       
    }
    
    tTaskExitCritical(status);
}

void timerTickInit()
{
    tickCount = 0;
}

void tTaskSystemTickHandler()
{
    tNode* node;
    
    uint32_t status = tTaskEnterCritical();
        
    // Here only need to scan the tasks who need to delay. If tasks don't have delay, then we don't need to scan them
    for (node=delayList.node.nextNode; node!=&delayList.node; node=node->nextNode)
    {
        // From the delayNode we can find the its according task
        // If timeout, remove it from the delayList
        tTask* task = tNodeParent(node, tTask, delayNode);
        if (--task->systemTickCount == 0)
        {
            // This task is waiting for event
            if (task->waitEvent)
            {
                eventRemoveTask(task, (void*)0, ERROR_TIMEOUT);
            }
            
            // Find a currentTask
            timedTaskWakeUp(task);
            
            taskSchedReady(task);
        }
    }
    
    // Slice handle.
    // If slice is timeout, switch the tasks in the priority list. The first task in this list is the one that will be executed.
    if (--currentTask->slice == 0)
    {
        if (listCount(&taskPriTable[currentTask->pri]) > 0)
        {
            // Update the list to make sure that the first task in this list is the one that wants to be executed
            listRemoveFirst(&taskPriTable[currentTask->pri]);
            listAddLast(&taskPriTable[currentTask->pri], &currentTask->linkNode);
            currentTask->slice = TINYOS_TASK_CLICE_COUNT;
        }
    }

#if TINYOS_ENABLE_CPUUSAGE_STAT    
    cpuCheckUsage();
#endif    
    
    tTaskExitCritical(status);
 
#if TINYOS_ENABLE_TIMER    
    // check timer in hardList first, then in soft timer task check the other list
    timerModuleNotify();
#endif
    
    tTaskSched();
}

#if TINYOS_ENABLE_CPUUSAGE_STAT
void cpuUsageInitState()
{
    enableCpuUsageState = 0;
    idleCount = 0;
    idleMaxCount = 0;
    cpuUsage = 0.0f;
}

// 检查cpu使用率的核心函数,在时间节拍中去调用
void cpuCheckUsage(void)
{
    tickCount++;

    if (enableCpuUsageState == 0)
    {
        enableCpuUsageState = 1;
        // 开始统计tickCount
        tickCount = 0;
        return;
    }
    
    // 第一秒时统计出idleMaxCount
    if (tickCount == TICKS_PER_SEC)
    {
        idleMaxCount = idleCount;
        // 开始下一轮的统计，这里已经统计到了最大的idleMaxCount
        idleCount = 0;
        
        tTaskSchedEnable();
    }
    else if (tickCount % TICKS_PER_SEC == 0)
    {
        //以后的每一秒，都计算一下cpu的使用率
        cpuUsage = 100 - idleCount  * 100.0 / idleMaxCount;
        idleCount = 0; // 开始下一轮的计数
    }
}

void cpuUsageSyncWithSysTick()
{
    // when systick didn't interrupt, we will wait here. Aim to Sync with the finish timestamp with systick
    while (enableCpuUsageState == 0)
    {
        ;
    }
}

float cpuUsageInfoGet(void)
{
    float usage = 0.0f;
    uint32_t stats = tTaskEnterCritical();
    
    usage = cpuUsage;
    
    tTaskExitCritical(stats);
    
    return usage;
}
#endif 

void idleTaskEntry(void* param)
{
    // 使用调度锁阻止任务调度，在第一次运行该idle entry时有效.禁掉别的task，只允许运行idle task，这样可以计算1s内最大systick数
    tTaskSchedDisable();
    
    // 初始化task的init都放在这里，等idle task计算完运行时间后，再去启动这些task。
    initApp();
 
#if TINYOS_ENABLE_TIMER     
    timerModuleTaskInit();
#endif
    
    tSetSysTickPeriod(TINYOS_SYSTICK_MS);// Every 10ms we will get a sysTick interrupt
  
#if TINYOS_ENABLE_CPUUSAGE_STAT    
    // 时钟同步：在systick开始时，我们的idleCount也开始++，这样CPU的运行时间统计的更准确一些。如果在systick中间开始统计，则误差较大。
    cpuUsageSyncWithSysTick();
#endif
    
    // 统计idle任务的运行时间
    for(;;)
    { 
        uint32_t stats = tTaskEnterCritical();
        idleCount++;
        tTaskExitCritical(stats);
    }
}

int main()
{
    // Init the sched lock
    tTaskSchedInit();
    
#if TINYOS_ENABLE_TIMER 
    timerModuleInit();
#endif
    
    timerTickInit();//这是时间相关的，所以单独拎出来
 
#if TINYOS_ENABLE_CPUUSAGE_STAT    
    cpuUsageInitState();
#endif    
    
    tTaskInit(&tIdleTask,   idleTaskEntry, (void*)0, TINYOS_PRI_COUNT - 1, idleTaskEnv, TINYOS_STACK_SIZE); // idle task的优先级是最低的：TINYOS_PRI_COUNT - 1
       
    nextTask = findHighestPriTask();
    
    tTaskRunFirst();
    
    // will never run to this
    return 0;
}

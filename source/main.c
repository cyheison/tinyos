#include "tinyOS.h"
#include "tLib.h"

tTask * currentTask;
tTask * nextTask;
tTask * taskPriTable[TINYOS_PRI_COUNT];
tBitMap taskBitMap;

tTask tTask1;
tTask tTask2;
tTask tIdleTask;

uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t idleTaskEnv[1024];

uint32_t tickCount;

uint8_t schedLockCount;

tList delayList;

void tTaskSchedInit()
{
    schedLockCount = 0;
    bitMapInit(&taskBitMap);
}

// All delayed tasks should be put into a list
void taskDelayInit(void)
{
    listInit(&delayList);
}


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
            tTaskSchedual();
        }
    }
    
    tTaskExitCritical(status);
}

tTask* findHighestPriTask()
{
    uint8_t pos = bitMapGetFirstSet(&taskBitMap);
    return taskPriTable[pos];
}

void timedTaskWait(tTask* task, uint32_t tick)
{
    // Add the delay task into delayList
    task->systemTickCount = tick;
    listAddLast(&delayList, &task->delayNode);
    task->state = TINYOS_TASK_STATE_DELAY;
    
    // Then clear this task in the taskPriTable
    // We can delete this sentance
    taskPriTable[task->pri] = (tTask*)0;
    bitMapClear(&taskBitMap, task->pri);
}

void timedTaskWakeUp(tTask* task)
{
    // First we need to remove the timeout task from the delayList
    listRemove(&delayList, &task->delayNode);
    task->state &= ~TINYOS_TASK_STATE_DELAY; 
    
    // Then we need to set taskPriTable to mark this task as ready 
    // We can delete this sentance because we only use bitMap per the priority to search the task. We don't use taskPriTable to search task.
    taskPriTable[task->pri] = task;
    bitMapSet(&taskBitMap, task->pri);
}


// This schedual function can be invoked by irq or task, so needs to to be protected.
void tTaskSchedual()
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

void setTaskDelay(uint32_t delay)
{
    uint32_t status = tTaskEnterCritical();
    
    // We put this timed delay task into delayList
    timedTaskWait(currentTask, delay);
    
    tTaskExitCritical(status);
    
    tTaskSchedual();// When current task is time delay, we should switch to another task to execute immediately
}

void tSetSysTickPeriod(uint32_t ms)
{
    SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
    NVIC_SetPriority(SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk | 
                    SysTick_CTRL_ENABLE_Msk;
}

void tTaskSystemTickHandler()
{
    tNode* node;
    
    uint32_t status = tTaskEnterCritical();
    
    // Here only need to scan the tasks who need to delay. If tasks don't have delay, then we don't need to scan them
    for (node=delayList.node.nextNode; node!=&delayList.node; node=node->nextNode)
    {
        // From the delayNode we can find the its according task
        tTask* task = tNodeParent(node, tTask, delayNode);
        if (--task->systemTickCount == 0)
        {
            timedTaskWakeUp(task);
        }
    }
    
    tTaskExitCritical(status);
    
    tTaskSchedual();
}

void SysTick_Handler()
{
    tTaskSystemTickHandler();
}


//task point; entry function; param; stack space.
void tTaskInit(tTask *task, void (*entry)(void*), void* param, uint32_t pri, uint32_t *stack)
{   
    //init the stack. when converting task, these values will pop into registers
    //will be stored automaticaly
    *(--stack) = (uint32_t)(1<<24);//xPSR
    *(--stack) = (uint32_t)entry;//PC
    *(--stack) = (uint32_t)0x14;//LR
    *(--stack) = (uint32_t)0x12;
    *(--stack) = (uint32_t)0x3;
    *(--stack) = (uint32_t)0x2;
    *(--stack) = (uint32_t)0x1;
    *(--stack) = (uint32_t)param;
    
    // need to store manually
    *(--stack) = (uint32_t)0x11;
    *(--stack) = (uint32_t)0x10;
    *(--stack) = (uint32_t)0x9;
    *(--stack) = (uint32_t)0x8;
    *(--stack) = (uint32_t)0x7;
    *(--stack) = (uint32_t)0x6;
    *(--stack) = (uint32_t)0x5;
    *(--stack) = (uint32_t)0x4;
    
    // now stack addr is &tTask->stack[1024] - 4 * 16. So when switching task, we need to use LDMIA to restore these registers' value
    task->stack = stack;
    task->systemTickCount = 0;
    
    // Init pri
    task->pri = pri;
    
    // Init taskBitMap
    bitMapSet(&taskBitMap, pri);
    
    // Init taskPriTable   
    taskPriTable[pri] = task;
}

int task1Flag;
void task1Entry(void* param)
{

    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt
    for(;;)
    {      
        task1Flag = 0;        
        // This delay will cause the task sched
        setTaskDelay(1); 
        task1Flag = 1;
        setTaskDelay(1);
    }
}

int task2Flag;
void task2Entry(void *param)
{
    for(;;)
    {
        task2Flag = 0;
        setTaskDelay(1);
        task2Flag = 1;
        setTaskDelay(1);
    }   
}

void idleTaskEntry(void* param)
{
    for(;;)
    { 
    }    
}

int main()
{
    // Init the sched lock
    tTaskSchedInit();
    
    taskDelayInit();
    
    // Init tasks
    tTaskInit(&tTask1,      task1Entry, (void*)0x11111111, 0, &task1Env[1024]);
    tTaskInit(&tTask2,      task2Entry, (void*)0x22222222, 1, &task2Env[1024]);
    tTaskInit(&tIdleTask,   idleTaskEntry, (void*)0, TINYOS_PRI_COUNT - 1, &idleTaskEnv[1024]);
       
    nextTask = findHighestPriTask();
    
    tTaskRunFirst();
    
    // will never run to this
    return 0;
}

#include "tinyOS.h"

tTask * currentTask;
tTask * nextTask;
tTask * idleTask;
tTask * taskTable[2];

tTask tTask1;
tTask tTask2;
tTask tIdleTask;

uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t idleTaskEnv[1024];

uint32_t tickCount;

// This schedual function can be invoked by irq or task, so needs to to be protected.
void tTaskSchedual()
{
    // Make sure taskTable won't be chagned. Rule is global variable should be protected.
    uint32_t status = tTaskEnterCritical();

    // To decide which task is the next task
    if (currentTask == idleTask)
    {
        if (taskTable[0]->systemTickCount == 0)
        {
            nextTask = taskTable[0];
        }
        else if (taskTable[1]->systemTickCount == 0)
        {
            nextTask = taskTable[1];
        }
        else
        {
            // Here is return, so must exit the critical sector
            tTaskExitCritical(status);
            return;
        }
    }
    else 
    {
        if (currentTask == taskTable[0])
        {
            if (taskTable[1]->systemTickCount == 0)
            {
                nextTask = taskTable[1];
            }
            else if (currentTask->systemTickCount > 0)
            {
                nextTask = idleTask;
            }
            else
            {
                tTaskExitCritical(status);
                return;
            }
        }
        else if (currentTask == taskTable[1])
        {
            if (taskTable[0]->systemTickCount == 0)
            {
                nextTask = taskTable[0];
            }
            else if (currentTask->systemTickCount > 0)
            {
                nextTask = idleTask;
            }
            else
            {
                tTaskExitCritical(status);
                return;
            }
        }
    }
    
    taskSwitch();
    
    tTaskExitCritical(status);
}

void setTaskDelay(uint32_t delay)
{
    uint32_t status = tTaskEnterCritical();
    currentTask->systemTickCount = delay;
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
    int i;
    
    uint32_t status = tTaskEnterCritical();
    for(i=0; i<2; i++)
    {
        if(taskTable[i]->systemTickCount > 0)
        {
            taskTable[i]->systemTickCount--;
        }
    }
    
    tTaskExitCritical(status);
    
    tTaskSchedual();
}

void SysTick_Handler()
{
    tTaskSystemTickHandler();
    
    // Doesn't have to put EnterCritical here because task will never preempt irq in this demo
    tickCount++;
}

int task1Flag;
void task1Entry(void* param)
{
    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt
    for(;;)
    {
        int i;
        int count = tickCount;
        uint32_t status = tTaskEnterCritical();
        
        for(i=0; i<0xffff; i++){}
        count = count + 1;
        
        tTaskExitCritical(status);
            
        task1Flag = 0;
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

//task point; entry function; param; stack space.
void tTaskInit(tTask *task, void (*entry)(void*), void* param, uint32_t *stack)
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
}


int main()
{
    tTaskInit(&tTask1, task1Entry, (void*)0x11111111, &task1Env[1024]);
    tTaskInit(&tTask2, task2Entry, (void*)0x22222222, &task2Env[1024]);
    tTaskInit(&tIdleTask, idleTaskEntry, (void*)0, &idleTaskEnv[1024]);
    
    taskTable[0] = &tTask1;
    taskTable[1] = &tTask2;
    
    idleTask = &tIdleTask;
    
    nextTask = taskTable[0];
    
    tTaskRunFirst();
    
    // will never run to this
    return 0;

}

#include "tinyOS.h"

void delay (int count)
{
	while (--count > 0);
}

tTask * currentTask;
tTask * nextTask;
tTask * taskTable[2];

tTask tTask1;
tTask tTask2;

uint32_t task1Env[1024];
uint32_t task2Env[1024];

void tTaskSchedual()
{
    if (currentTask == taskTable[0])
    {
        nextTask = taskTable[1];
    }
    else
    {
        nextTask = taskTable[0];
    }
    
    taskSwitch();
}

void tSetSysTickPeriod(uint32_t ms)
{
    SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
    NVIC_SetPriority(SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk | 
                    SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler()
{
    tTaskSchedual();
}

int task1Flag;
void task1Entry(void* param)
{
    tSetSysTickPeriod(10);
    for(;;)
    {
        task1Flag = 0;
        delay(100);
        task1Flag = 1;
        delay(100);
    }
}

int task2Flag;
void task2Entry(void *param)
{
    for(;;)
    {
        task2Flag = 0;
        delay(100);
        task2Flag = 1;
        delay(100);
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

}


int main()
{
    tTaskInit(&tTask1, task1Entry, (void*)0x11111111, &task1Env[1024]);
    tTaskInit(&tTask2, task2Entry, (void*)0x22222222, &task2Env[1024]);
    
    taskTable[0] = &tTask1;
    taskTable[1] = &tTask2;
    
    nextTask = taskTable[0];
    
    tTaskRunFirst();
    
    // will never run to this
    return 0;

}

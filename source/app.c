#include "tinyos.h"

tTask tTask1;
tTask tTask2;
tTask tTask3;

uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t task3Env[1024];

void timedDelay()
{
    int i;
    
    for (i=0; i<0xff; i++){}
}

int task1Flag;
void task1Entry(void* param)
{

    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt
    for(;;)
    {      
        task1Flag = 0;        
        // To make sure this task is running per the slice
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
        timedDelay();
        task2Flag = 1;
        timedDelay();
    }   
}

int task3Flag;
void task3Entry(void *param)
{
    for(;;)
    {
        task3Flag = 0;
        timedDelay();
        task3Flag = 1;
        timedDelay();
    }   
}

void initApp()
{
    // Init tasks
    tTaskInit(&tTask1,      task1Entry, (void*)0x11111111, 0, &task1Env[1024]);
    tTaskInit(&tTask2,      task2Entry, (void*)0x22222222, 1, &task2Env[1024]);
    tTaskInit(&tTask3,      task3Entry, (void*)0x33333333, 1, &task3Env[1024]);
   
}

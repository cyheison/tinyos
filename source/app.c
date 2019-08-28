#include "tinyos.h"

tTask tTask1; // task1 will remove all of the event
tTask tTask2; // task2,3,4 will be added into the same event
tTask tTask3;
tTask tTask4; 


uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t task3Env[1024];
uint32_t task4Env[1024];

tTimer timer1;
tTimer timer2;
tTimer timer3;

uint32_t bit1 = 0;
uint32_t bit2 = 0;
uint32_t bit3 = 0;

void timerFunc(void* arg)
{
    uint32_t* ptrBit = (uint32_t*)arg;
    *ptrBit ^= 0x1;//toggle its lowest bit
}

int task1Flag;
void task1Entry(void* param)
{    
    uint8_t stopped = 0;
    
    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt

    timerInit(&timer1, 100, 10, timerFunc, (void*)&bit1, TIMER_CONFIG_TYPE_HARD);//dealy 100ms to start timer, every 10ms timer irq,
    timerStart(&timer1);
    timerInit(&timer2, 200, 20, timerFunc, (void*)&bit2, TIMER_CONFIG_TYPE_HARD);
    timerStart(&timer2);
    timerInit(&timer3, 300, 0, timerFunc, (void*)&bit3, TIMER_CONFIG_TYPE_SOFT);// only lasts for 300 ms, then this timer will be stopped
    timerStart(&timer3);
    // When task1 has been destroyed, then task1 can go here
    for(;;)
    {          
        task1Flag = 0;
        // To make sure this task is running per the slice
        setTaskDelay(1);
        task1Flag = 1;
        setTaskDelay(1);
        
        if (stopped == 0 )
        {
            setTaskDelay(200);
            // stop timer1 first
            timerStop(&timer1);
            stopped = 1;
        }

    }
}

int task2Flag;
void task2Entry(void *param)
{   
    
    for(;;)
    {

        task2Flag = 0;
        // To make sure this task is running per the slice
        setTaskDelay(1);
        task2Flag = 1;
        setTaskDelay(1);
        

    }   
}

int task3Flag;
void task3Entry(void *param)
{   

    for(;;)
    {
        task3Flag = 0;
        setTaskDelay(1);
        task3Flag = 1;
        setTaskDelay(1);
    }   
}

int task4Flag;
void task4Entry(void *param)
{    
   
    for(;;)
    {
        task4Flag = 0;
        setTaskDelay(1);
        task4Flag = 1;
        setTaskDelay(1);
    }   
}

void initApp()
{
    // Init tasks
    tTaskInit(&tTask1,      task1Entry, (void*)0x11111111, 0, &task1Env[1024]);
    tTaskInit(&tTask2,      task2Entry, (void*)0x22222222, 1, &task2Env[1024]);
    tTaskInit(&tTask3,      task3Entry, (void*)0x33333333, 1, &task3Env[1024]);
    tTaskInit(&tTask4,      task4Entry, (void*)0x44444444, 1, &task4Env[1024]);

}

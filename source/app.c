#include "tinyos.h"

tTask tTask1; // task1 will remove all of the event
tTask tTask2; // task2,3,4 will be added into the same event
tTask tTask3;
tTask tTask4; 


uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t task3Env[1024];
uint32_t task4Env[1024];

uint8_t mem[20][100];
tMemBlock memBlock;


int task1Flag;
void task1Entry(void* param)
{    
    memBlockInfo info;
    int i;
    uint8_t* mem1;
    
    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt
 
    memBlockInit(&memBlock, (uint8_t*)mem, 100, 20);
    memInfoGet(&memBlock, &info);

    for (i = 0; i < 20; i++)
    {
        // Get all the blocks in the mem block
        memBlockWait(&memBlock, (uint8_t**)&mem1, 0);
    }
    
    // here task1 will enter waiting state.
    memBlockWait(&memBlock, (uint8_t**)&mem1, 0);

    // When task1 has been destroyed, then task1 can go here
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
    int destroyed = 0;
    
    for(;;)
    {      
        if (!destroyed)
        {
            // delete the task1 which is waiting for memBlock
            memBlockDestroy(&memBlock);
            destroyed = 1;
        }
        
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

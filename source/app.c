#include "tinyos.h"

tTask tTask1;
tTask tTask2; // In task2, we will force delete task1
tTask tTask3;
tTask tTask4; // In task 4, we will request delete task3


uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t task3Env[1024];
uint32_t task4Env[1024];

int task1Flag;
void task1Clean(void* param)
{
    // Simulate the clean function
    task1Flag = 0;
}

void task1Entry(void* param)
{
    taskRegisterCleanFunc(currentTask, task1Clean, (void*)0);
    
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
    // Used to mark that whether task1 has been deleted
    uint8_t deleted = 0;
    
    for(;;)
    {
        task2Flag = 0;
        setTaskDelay(1);
        task2Flag = 1;
        setTaskDelay(1);
        
        if (!deleted)
        {
            taskForceDelete(&tTask1);
            deleted = 1;
        }
    }   
}

int task3Flag;
void task3Clean(void* param)
{    
    task3Flag = 0;
}

void task3Entry(void *param)
{
    taskRegisterCleanFunc(currentTask, task3Clean, (void*)0);
    for(;;)
    {
        task3Flag = 0;
        setTaskDelay(1);
        task3Flag = 1;
        setTaskDelay(1);
        
        if (taskRequestDeleteFlag(&tTask3))
        {
            taskDeleteSelf(&tTask3);
        }
    }   
}

int task4Flag;
void task4Entry(void *param)
{
    uint8_t deleted = 0;
    
    for(;;)
    {
        task4Flag = 0;
        setTaskDelay(1);
        task4Flag = 1;
        setTaskDelay(1);
        
        if (!deleted)
        {
            // Request to delete task3. So in task3 it needs to query that whether it has been requested to be deleted.
            taskRequestDelete(&tTask3);
            deleted = 1;
        }
    }   
}

void initApp()
{
    // Init tasks
    tTaskInit(&tTask1,      task1Entry, (void*)0x11111111, 0, &task1Env[1024]);
    tTaskInit(&tTask2,      task2Entry, (void*)0x22222222, 1, &task2Env[1024]);
    tTaskInit(&tTask3,      task3Entry, (void*)0x33333333, 0, &task3Env[1024]);
    tTaskInit(&tTask4,      task4Entry, (void*)0x44444444, 1, &task4Env[1024]);

}

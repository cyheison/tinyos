#include "tinyos.h"

tTask tTask1; // task1 will remove all of the event
tTask tTask2; // task2,3,4 will be added into the same event
tTask tTask3;
tTask tTask4; 


uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t task3Env[1024];
uint32_t task4Env[1024];

tMuxte mutex;

int task1Flag;
void task1Entry(void* param)
{    
   
    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt
    
    mutexInit(&mutex);

    // When task1 has been destroyed, then task1 can go here
    for(;;)
    {          
        mutexWait(&mutex, 0);//第一次可以get到mutex
        mutexWait(&mutex, 0);//第二次发现owner是自己，仅仅做一个lockedCount++的操作，此时waitlist中没有node
        
        task1Flag = 0;
        // To make sure this task is running per the slice
        setTaskDelay(1);
        task1Flag = 1;
        setTaskDelay(1);
        
        mutexNotify(&mutex);// Here only -- lockedCount, won't wake up task2
        mutexNotify(&mutex);// Here mutex owner will change to task2，但是task2的优先级低于task1，所以该mutex的owner是task2，但程序还是运行在task1中
                            //但此时会提升task2的优先级。并把task1放入waitlist中。
    }
}

int task2Flag;
void task2Entry(void *param)
{   

    
    for(;;)
    {
        mutexWait(&mutex, 0);//task1 lock了两次，所以这次不会增加lockedCount，因为获取不到该mutex，只是在waitList中留下了记录
        mutexWait(&mutex, 0);
        
        task2Flag = 0;
        // To make sure this task is running per the slice
        setTaskDelay(1);
        task2Flag = 1;
        setTaskDelay(1);
        
        mutexNotify(&mutex);
        mutexNotify(&mutex);
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

#include "tinyos.h"

tTask tTask1; // task1 will remove all of the event
tTask tTask2; // task2,3,4 will be added into the same event
tTask tTask3;
tTask tTask4; 


uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t task3Env[1024];
uint32_t task4Env[1024];

tSem sem1; // task1 will waiting sem1, and task2 will check its info then remove it.

tMbox mBox1;// task2 will destroy mBox1, so task1 can continue running.
void* mBoxMsgBuffer1[20];// store pointer in this array
uint32_t msg1[20];


tMbox mBox2;// task3 notify msg at mBox2 and task4 will for this mBox2, when task4 get the msg from mBox2 then it will clean up all the msgs at mBox2.
void* mBoxMsgBuffer2[20];// store pointer in this array
uint32_t msg2[20];

int task1Flag;
void task1Entry(void* param)
{
    void* msg;
    
    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt
    
    mBoxInit(&mBox1, mBoxMsgBuffer1, 20);// 数组的首地址用void*修饰表明数组中存的都是void*变量。
    mBoxInit(&mBox2, mBoxMsgBuffer2, 20);// 数组的首地址用void*修饰表明数组中存的都是void*变量。

    mBoxWait(&mBox1, (void*)msg, 0); // task1 will be waiting for the mBox1
    
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
    uint8_t destroyed = 0;
    
    for(;;)
    {      
        task2Flag = 0;
        setTaskDelay(1);
        task2Flag = 1;
        setTaskDelay(1);
        
        if (!destroyed)
        {
            mBoxRemoveTask(&mBox1);
            destroyed = 1;
        }
    }   
}

int task3Flag;
void task3Entry(void *param)
{   
    for(;;)
    {
        int i = 0;
        for (i = 0; i < 20; i++)
        {
            msg2[i] = i;
            mBoxNotify(&mBox2, &msg2[i], tMOXSendFront);
        }
        
        
        task3Flag = 0;
        setTaskDelay(1);
        task3Flag = 1;
        setTaskDelay(1);
    }   
}

int task4Flag;
void task4Entry(void *param)
{    
    void *msg;
    
    for(;;)
    {
        mBoxWait(&mBox2, (void*)&msg, 0);
        task4Flag = *(uint32_t*)msg;
        
        mBoxFlush(&mBox2);
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

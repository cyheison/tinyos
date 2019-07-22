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

tMbox mBox;
tMbox mBox2;
void* mBoxMsgBuffer[20];// store pointer in this array
void* mBoxMsgBuffer2[20];// store pointer in this array

int task1Flag;

uint32_t msg[20];

void task1Entry(void* param)
{  
    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt
    
    mBoxInit(&mBox, (void*)mBoxMsgBuffer, 20);// 数组的首地址用void*修饰表明数组中存的都是void*变量。

    for(;;)
    {   
        
        uint32_t i;
        for (i=0; i< 20; i++)
        {
            msg[i] = i;
            mBoxNotify(&mBox, &msg[i], tMOXSendNormal);
        }
        
        setTaskDelay(100); // To make sure upper's writing have been all read by task2
        
        for (i=0; i<20; i++)
        {
            msg[i] = i;
            mBoxNotify(&mBox, &msg[i], tMOXSendFront);
        }
        
        setTaskDelay(100);
        
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
        void* msgg;
        uint8_t errNo = mBoxWait(&mBox, &msgg, 0); // 0 means no timeout
        if (errNo == ERROR_NOERROR)
        {
            uint32_t value = *(uint32_t*)msgg;
            task2Flag = value; // watch task1's value which is passed in 
            setTaskDelay(1);
        }       
        
        task2Flag = 0;
        setTaskDelay(1);
        task2Flag = 1;
        setTaskDelay(1);
    }   
}

int task3Flag;
void task3Entry(void *param)
{
    mBoxInit(&mBox2, mBoxMsgBuffer2, 20); // Wait until timeout   
    
    for(;;)
    {
        void* msgg;
        mBoxWait(&mBox2, &msgg, 100);
        
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

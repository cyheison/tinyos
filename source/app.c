#include "tinyos.h"

tTask tTask1; // task1 will remove all of the event
tTask tTask2; // task2,3,4 will be added into the same event
tTask tTask3;
tTask tTask4; 


uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t task3Env[1024];
uint32_t task4Env[1024];

typedef uint8_t (*tBlock)[100];//����һ������ָ�����ͣ�ָ��100byte�Ĵ洢������׵�ַ
uint8_t mem1[20][100]; // every mem size is 100byte, total is 20
tMemBlock memBlock;


int task1Flag;
void task1Entry(void* param)
{
    uint8_t i;
    tBlock block[20];//����20���洢tBlock ������
    
    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt
 
    // �����ǽ���ά���鵱��һά����������������ά����ĵ�ַ����һά����ĵ�ַ��������Ϊ�����в�û�жԶ�ά����ȡֵ��ֻ������һ�¸�����ĵ�ֵַ��
    // ����Ϊ�õ�ַ�Ƕ�άָ�룬����Ҫ��uint8_t*������һ�¡���init�����п���ȡ���õ�ַ������ˡ�
    memBlockInit(&memBlock, (uint8_t*)mem1, 100, 20);
    
    for (i=0; i<20; i++)
    {
        // Get all the values in memBlock->blockList. every value will be stored in block[i], so every element is a tBlock
        //&block[i]�������Ǹ���άָ�룬���Կ�����uint8_t**���Ρ�
        //���Ȼ�ȡ���еĴ洢�飬��ʼֵ����0
        memBlockWait(&memBlock, (uint8_t**)&block[i], 0);
    }
    
    setTaskDelay(2);

    //���洢�鸳ֵ���͸�task2
    for (i = 0; i < 20; i++)
    {
        //��Ϊblock[i]�д����ָ�룬ÿ��ָ�붼ָ����һ��100byte������
        // �����ǽ���ά����ȫ����ʼ��һ�飬ÿһ�ж���11111, 22222, 33333,������ȥ�����Ǹ���ά�����ʼ���ĺ÷�����ʹ������ָ�롣
        memset(block[i], i, 100);
        // Notify ��Ҫ����ָ�룬����block[i]��һ��ָ�����顣
        //���ﴫ���ָ�룬��Ҫ��ָ��ȡ��ַ����Ϊnotify������û�н���ȡ��ַ�Ĳ�����ָ��ԭ����ʲô������notify�����л���ԭ������ֻ�Ǽ��˸�(uint8_t*��void*)�����ζ��ѡ�
        memBlockNotify(&memBlock, (uint8_t*)block[i]);
        setTaskDelay(1);
    }
    
    
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
    
    for(;;)
    {      
        //In task2 will get the msg notified by task1
        tBlock block;
        memBlockWait(&memBlock, (uint8_t**)&block, 0);
        // only print the first value in this section
        task2Flag = *(uint8_t*)block;
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

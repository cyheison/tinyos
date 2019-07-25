#include "tinyos.h"

tTask tTask1; // task1 will remove all of the event
tTask tTask2; // task2,3,4 will be added into the same event
tTask tTask3;
tTask tTask4; 


uint32_t task1Env[1024];
uint32_t task2Env[1024];
uint32_t task3Env[1024];
uint32_t task4Env[1024];

typedef uint8_t (*tBlock)[100];//定义一个数组指针类型，指向100byte的存储区域的首地址
uint8_t mem1[20][100]; // every mem size is 100byte, total is 20
tMemBlock memBlock;


int task1Flag;
void task1Entry(void* param)
{
    uint8_t i;
    tBlock block[20];//定义20个存储tBlock 的数组
    
    tSetSysTickPeriod(10);// Every 10ms we will get a sysTick interrupt
 
    // 这里是将二维数组当做一维数组来看待。将二维数组的地址当做一维数组的地址看待，因为函数中并没有对二维数组取值，只是用了一下该数组的地址值。
    // 但因为该地址是二维指针，所以要用uint8_t*来修饰一下。在init函数中可以取出该地址随便用了。
    memBlockInit(&memBlock, (uint8_t*)mem1, 100, 20);
    
    for (i=0; i<20; i++)
    {
        // Get all the values in memBlock->blockList. every value will be stored in block[i], so every element is a tBlock
        //&block[i]本来就是个二维指针，所以可以用uint8_t**修饰。
        //首先获取所有的存储块，初始值都是0
        memBlockWait(&memBlock, (uint8_t**)&block[i], 0);
    }
    
    setTaskDelay(2);

    //将存储块赋值后发送给task2
    for (i = 0; i < 20; i++)
    {
        //因为block[i]中存的是指针，每个指针都指向了一个100byte的数组
        // 这里是将二维数组全部初始化一遍，每一行都是11111, 22222, 33333,这样下去。这是给二维数组初始化的好方法，使用数组指针。
        memset(block[i], i, 100);
        // Notify 需要传送指针，所以block[i]是一个指针数组。
        //这里传入的指针，不要给指针取地址，因为notify函数就没有进行取地址的操作，指针原来是什么样，在notify函数中还是原样处理，只是加了个(uint8_t*或void*)的修饰而已。
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

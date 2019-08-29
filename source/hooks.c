#include "hooks.h"

#if TINYOS_ENABLE_HOOKS
void hooksCpuIdle(void)
{

}

void hookSysTick(void)
{

}

void hooksTaskSwitch(tTask* from, tTask* to)
{

}

void hooksTaskInit(tTask* task)
{
    //当我还想做一些别的事情时，都可以把代码添加到这里来
}
#endif

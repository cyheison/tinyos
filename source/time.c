#include "tinyos.h"

void setTaskDelay(uint32_t delay)
{
    uint32_t status = tTaskEnterCritical();
    
    // We put this timed delay task into delayList
    timedTaskWait(currentTask, delay);
    
    taskSchedUnReady(currentTask);
    
    tTaskExitCritical(status);
    
    tTaskSchedual();// When current task is time delay, we should switch to another task to execute immediately
}



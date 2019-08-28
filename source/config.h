#ifndef CONFIG_H
#define CONFIG_H

#define TINYOS_PRI_COUNT            32

#define TINYOS_TASK_CLICE_COUNT     10

#define TINYOS_STACK_SIZE           1024

#define TINYOS_TIMERTASK_STACK_SIZE 1024 // timer task stack size
#define TINYOS_TIMERTASK_PRI        1    // must higher than idle task

#endif

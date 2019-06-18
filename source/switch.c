#include "tinyOS.h"

__asm void PendSV_Handler(void)
{
    IMPORT currentTask;
    IMPORT nextTask;
    
    MRS R0, PSP
    CBZ R0, PendSVHandler_nosave
    STMDB R0!, {R4-R11}
    LDR R1, =currentTask
    LDR R1, [R1]
    STR R0, [R1]

        
PendSVHandler_nosave
    LDR R0, =currentTask
    LDR R1, =nextTask
    LDR R2, [R1]
    STR R2, [R0]
    
    LDR R0, [R2]
    LDMIA R0!, {R4-R11}
    
    MSR PSP, R0
    ORR LR, LR, #0x04
    BX LR
}

void taskSwitch()
{
    mem32(NVIC_INT_CTRL) = NVIC_PENDSVSET;

}

void tTaskRunFirst()
{
    __set_PSP(0);
    
    mem32(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;
    mem32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

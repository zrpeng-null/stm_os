    EXPORT  CPU_SR_Save    
    EXPORT  CPU_SR_Restore

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

CPU_SR_Save
    MRS     R0, PRIMASK     
    CPSID   I
    BX      LR

CPU_SR_Restore
    MSR     PRIMASK, R0
    BX      LR

    END

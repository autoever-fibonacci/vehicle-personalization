#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"

#include "UART_Config.h"
#include "Driver_Stm.h"
#include "App_Scheduler.h"

IFX_ALIGN(4) IfxCpu_syncEvent cpuSyncEvent = 0;

int core0_main(void)
{
    uint16 endinitPw;

    endinitPw = IfxScuWdt_getCpuWatchdogPassword();
    IfxScuWdt_disableCpuWatchdog(endinitPw);

    endinitPw = IfxScuWdt_getSafetyWatchdogPassword();
    IfxScuWdt_disableSafetyWatchdog(endinitPw);

    IfxCpu_enableInterrupts();

    IfxCpu_emitEvent(&cpuSyncEvent);
    IfxCpu_waitEvent(&cpuSyncEvent, 1);

    init_UART();
    Driver_Stm_Init();

    App_Init();

    UART_Printf("\r\n[BOOT] ECU start\r\n");

    while (1)
    {
        AppScheduling();
    }

    return 0;
}

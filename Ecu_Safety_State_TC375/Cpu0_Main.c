#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"

#include "Bsp.h"
#include "UART_Config.h"
#include "App_Scheduler.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

void core0_main(void)
{
    IfxCpu_enableInterrupts();

    /* prototyping 용 */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    init_UART();
    App_Scheduler_Init();

    UART_Printf("\r\n");
    UART_Printf("[TC375][TEST] boot done\r\n");
    UART_Printf("[TC375][TEST] dummy profile table sent, wait AB_PROFILE_IDX\r\n");

    while (1)
    {
        App_Scheduler_Run();
    }
}

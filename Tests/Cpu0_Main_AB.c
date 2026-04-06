#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"

#include "Bsp.h"
#include "UART_Config.h"
#include "Driver_Stm.h"
#include "App_Scheduler.h"

IfxCpu_syncEvent cpuSyncEvent = 0;

void core0_main(void)
{
    IfxCpu_enableInterrupts();

    /* prototyping 용 */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* 프로젝트 실제 초기화 함수명에 맞춰 사용 */
    Driver_Stm_Init();
    UART_Init();

    App_Init();

    UART_Printf("\r\n");
    UART_Printf("[TC275][TEST] boot done\r\n");
    UART_Printf("[TC275][TEST] wait first RFID tag in SLEEP\r\n");

    while (1)
    {
        AppScheduling();
    }
}
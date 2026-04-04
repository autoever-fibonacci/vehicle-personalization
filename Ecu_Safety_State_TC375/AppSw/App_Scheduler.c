#include "App_Scheduler.h"

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "Base_Driver_Stm.h"

/* TODO: 필요한 모듈들 추가 */
#include "App_Manager_System.h"
#include "App_Manager_Temp.h"
#include "MCMCAN_FD.h"

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
static void App_Scheduler_Run_1ms(void);
static void App_Scheduler_Run_10ms(void);
static void App_Scheduler_Run_100ms(void);
static void App_Scheduler_Run_1s(void);
static void App_Scheduler_Run_10s(void);

/*********************************************************************************************************************/
/*------------------------------------------------Functions----------------------------------------------------------*/
/*********************************************************************************************************************/
void App_Scheduler_Init(void)
{
    Base_Driver_Stm_Init();

    /* TODO: 드라이버 / 앱 모듈 초기화 */
    App_Manager_System_Init();
    App_Manager_Temp_Init();
    initMcmcan();
}

void App_Scheduler_Run(void)
{
    Base_Driver_Stm_SchedulingFlag_t flags;

    Base_Driver_Stm_GetAndClearSchedulingFlags(&flags);

    if (flags.scheduling_1ms_flag != 0u)
    {
        App_Scheduler_Run_1ms();
    }

    if (flags.scheduling_10ms_flag != 0u)
    {
        App_Scheduler_Run_10ms();
    }

    if (flags.scheduling_100ms_flag != 0u)
    {
        App_Scheduler_Run_100ms();
    }

    if (flags.scheduling_1s_flag != 0u)
    {
        App_Scheduler_Run_1s();
    }

    if (flags.scheduling_10s_flag != 0u)
    {
        App_Scheduler_Run_10s();
    }
}

static void App_Scheduler_Run_1ms(void)
{
    /* 아주 짧고 자주 돌아야 하는 작업 */
    /* 예: LED 토글용 소프트 타이머, debounce tick, watchdog service 등 */
}

static void App_Scheduler_Run_10ms(void)
{
    uint32                     now_ms;
    App_Manager_System_Input_t system_input;
    App_Manager_System_Output_t system_output;
    sint16                     local_temperature_x10;

    now_ms = Base_Driver_Stm_GetNowMs();

    /* TODO: 실제 입력값으로 교체 */
    system_input.auth_event_valid     = FALSE;
    system_input.shutdown_request     = FALSE;
    system_input.active_profile_index = SHARED_PROFILE_INDEX_INVALID;

    local_temperature_x10 = 250; /* 25.0도 예시 */

    App_Manager_System_Run(now_ms,
                           local_temperature_x10,
                           &system_input,
                           &system_output);

    /* TODO: output을 CAN 송신 버퍼나 다른 모듈에 반영 */
    (void)system_output;
}

static void App_Scheduler_Run_100ms(void)
{
    /* 중간 주기 작업 */
    /* 예: RFID polling, 온도 갱신 요청, CAN 상태 송신 등 */
}

static void App_Scheduler_Run_1s(void)
{
    /* 느린 주기 작업 */
    /* 예: heartbeat, 상태 진단 로그, timeout check */
}

static void App_Scheduler_Run_10s(void)
{
    /* 매우 느린 주기 작업 */
    /* 예: 유지보수용 통계, 비휘발성 저장 조건 검사 */
}

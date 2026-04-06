/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "App_Hvac.h"
#include "Base_Fan.h"
#include "IfxPort.h"
#include "Ifx_PinMap.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
// LED 포트
#define BLUELED &MODULE_P10, 2
#define REDLED  &MODULE_P10, 1

// 임계값 설정 최대/최소치
#define MIN_H_TH 6
#define MAX_H_TH 22
#define MIN_C_TH 16
#define MAX_C_TH 30

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
/*********************************************************************************************************************/
volatile static sint8 heat_threshold;
volatile static sint8 cool_threshold;

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
// 구현 필요한 함수
static sint8 Hvac_getTemperature(void);

// 외부 API
void App_Manager_Ambient_Init(void);
void App_Ambient_Nextmode(void);
void App_Manager_Ambient_Run(void);
void App_Ambient_changeColor(sint8 amount);
void Amb_getmode(Amb_mode_e *mode);
void Amb_off(void);
void Amb_on(void);

// 내부 라이브러리
static void turnonCooling(void);
static void turnonHeating(void);
static void turnoff(void);

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
void App_Manaver_HVAC_Init(void)
{
  IfxPort_setPinModeOutput(BLUELED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
  IfxPort_setPinModeOutput(REDLED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
  IfxPort_setPinHigh(BLUELED);
  IfxPort_setPinHigh(REDLED);
  heat_threshold = 18;
  cool_threshold = 26;
  Fan_init();
}

uint8 Hvac_setHeatThreshold(sint8 th)
{
  if (th < MIN_H_TH || th > MAX_H_TH || th + 2 >= cool_threshold)
    return 1;
  heat_threshold = th;
  return 0;
}

uint8 Hvac_getHeatThreshold(void)
{
  return heat_threshold;
}

uint8 Hvac_setCoolThreshold(sint8 th)
{
  if (th < MIN_C_TH || th > MAX_C_TH || th - 2 <= heat_threshold)
    return 1;
  cool_threshold = th;
  return 0;
}

uint8 Hvac_getCoolThreshold(void)
{
  return cool_threshold;
}

void App_Manager_HVAC_Run(void)
{
  volatile sint8 temp = Hvac_getTemperature();
  if (temp >= cool_threshold + 2)
    turnonCooling();
  else if (temp <= heat_threshold - 2)
    turnonHeating();
  else if (temp <= cool_threshold - 1 && temp >= heat_threshold + 1)
    turnoff();
}

static void turnonCooling(void)
{
  IfxPort_setPinLow(BLUELED);
  Fan_setSpeed(80);
}

static void turnonHeating(void)
{
  IfxPort_setPinLow(REDLED);
  Fan_setSpeed(80);
}

static void turnoff(void)
{
  IfxPort_setPinHigh(BLUELED);
  IfxPort_setPinHigh(REDLED);
  Fan_setSpeed(0);
}

static sint8 Hvac_getTemperature(void) // TODO
{
  return 22;
}
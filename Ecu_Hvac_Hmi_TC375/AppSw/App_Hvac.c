#include "App_Hvac.h"
#include "Base_Fan.h"
#include "IfxPort.h"
#include "Ifx_PinMap.h"

static uint8 heat_threshold;
static uint8 cool_threshold;

#define BLUELED &MODULE_P10, 2
#define REDLED  &MODULE_P10, 1

// 구현 필요한 함수
// extern uint8 Hvac_getTemperature(void);

// 내부 라이브러리
static void turnonCooling(void);
static void turnonHeating(void);
static void turnoff(void);

void Hvac_init(void)
{
  IfxPort_setPinModeOutput(BLUELED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
  IfxPort_setPinModeOutput(REDLED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
  IfxPort_setPinHigh(BLUELED);
  IfxPort_setPinHigh(REDLED);
  Fan_init();
}

void Hvac_setHeatThreshold(uint8 th)
{
  heat_threshold = th;
}

void Hvac_setCoolThreshold(uint8 th)
{
  cool_threshold = th;
}

void Hvac_updateHvac(void)
{
  uint8 temp = 10;
  // uint8 temp = Hvac_getTemperature();
  if (temp >= cool_threshold + 2)
    turnonCooling();
  else if (temp <= heat_threshold - 2)
    turnonHeating();
  else if (temp <= cool_threshold - 1 || temp >= heat_threshold + 1)
    turnoff();
}

static void turnonCooling(void)
{
  IfxPort_setPinLow(BLUELED);
  Fan_setSpeed(10);
}

static void turnonHeating(void)
{
  IfxPort_setPinLow(REDLED);
  Fan_setSpeed(10);
}

static void turnoff(void)
{
  IfxPort_setPinHigh(BLUELED);
  IfxPort_setPinHigh(REDLED);
  Fan_setSpeed(0);
}
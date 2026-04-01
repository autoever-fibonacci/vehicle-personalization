#include "App_Amb.h"
#include "Base_Neopixel.h"

#define MAXVAL 75
#define MINVAL 20

static Amb_mode_e ambmode = AMB_CONSTANT;
static int baseh, bases, basev;
static int nowv;
static boolean breathdesc;

void Amb_init(void)
{
  initNeopixel();
  baseh = 150;
  bases = 90;
  basev = MAXVAL;
}

void Amb_nextmode(void)
{
  switch (ambmode)
  {
  case AMB_CONSTANT:
    ambmode = AMB_BREATH;
    nowv = MAXVAL;
    breathdesc = TRUE;
    break;
  case AMB_BREATH:
    nowv = MAXVAL;
    ambmode = AMB_WAVE_L;
    break;
  case AMB_WAVE_L:
    ambmode = AMB_WAVE_R;
    break;
  case AMB_WAVE_R:
    ambmode = AMB_CONSTANT;
    break;
  }
}

void Amb_transition(void)
{
  switch (ambmode)
  {
  case AMB_CONSTANT:
    setAllLEDColorHSV(baseh, bases, basev);
    break;
  case AMB_BREATH:
    if (breathdesc)
    {
      if (nowv-- <= MINVAL)
        breathdesc = FALSE, nowv = MINVAL;
    }
    else
    {
      if (nowv++ >= MAXVAL)
        breathdesc = TRUE, nowv = MAXVAL;
    }
    setAllLEDColorHSV(baseh, bases, nowv);
    break;
  case AMB_WAVE_L:
    break;
  case AMB_WAVE_R:
    break;
  }

  transmitNeopixel();
}

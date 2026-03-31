#include "App_Amb.h"
#include "Base_Neopixel.h"

#define MAXVAL 75
#define MINVAL 20

static Amb_mode_e ambmode = AMB_CONSTANT;
static int baseh, bases, basev;
static int breathv;
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
    breathv = MAXVAL;
    breathdesc = TRUE;
    break;
  case AMB_BREATH:
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
      if (breathv-- <= MINVAL)
        breathdesc = FALSE, breathv = MINVAL;
    }
    else
    {
      if (breathv++ >= MAXVAL)
        breathdesc = TRUE, breathv = MAXVAL;
    }
    setAllLEDColorHSV(baseh, bases, breathv);
    break;
  case AMB_WAVE_L:
    break;
  case AMB_WAVE_R:
    break;
  }

  transmitNeopixel();
}
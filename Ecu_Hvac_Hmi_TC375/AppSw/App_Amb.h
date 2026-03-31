#ifndef APP_AMB_H_
#define APP_AMB_H_

#ifndef AMB_MODE_E
#define AMB_MODE_E
typedef enum
{
  AMB_CONSTANT,
  AMB_BREATH,
  AMB_WAVE_L,
  AMB_WAVE_R
} Amb_mode_e;
#endif

void Amb_init(void);
void Amb_nextmode(void);
void Amb_transition(void);

#endif
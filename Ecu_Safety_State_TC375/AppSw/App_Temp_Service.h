#ifndef APPSW_APP_TEMP_SERVICE_H_
#define APPSW_APP_TEMP_SERVICE_H_

#include "Platform_Types.h"
#include "Base_OneWire.h"
#include "Base_Ds18b20_Fsm.h"

void            App_Temp_Service_Init(void);
void            App_Temp_Service_Run(void);

boolean         App_Temp_Service_RequestUpdate(void);

boolean         App_Temp_Service_IsBusy(void);
boolean         App_Temp_Service_IsDataValid(void);
boolean         App_Temp_Service_IsError(void);

boolean         App_Temp_Service_GetLatestTemp_X10(sint16* temp_x10);
Ds18b20_State_t App_Temp_Service_GetFsmState(void);

#endif /* APPSW_APP_TEMP_SERVICE_H_ */

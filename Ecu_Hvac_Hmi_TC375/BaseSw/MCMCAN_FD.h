#ifndef MCMCAN_FD_H
#define MCMCAN_FD_H

#include "Ifx_Types.h"
#include "IfxCan_Can.h"
#include "IfxCan.h"
#include "IfxPort.h"
#include "IfxScuWdt.h"

/* 매크로 설정 */
#define TX_PIN                   IfxCan_TXD00_P20_8_OUT
#define RX_PIN                   IfxCan_RXD00B_P20_7_IN
#define ISR_PRIORITY_CAN_TX      2
#define ISR_PRIORITY_CAN_RX      1
#define MAXIMUM_CAN_DATA_PAYLOAD 16U    /* uint32 x 16 = 64 bytes (CAN FD) */
#define INVALID_RX_DATA_VALUE    0xAAAAAAAAU

/* 구조체 정의 */
typedef struct
{
    IfxCan_Can_Config     canConfig;
    IfxCan_Can            canModule;
    IfxCan_Can_Node       canSrcNode;
    IfxCan_Can_NodeConfig canNodeConfig;
    IfxCan_Filter         canFilter;
    IfxCan_Message        txMsg;
    IfxCan_Message        rxMsg;
    uint32                txData[MAXIMUM_CAN_DATA_PAYLOAD];
    uint32                rxData[MAXIMUM_CAN_DATA_PAYLOAD];
} McmcanType;

/* 전역 변수 */
extern McmcanType g_mcmcan;

/* 함수 선언 */
boolean initMcmcan(void);
boolean transmitCanMessage(uint32 txId, const uint32 *pData);
boolean receiveCanMessage(uint32 *rxData);

#endif /* MCMCAN_FD_H */

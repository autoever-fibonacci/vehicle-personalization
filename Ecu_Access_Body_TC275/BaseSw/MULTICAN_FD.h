#ifndef MULTICAN_FD_H
#define MULTICAN_FD_H

#include "Ifx_Types.h"
#include "IfxMultican_Can.h"
#include "IfxMultican.h"
#include "IfxPort.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/*
 * RX interrupt
 * - 현재 polling 기반으로 써도 되고
 * - 이후 RX 인터럽트 확장 시 그대로 사용 가능
 */
#define ISR_PRIORITY_CAN_RX                 (1U)
#define RX_INTERRUPT_SRC_ID                 (IfxMultican_SrcId_0)

/*
 * CAN FD payload work buffer
 * - uint32[16] = 64 bytes
 */
#define MAXIMUM_CAN_DATA_PAYLOAD            (16U)

/*********************************************************************************************************************/
/*------------------------------------------------Type Definitions--------------------------------------------------*/
/*********************************************************************************************************************/
typedef struct
{
    IfxMultican_Can               can;             /* CAN module handle */
    IfxMultican_Can_Config        canConfig;       /* CAN module config */

    IfxMultican_Can_Node          canNode0;        /* CAN node handle */
    IfxMultican_Can_NodeConfig    nodeConfig;      /* CAN node config */

    IfxMultican_Can_MsgObj        canSrcMsgObj;    /* TX message object */
    IfxMultican_Can_MsgObj        canRxMsgObj;     /* RX message object (single common RX object) */
    IfxMultican_Can_MsgObjConfig  canMsgObjConfig; /* Message object config */

    IfxMultican_Message           txMsg;           /* TX message */
    IfxMultican_Message           rxMsg;           /* RX message */

    uint32                        txData[MAXIMUM_CAN_DATA_PAYLOAD]; /* TX data buffer (64B) */
    uint32                        rxData[MAXIMUM_CAN_DATA_PAYLOAD]; /* RX data buffer (64B) */
} multicanType;

extern multicanType g_multican;

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void    initMultican(void);
void    transmitCanMessage(uint32 txId, uint32 *pData);
boolean receiveCanMessage(uint32 *rxData);

#endif /* MULTICAN_FD_H */
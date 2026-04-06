/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include "MULTICAN_FD.h"
#include "Shared_Can_Message.h"
#include "UART_Config.h"

#include <string.h>

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
#define MULTICAN_NODE_ID                    (IfxMultican_NodeId_0)

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
static void Multican_EnableTransceiver(void);
static void Multican_InitModule(void);
static void Multican_InitNode(void);
static void Multican_InitRxMsgObj(void);
static void Multican_InitTxMsgObj(void);
static void Multican_EnableFdBrsOnMo(uint8 msgObjId);
static const char *Multican_GetProfileTableRxName(uint32 message_id);
static void        Multican_LogProfileTableRxCompact(uint32        message_id,
                                                     const uint32 *rx_data);

/*********************************************************************************************************************/
/*--------------------------------------------------Global Variables-------------------------------------------------*/
/*********************************************************************************************************************/
multicanType g_multican;

/*********************************************************************************************************************/
/*------------------------------------------------Functions----------------------------------------------------------*/
/*********************************************************************************************************************/
void initMultican(void)
{
    (void)memset(&g_multican, 0, sizeof(g_multican));

    Multican_EnableTransceiver();
    Multican_InitModule();
    Multican_InitNode();
    Multican_InitRxMsgObj();
    Multican_InitTxMsgObj();

    (void)memset(g_multican.txData, 0, sizeof(g_multican.txData));
    (void)memset(g_multican.rxData, 0, sizeof(g_multican.rxData));
}

static const char *Multican_GetProfileTableRxName(uint32 message_id)
{
    switch (message_id)
    {
        case SHARED_CAN_MSG_ID_SS_PROFILE_TABLE:
        {
            return "SS_PT";
        }

        case SHARED_CAN_MSG_ID_AB_PROFILE_TABLE:
        {
            return "AB_PT";
        }

        case SHARED_CAN_MSG_ID_HH_PROFILE_TABLE:
        {
            return "HH_PT";
        }

        default:
        {
            return NULL_PTR;
        }
    }
}

static void Multican_LogProfileTableRxCompact(uint32        message_id,
                                              const uint32 *rx_data)
{
    static const char hex_chars[] = "0123456789ABCDEF";
    const char       *rx_name;
    const uint8      *raw_bytes;
    char              line[120];
    uint8             line_idx;
    uint8             byte_idx;

    if (rx_data == NULL_PTR)
    {
        return;
    }

    rx_name = Multican_GetProfileTableRxName(message_id);
    if (rx_name == NULL_PTR)
    {
        return;
    }

    raw_bytes = (const uint8 *)rx_data;
    line_idx  = 0U;

    line[line_idx++] = '[';
    line[line_idx++] = 'M';
    line[line_idx++] = 'C';
    line[line_idx++] = ']';
    line[line_idx++] = '[';
    line[line_idx++] = 'R';
    line[line_idx++] = 'X';
    line[line_idx++] = ']';
    line[line_idx++] = ' ';

    while ((*rx_name != '\0') && (line_idx < (uint8)(sizeof(line) - 1U)))
    {
        line[line_idx++] = *rx_name;
        rx_name++;
    }

    if (line_idx < (uint8)(sizeof(line) - 1U))
    {
        line[line_idx++] = ' ';
    }

    for (byte_idx = 0U; byte_idx < SHARED_CAN_MSG_SIZE_PROFILE_TABLE; ++byte_idx)
    {
        if ((byte_idx != 0U) && ((byte_idx % SHARED_PROFILE_SIZE_BYTE) == 0U))
        {
            if (line_idx >= (uint8)(sizeof(line) - 2U))
            {
                break;
            }

            line[line_idx++] = '|';
        }

        if (line_idx >= (uint8)(sizeof(line) - 3U))
        {
            break;
        }

        line[line_idx++] = hex_chars[(raw_bytes[byte_idx] >> 4U) & 0x0FU];
        line[line_idx++] = hex_chars[raw_bytes[byte_idx] & 0x0FU];
    }

    line[line_idx] = '\0';

    UART_Printf("%s\r\n", line);
}

static void Multican_EnableTransceiver(void)
{
    /* 보드의 CAN transceiver enable / standby 해제 */
    IfxPort_setPinModeOutput(&MODULE_P20, 6, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(&MODULE_P20, 6);
}

static void Multican_InitModule(void)
{
    IfxMultican_Can_initModuleConfig(&g_multican.canConfig, &MODULE_CAN);
    IfxMultican_Can_initModule(&g_multican.can, &g_multican.canConfig);
}

static void Multican_InitNode(void)
{
    IfxMultican_Can_Node_initConfig(&g_multican.nodeConfig, &g_multican.can);

    g_multican.nodeConfig.nodeId           = MULTICAN_NODE_ID;
    g_multican.nodeConfig.rxPin            = &IfxMultican_RXD0B_P20_7_IN;
    g_multican.nodeConfig.txPin            = &IfxMultican_TXD0_P20_8_OUT;
    g_multican.nodeConfig.flexibleDataRate = TRUE;

    /* arbitration: 500 kbps, data: 5 Mbps */
    g_multican.nodeConfig.baudrate                  = 500000U;
    g_multican.nodeConfig.fdConfig.nominalBaudrate = 500000U;
    g_multican.nodeConfig.fdConfig.fastBaudrate    = 5000000U;
    g_multican.nodeConfig.fdConfig.fastSamplePoint = 7500U;
    g_multican.nodeConfig.fdConfig.loopDelayOffset = 12U;

    IfxMultican_Can_Node_init(&g_multican.canNode0, &g_multican.nodeConfig);
}

static void Multican_InitRxMsgObj(void)
{
    IfxMultican_Can_MsgObj_initConfig(&g_multican.canMsgObjConfig, &g_multican.canNode0);

    g_multican.canMsgObjConfig.msgObjId               = MULTICAN_RX_BASE_MSGOBJ_ID;
    g_multican.canMsgObjConfig.messageId              = 0U;
    g_multican.canMsgObjConfig.acceptanceMask         = 0U;
    g_multican.canMsgObjConfig.frame                  = IfxMultican_Frame_receive;

    /* 모든 표준 ID를 공용 RX object 하나로 수신 */
    g_multican.canMsgObjConfig.control.extendedFrame  = FALSE;
    g_multican.canMsgObjConfig.control.matchingId     = TRUE;

    /* 64B long frame linked message object */
    g_multican.canMsgObjConfig.control.topMsgObjId    = MULTICAN_RX_TOP_MSGOBJ_ID;
    g_multican.canMsgObjConfig.control.bottomMsgObjId = MULTICAN_RX_BOTTOM_MSGOBJ_ID;

    g_multican.canMsgObjConfig.control.messageLen     = IfxMultican_DataLengthCode_64;
    g_multican.canMsgObjConfig.control.fastBitRate    = TRUE;

    IfxMultican_Can_MsgObj_init(&g_multican.canRxMsgObj, &g_multican.canMsgObjConfig);

    Multican_EnableFdBrsOnMo(MULTICAN_RX_BASE_MSGOBJ_ID);
    Multican_EnableFdBrsOnMo(MULTICAN_RX_TOP_MSGOBJ_ID);
    Multican_EnableFdBrsOnMo(MULTICAN_RX_BOTTOM_MSGOBJ_ID);
}

static void Multican_InitTxMsgObj(void)
{
    IfxMultican_Can_MsgObj_initConfig(&g_multican.canMsgObjConfig, &g_multican.canNode0);

    g_multican.canMsgObjConfig.msgObjId               = MULTICAN_TX_BASE_MSGOBJ_ID;
    g_multican.canMsgObjConfig.messageId              = SHARED_CAN_MSG_ID_AB_PROFILE_IDX;
    g_multican.canMsgObjConfig.frame                  = IfxMultican_Frame_transmit;

    g_multican.canMsgObjConfig.control.extendedFrame  = FALSE;
    g_multican.canMsgObjConfig.control.matchingId     = TRUE;

    /* 64B long frame linked message object */
    g_multican.canMsgObjConfig.control.topMsgObjId    = MULTICAN_TX_TOP_MSGOBJ_ID;
    g_multican.canMsgObjConfig.control.bottomMsgObjId = MULTICAN_TX_BOTTOM_MSGOBJ_ID;

    g_multican.canMsgObjConfig.control.messageLen     = IfxMultican_DataLengthCode_64;
    g_multican.canMsgObjConfig.control.fastBitRate    = TRUE;

    IfxMultican_Can_MsgObj_init(&g_multican.canSrcMsgObj, &g_multican.canMsgObjConfig);

    Multican_EnableFdBrsOnMo(MULTICAN_TX_BASE_MSGOBJ_ID);
    Multican_EnableFdBrsOnMo(MULTICAN_TX_TOP_MSGOBJ_ID);
    Multican_EnableFdBrsOnMo(MULTICAN_TX_BOTTOM_MSGOBJ_ID);
}

static void Multican_EnableFdBrsOnMo(uint8 msgObjId)
{
    MODULE_CAN.MO[msgObjId].FCR.B.FDF = 1U;
    MODULE_CAN.MO[msgObjId].FCR.B.BRS = 1U;
}

/* -------------------------------------------------------------------------------------------------
 * 송신 함수
 * - 64바이트 고정 frame
 * - pData는 반드시 uint32[16] 버퍼여야 함
 * ------------------------------------------------------------------------------------------------- */
void transmitCanMessage(uint32 txId, uint32 *pData)
{
    uint8 idx;

    if (pData == NULL_PTR)
    {
        return;
    }

    for (idx = 0U; idx < MAXIMUM_CAN_DATA_PAYLOAD; ++idx)
    {
        g_multican.txData[idx] = pData[idx];
    }

    IfxMultican_Message_longFrameInit(&g_multican.txMsg,
                                      txId,
                                      IfxMultican_DataLengthCode_64,
                                      TRUE);

    while (IfxMultican_Can_MsgObj_sendLongFrame(&g_multican.canSrcMsgObj,
                                                &g_multican.txMsg,
                                                g_multican.txData) == IfxMultican_Status_notSentBusy)
    {
        /* busy wait */
    }
}

/* -------------------------------------------------------------------------------------------------
 * 수신 함수
 * - 하나의 RX object에서 모든 메시지를 수신
 * - 읽힌 메시지는 g_multican.rxMsg / g_multican.rxData에 반영
 * - 상위에서는 g_multican.rxMsg.id 를 보고 메시지 ID를 파싱
 * ------------------------------------------------------------------------------------------------- */
boolean receiveCanMessage(uint32 *rxData)
{
    IfxMultican_Status readStatus;
    uint8              idx;

    if (rxData == NULL_PTR)
    {
        return FALSE;
    }

    if (IfxMultican_Can_MsgObj_isRxPending(&g_multican.canRxMsgObj) == FALSE)
    {
        return FALSE;
    }

    IfxMultican_Message_longFrameInit(&g_multican.rxMsg,
                                      0xFFFFFFFFU,
                                      IfxMultican_DataLengthCode_64,
                                      TRUE);

    readStatus = IfxMultican_Can_MsgObj_readLongFrame(&g_multican.canRxMsgObj,
                                                      &g_multican.rxMsg,
                                                      g_multican.rxData);

    IfxMultican_Can_MsgObj_clearRxPending(&g_multican.canRxMsgObj);

    if ((readStatus & IfxMultican_Status_newData) == 0U)
    {
        return FALSE;
    }

    for (idx = 0U; idx < MAXIMUM_CAN_DATA_PAYLOAD; ++idx)
    {
        rxData[idx] = g_multican.rxData[idx];
    }

    Multican_LogProfileTableRxCompact(g_multican.rxMsg.id, g_multican.rxData);

    return TRUE;
}

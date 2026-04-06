#include "MULTICAN_FD.h"
#include "Shared_Can_Message.h"

multicanType g_multican;

/* 파일 내부 전용 RX 객체 1개 */
static IfxMultican_Can_MsgObj g_can_rx_msg_obj;

void initMultican(void)
{
    /* 트랜시버 전원 활성화 */
    IfxPort_setPinModeOutput(&MODULE_P20, 6, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(&MODULE_P20, 6);

    /* CAN 모듈 초기화 */
    IfxMultican_Can_initModuleConfig(&g_multican.canConfig, &MODULE_CAN);
    IfxMultican_Can_initModule(&g_multican.can, &g_multican.canConfig);

    /* CAN 노드 설정 */
    IfxMultican_Can_Node_initConfig(&g_multican.nodeConfig, &g_multican.can);

    g_multican.nodeConfig.nodeId = IfxMultican_NodeId_0;
    g_multican.nodeConfig.rxPin  = &IfxMultican_RXD0B_P20_7_IN;
    g_multican.nodeConfig.txPin  = &IfxMultican_TXD0_P20_8_OUT;
    g_multican.nodeConfig.flexibleDataRate = TRUE;

    /* arbitration: 500 kbps, data: 5 Mbps */
    g_multican.nodeConfig.fdConfig.nominalBaudrate  = 500000;
    g_multican.nodeConfig.fdConfig.fastBaudrate     = 5000000;
    g_multican.nodeConfig.fdConfig.fastSamplePoint  = 7500;
    g_multican.nodeConfig.fdConfig.loopDelayOffset  = 12;

    IfxMultican_Can_Node_init(&g_multican.canNode0, &g_multican.nodeConfig);

    /* ---------------------------------------------------------------------------------------------
     * RX object #1 : 모든 수신 메시지 공용
     * - acceptanceMask = 0 으로 두어 ID 비교를 사실상 하지 않음
     * - 실제 메시지 구분은 readMessage() 이후 상위에서 messageId로 파싱
     * --------------------------------------------------------------------------------------------- */
    IfxMultican_Can_MsgObj_initConfig(&g_multican.canMsgObjConfig, &g_multican.canNode0);
    g_multican.canMsgObjConfig.msgObjId               = 1U;
    g_multican.canMsgObjConfig.messageId              = 0U;
    g_multican.canMsgObjConfig.acceptanceMask         = 0U;
    g_multican.canMsgObjConfig.frame                  = IfxMultican_Frame_receive;
    g_multican.canMsgObjConfig.control.messageLen     = IfxMultican_DataLengthCode_64;
    g_multican.canMsgObjConfig.control.fastBitRate    = TRUE;

    IfxMultican_Can_MsgObj_init(&g_can_rx_msg_obj, &g_multican.canMsgObjConfig);

    /* ---------------------------------------------------------------------------------------------
     * TX object : AB_ACCESS_IDX / AB_PROFILE_TABLE 송신용
     * --------------------------------------------------------------------------------------------- */
    IfxMultican_Can_MsgObj_initConfig(&g_multican.canMsgObjConfig, &g_multican.canNode0);
    g_multican.canMsgObjConfig.msgObjId               = 0U;

    /* Note: actual TX messageId may be overwritten at send time */
    g_multican.canMsgObjConfig.messageId              = SHARED_CAN_MSG_ID_AB_PROFILE_IDX;

    g_multican.canMsgObjConfig.frame                  = IfxMultican_Frame_transmit;
    g_multican.canMsgObjConfig.control.messageLen     = IfxMultican_DataLengthCode_64;
    g_multican.canMsgObjConfig.control.fastBitRate    = TRUE;

    IfxMultican_Can_MsgObj_init(&g_multican.canSrcMsgObj, &g_multican.canMsgObjConfig);

    /* FD(FDF) + BRS 활성화 */
    MODULE_CAN.MO[0].FCR.B.FDF = 1U;
    MODULE_CAN.MO[0].FCR.B.BRS = 1U;

    MODULE_CAN.MO[1].FCR.B.FDF = 1U;
    MODULE_CAN.MO[1].FCR.B.BRS = 1U;
}

/* 송신 함수 */
void transmitCanMessage(uint32 txId, uint32 *pData)
{
    sint32 idx;

    IfxMultican_Message_init(&g_multican.txMsg,
                             txId,
                             0U,
                             0U,
                             IfxMultican_DataLengthCode_64);

    g_multican.txMsg.fastBitRate = TRUE;

    for (idx = 0; idx < 16; ++idx)
    {
        g_multican.txMsg.data[idx] = pData[idx];
        g_multican.txData[idx]     = pData[idx];
    }

    while (IfxMultican_Status_notSentBusy ==
           IfxMultican_Can_MsgObj_sendMessage(&g_multican.canSrcMsgObj, &g_multican.txMsg))
    {
        /* busy wait */
    }
}

/* 수신 함수
 * - 하나의 RX object에서 모든 메시지를 수신
 * - 읽힌 메시지는 g_multican.rxMsg / g_multican.rxData에 반영
 * - 상위에서는 g_multican.rxMsg의 ID를 보고 파싱
 */
boolean receiveCanMessage(uint32 *rxData)
{
    sint32 idx;

    if (rxData == NULL_PTR)
    {
        return FALSE;
    }

    IfxMultican_Message_init(&g_multican.rxMsg,
                             0U,
                             0U,
                             0U,
                             IfxMultican_DataLengthCode_64);

    if (IfxMultican_Can_MsgObj_readMessage(&g_can_rx_msg_obj, &g_multican.rxMsg) ==
        IfxMultican_Status_newData)
    {
        for (idx = 0; idx < 16; ++idx)
        {
            g_multican.rxData[idx] = g_multican.rxMsg.data[idx];
            rxData[idx]            = g_multican.rxData[idx];
        }
        return TRUE;
    }

    return FALSE;
}

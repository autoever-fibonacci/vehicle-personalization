#include "MCMCAN_FD.h"

McmcanType g_mcmcan;

/* 핀 설정 */
static const IfxCan_Can_Pins canPins =
{
    &TX_PIN, IfxPort_OutputMode_pushPull,
    &RX_PIN, IfxPort_InputMode_pullUp,
    IfxPort_PadDriver_cmosAutomotiveSpeed1
};

static boolean Mcmcan_WaitModuleReady(void)
{
    sint32 timeout = 1000000;

    while (MODULE_CAN0.CLC.B.DISS != 0U)
    {
        timeout--;
        if (timeout <= 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static boolean Mcmcan_WaitNodeSync(void)
{
    sint32 timeout = 1000000;

    while (IfxCan_Can_isNodeSynchronized(&g_mcmcan.canSrcNode) != TRUE)
    {
        timeout--;
        if (timeout <= 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean initMcmcan(void)
{
    uint16 password;

    /* 1. 트랜시버 전원 및 CAN 모듈 활성화 */
    IfxPort_setPinModeOutput(&MODULE_P20, 6, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(&MODULE_P20, 6);

    password = IfxScuWdt_getCpuWatchdogPassword();
    IfxScuWdt_clearCpuEndinit(password);
    MODULE_CAN0.CLC.B.DISR = 0U;
    IfxScuWdt_setCpuEndinit(password);

    if (Mcmcan_WaitModuleReady() == FALSE)
    {
        return FALSE;
    }

    /* 2. 모듈 초기화 */
    IfxCan_Can_initModuleConfig(&g_mcmcan.canConfig, &MODULE_CAN0);
    IfxCan_Can_initModule(&g_mcmcan.canModule, &g_mcmcan.canConfig);

    /* 3. 노드 설정 */
    IfxCan_Can_initNodeConfig(&g_mcmcan.canNodeConfig, &g_mcmcan.canModule);

    g_mcmcan.canNodeConfig.pins = &canPins;
    g_mcmcan.canNodeConfig.busLoopbackEnabled = FALSE; /* 단일보드 자가시험이면 TRUE로 */
    g_mcmcan.canNodeConfig.nodeId = IfxCan_NodeId_0;
    g_mcmcan.canNodeConfig.frame.type = IfxCan_FrameType_transmitAndReceive;
    g_mcmcan.canNodeConfig.frame.mode = IfxCan_FrameMode_fdLongAndFast;

    /* 폴링 방식이므로 인터럽트는 끔 */
    g_mcmcan.canNodeConfig.interruptConfig.transmissionCompletedEnabled = FALSE;
    g_mcmcan.canNodeConfig.interruptConfig.rxFifo0NewMessageEnabled = FALSE;

    /* Bit timing */
    g_mcmcan.canNodeConfig.baudRate.baudrate = 500000;
    g_mcmcan.canNodeConfig.fastBaudRate.baudrate = 5000000;
    g_mcmcan.canNodeConfig.fastBaudRate.tranceiverDelayOffset = 12;
    g_mcmcan.canNodeConfig.calculateBitTimingValues = TRUE;

    /* Message RAM */
    g_mcmcan.canNodeConfig.filterConfig.standardListSize = 1U;

    g_mcmcan.canNodeConfig.messageRAM.baseAddress = (uint32)&MODULE_CAN0;
    g_mcmcan.canNodeConfig.messageRAM.standardFilterListStartAddress = 0x100;
    g_mcmcan.canNodeConfig.messageRAM.rxFifo0StartAddress = 0x200;
    g_mcmcan.canNodeConfig.messageRAM.txBuffersStartAddress = 0x600;

    /* TX/RX buffer 설정 */
    g_mcmcan.canNodeConfig.txConfig.txFifoQueueSize = 1U;
    g_mcmcan.canNodeConfig.txConfig.dedicatedTxBuffersNumber = 1U;
    g_mcmcan.canNodeConfig.txConfig.txBufferDataFieldSize = IfxCan_DataFieldSize_64;

    g_mcmcan.canNodeConfig.rxConfig.rxMode = IfxCan_RxMode_fifo0;
    g_mcmcan.canNodeConfig.rxConfig.rxFifo0Size = 16U;
    g_mcmcan.canNodeConfig.rxConfig.rxFifo0DataFieldSize = IfxCan_DataFieldSize_64;
    g_mcmcan.canNodeConfig.rxConfig.rxFifo0OperatingMode = IfxCan_RxFifoMode_blocking;

    /* 노드 초기화 */
    IfxCan_Can_initNode(&g_mcmcan.canSrcNode, &g_mcmcan.canNodeConfig);

    /* 4. 0x300만 받도록 필터 설정 */
    g_mcmcan.canFilter.number = 0U;
    g_mcmcan.canFilter.elementConfiguration = IfxCan_FilterElementConfiguration_storeInRxFifo0;
    g_mcmcan.canFilter.type = IfxCan_FilterType_range;
    g_mcmcan.canFilter.id1 = 0x300U;
    g_mcmcan.canFilter.id2 = 0x300U;

    IfxCan_Can_setStandardFilter(&g_mcmcan.canSrcNode, &g_mcmcan.canFilter);

    if (Mcmcan_WaitNodeSync() == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

boolean transmitCanMessage(uint32 txId, const uint32 *pData)
{
    IfxCan_Status status;
    sint32        timeout = 100000;
    sint32        i;

    if (pData == NULL_PTR)
    {
        return FALSE;
    }

    IfxCan_Can_initMessage(&g_mcmcan.txMsg);
    g_mcmcan.txMsg.frameMode = IfxCan_FrameMode_fdLongAndFast;
    g_mcmcan.txMsg.dataLengthCode = IfxCan_DataLengthCode_64;
    g_mcmcan.txMsg.messageId = txId;

    for (i = 0; i < 16; i++)
    {
        g_mcmcan.txData[i] = pData[i];
    }

    do
    {
        status = IfxCan_Can_sendMessage(&g_mcmcan.canSrcNode,
                                        &g_mcmcan.txMsg,
                                        &g_mcmcan.txData[0]);

        if (status == IfxCan_Status_ok)
        {
            return TRUE;
        }

        if (status != IfxCan_Status_notSentBusy)
        {
            return FALSE;
        }

        timeout--;
    } while (timeout > 0);

    return FALSE;
}

boolean receiveCanMessage(uint32 *rxData)
{
    IfxCan_Status status;
    sint32        i;

    if (rxData == NULL_PTR)
    {
        return FALSE;
    }

    if (IfxCan_Can_getRxFifo0FillLevel(&g_mcmcan.canSrcNode) == 0U)
    {
        return FALSE;
    }

    IfxCan_Can_initMessage(&g_mcmcan.rxMsg);
    g_mcmcan.rxMsg.readFromRxFifo0 = TRUE;

    IfxCan_Can_readMessage(&g_mcmcan.canSrcNode,
                           &g_mcmcan.rxMsg,
                           rxData);

    for (i = 0; i < 16; i++)
    {
        g_mcmcan.rxData[i] = rxData[i];
    }

    return TRUE;
}

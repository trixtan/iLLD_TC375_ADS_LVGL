#include "IfxPort.h"
#include "SPI.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/
/* QSPI modules */
#define QSPI1_MASTER                &MODULE_QSPI1   /* SPI Master module                                            */

#define MASTER_CHANNEL_BAUDRATE     20000000        /* Master channel baud rate                                     */

/* Interrupt Service Routine priorities for Master SPI communication */
#define ISR_PRIORITY_MASTER_TX      50
#define ISR_PRIORITY_MASTER_RX      51
#define ISR_PRIORITY_MASTER_ER      52

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
qspiComm g_qspi;

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
static void initQSPI1Master(IfxQspi_SpiMaster *spiMaster);
static void initQSPI1MasterChannel(
        IfxQspi_SpiMaster * spiMaster,
        IfxQspi_SpiMaster_Channel * spiMasterChannel,
        IfxQspi_Slso_Out * chipSelectPin);

/*********************************************************************************************************************/
/*----------------------------------------------Function Implementations---------------------------------------------*/
/*********************************************************************************************************************/
IFX_INTERRUPT(masterTxISR, 0, ISR_PRIORITY_MASTER_TX);                  /* SPI Master ISR for transmit data         */
IFX_INTERRUPT(masterRxISR, 0, ISR_PRIORITY_MASTER_RX);                  /* SPI Master ISR for receive data          */
IFX_INTERRUPT(masterErISR, 0, ISR_PRIORITY_MASTER_ER);                  /* SPI Master ISR for error                 */

void masterTxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrTransmit(&g_qspi.spiMaster);
}

void masterRxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrReceive(&g_qspi.spiMaster);
}

void masterErISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrError(&g_qspi.spiMaster);
}

/* QSPI Master initialization
 * This function initializes the QSPI1 module in Master mode.
 */
static void initQSPI1Master(IfxQspi_SpiMaster *spiMaster)
{
    IfxQspi_SpiMaster_Config spiMasterConfig;                           /* Define a Master configuration            */

    IfxQspi_SpiMaster_initModuleConfig(&spiMasterConfig, QSPI1_MASTER); /* Initialize it with default values        */

    spiMasterConfig.base.mode = SpiIf_Mode_master;                      /* Configure the mode                       */

    /* Select the port pins for communication */
    const IfxQspi_SpiMaster_Pins qspi1MasterPins = {
        TFT_SCLK, IfxPort_OutputMode_pushPull,                          /* SCLK Pin                       (CLK)     */
        TFT_MOSI, IfxPort_OutputMode_pushPull,                          /* MasterTransmitSlaveReceive pin (MOSI)    */
        TFT_MISO, IfxPort_InputMode_pullDown,                           /* MasterReceiveSlaveTransmit pin (MISO)    */
        IfxPort_PadDriver_cmosAutomotiveSpeed3                          /* Pad driver mode                          */
    };
    spiMasterConfig.pins = &qspi1MasterPins;                            /* Assign the Master's port pins            */

    /* Set the ISR priorities and the service provider */
    spiMasterConfig.base.txPriority = ISR_PRIORITY_MASTER_TX;
    spiMasterConfig.base.rxPriority = ISR_PRIORITY_MASTER_RX;
    spiMasterConfig.base.erPriority = ISR_PRIORITY_MASTER_ER;
    spiMasterConfig.base.isrProvider = IfxSrc_Tos_cpu0;

    /* Initialize the QSPI Master module */
    IfxQspi_SpiMaster_initModule(spiMaster, &spiMasterConfig);
}

/* QSPI Master channel initialization */
static void initQSPI1MasterChannel(
        IfxQspi_SpiMaster * spiMaster,
        IfxQspi_SpiMaster_Channel * spiMasterChannel,
        IfxQspi_Slso_Out * chipSelectPin)
{
    IfxQspi_SpiMaster_ChannelConfig spiMasterChannelConfig;             /* Define a Master Channel configuration    */

    /* Initialize the configuration with default values */
    IfxQspi_SpiMaster_initChannelConfig(&spiMasterChannelConfig, spiMaster);

    spiMasterChannelConfig.base.baudrate = MASTER_CHANNEL_BAUDRATE;

    /* Set the transfer data width */
    spiMasterChannelConfig.base.mode.dataWidth = 16;
    spiMasterChannelConfig.base.mode.csLeadDelay = SpiIf_SlsoTiming_1;
    spiMasterChannelConfig.base.mode.csTrailDelay = SpiIf_SlsoTiming_0;
    spiMasterChannelConfig.base.mode.csInactiveDelay = SpiIf_SlsoTiming_0;
    spiMasterChannelConfig.base.mode.shiftClock = SpiIf_ShiftClock_shiftTransmitDataOnTrailingEdge;

    /* Select the port pin for the Chip Select signal */
    const IfxQspi_SpiMaster_Output qspi1SlaveSelect = {                 /* QSPI1 Master selects the QSPI1 Slave     */
        chipSelectPin, IfxPort_OutputMode_pushPull,                     /* Slave Select port pin (CS)               */
        IfxPort_PadDriver_cmosAutomotiveSpeed3                          /* Pad driver mode                          */
    };
    spiMasterChannelConfig.sls.output = qspi1SlaveSelect;

    /* Initialize the QSPI Master channel */
    IfxQspi_SpiMaster_initChannel(spiMasterChannel, &spiMasterChannelConfig);
}

/* This function initialize the QSPI modules */
void initQSPI(void)
{
    initQSPI1Master(&g_qspi.spiMaster);
    //Init TFT Channel
    initQSPI1MasterChannel(
            &g_qspi.spiMaster,
            &g_qspi.tftSPIMasterChannel,
            TFT_LCD_CS);
    //Init Touch Channel
//    initQSPI1MasterChannel(
//            &g_qspi.spiMaster,
//            &g_qspi.touchSPIMasterChannel,
//            TFT_TOUCH_CS);
}

/* This function starts the data transfer */
void transferDataToTFT(const void *dataBuffer, Ifx_SizeT count)
{
    while(IfxQspi_SpiMaster_getStatus(&g_qspi.tftSPIMasterChannel) == SpiIf_Status_busy)
    {   /* Wait until the previous communication has finished, if any */ }

    /* Send a data stream through the SPI Master */
    IfxQspi_SpiMaster_exchange(
            &g_qspi.tftSPIMasterChannel,
            dataBuffer,
            NULL_PTR,
            count);

    while(IfxQspi_SpiMaster_getStatus(&g_qspi.tftSPIMasterChannel) == SpiIf_Status_busy)
        {   /* Wait until the previous communication has finished, if any */ }
}

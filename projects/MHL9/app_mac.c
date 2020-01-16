/*******************************************************************************
 * @file    app_mac.c
 * @brief   Radio MAC task
 *
 * @version 1.0.0
 *******************************************************************************
 * @license Refer License or other description Docs
 * @author  Felix
 ******************************************************************************/
#include "app_mac.h"
#include "at/at_config.h"
#include "mac/node/mac_radio.h"

#define RADIO_SPI_IDX   BSP_SPI0

/****
Global Variables
****/

/****
Local Variables
****/
static struct mac_lorawan_t sMacParam = {0};

/****
Local Functions
****/
#ifdef USE_MODE_RX
/* print recv data */
static void RadioPrintRecv(bool format)
{
    uint8_t i = 0;

    osSaveCritical();
    osEnterCritical();

    if(sMacParam.ack){
        sMacParam.ack = false;
        printk("\r\nOK\r\n");
    } else if(sMacParam.rxLen > 0){
        if(format){
            printk("\r\n+DATA:%d,%u,%d,%d,%u\r\n\r\n",
                   sMacParam.qos.freqerr, gDevFlash.config.rxfreq,
                   sMacParam.qos.snr, sMacParam.qos.rssi,
                   sMacParam.rxLen);
            for(i = sMacParam.rxIdx; i < sMacParam.rxLen + sMacParam.rxIdx; i++){
                printk("%02X", sMacParam.payload[i]);
            }
            printk("\r\n");
        } else {
            if((gDevFlash.config.dtype >> TYPE_BITS_RAW)&0x01){
                UserDebugWrite(sMacParam.payload, sMacParam.size);
            } else {
                UserDebugWrite(sMacParam.payload + sMacParam.rxIdx, sMacParam.rxLen);
            }
        }
    }
    osExitCritical();
}
#else
static uint32_t AT_TxFreq(uint32_t freq, uint8_t *buf, uint32_t len)
{
    uint32_t status = 0;

    sMacParam.freq = freq;
    status = MacRadio_TxProcess(BSP_SPI0, buf, len, &sMacParam);

    return status;
}
#endif

static void AppRadioParameters(void)
{
    /* TODO: change default settings, example */

    /*
    gDevFlash.config.txfreq = 475500000;
    gDevFlash.config.rxfreq = 475500000;
    gDevFlash.config.txsf = RF_SF_12;
    gDevFlash.config.rxsf = RF_SF_12;
    gDevFlash.config.rps.tiq = 0;
    gDevFlash.config.rps.riq = 0;
    gDevFlash.config.prop.netmode = NET_MODE_NONE;
    */
}

/****
Global Functions
****/
bool AppMacTask(void)
{

    memset(&sMacParam, 0 ,sizeof(struct mac_lorawan_t));

    if (false == MacRadio_Init()) {
        LOG_ERR(("Radio Mac Init error.\r\n"));
        return false;
    }

    return true;
}

void AppTaskManager(void)
{
    uint32_t status = AT_STATUS_NONE;

    AppRadioParameters();

#ifdef USE_MODE_RX
    status = MacRadio_RxProcess(RADIO_SPI_IDX, false);
    /* status = MacRadio_CadProcess(RADIO_SPI_IDX, true); */
    if(status == AT_STATUS_OK){
        RadioPrintRecv(true);
    } else if(status == AT_STATUS_RX_ERR) {
        printk("CRC ERR,SNR:%d, RSSI:%ddBm,Calc:%d\r\n",
               sMacParam.qos.snr, sMacParam.qos.rssi, sMacParam.qos.freqerr);
    }
#else
    status = AT_TxFreq(475500000, "TEST0123456789\r\n", strlen("TEST0123456789\r\n"));
    if(status == AT_STATUS_OK){
        printk("\r\nOK\r\n");
    } else {
        printk("\r\nTX ERR\r\n");
    }
#endif

    /* PlatformSleep(1); */
}

RadioIrqType_t RadioRxFinish(uint8_t spiIdx)
{
    struct sx127x_rx_t rxObj;
    RadioIrqType_t type_flag = RADIO_IRQ_UNKOWN;
    {
        rxObj.modem = gDevFlash.config.rps.modem;
        rxObj.bandwidth = gDevFlash.config.rps.bw;
        rxObj.opmode = OP_MODE_RX;
        rxObj.freq = gDevFlash.config.rxfreq;
        rxObj.crc = gDevFlash.config.rps.crc;
    }

    type_flag =  RadioDecode(spiIdx, &rxObj, &sMacParam);

    if(RADIO_IRQ_LORA_RX != type_flag && RADIO_IRQ_FSK_RX != type_flag){
        /* clear buffer */
        memset(sMacParam.payload, 0, sizeof(sMacParam.payload));
        sMacParam.size = 0;
    }

    return type_flag;
}


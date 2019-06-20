/*******************************************************************************
 * @file    app_project.c
 * @brief   Project application interface
 *
 * @version 0.0.1
 *******************************************************************************
 * @license Refer License or other description Docs
 * @author  Felix
 ******************************************************************************/
#include "app.h"
#include "app_at.h"
#include "app_mac.h"
#include "at/at_config.h"
#include "radio/sx127x/sx127x_common.h"
#include "mac/node/mac_radio.h"
#include "mac/node/mac_api.h"

volatile bool   gEnableRadioRx  = true;

char *gCodeVers = "1016";

/****
Local Variables
****/

/****
Local Functions
****/

#ifdef USE_MODE_RX
/* print recv data */
static void RadioPrintRecv(bool format)
{
    uint8_t i = 0;
    if(gMacParam.ack){
        gMacParam.ack = false;
        printk("\r\nOK\r\n");
    } else if(gMacParam.dataLen > 0){
        if(format){
            printk("\r\n+DATA:%d,%u,%d,%d,%u\r\n\r\n",
                   gMacParam.qos.freqerr, gDevFlash.config.rxfreq,
                   gMacParam.qos.snr, gMacParam.qos.rssi,
                   gMacParam.dataLen);
            for(i = gMacParam.dataIdx; i < gMacParam.dataLen + gMacParam.dataIdx; i++){
                printk("%02X", gMacParam.frame[i]);
            }
            printk("\r\n");
        } else {
            UserDebugWrite(gMacParam.frame + gMacParam.dataIdx, gMacParam.dataLen);
        }
    }
}
#endif

/**
 * @brief  Initialize all tasks
 *
 * @return  true if success else false
 */
static bool AppTaskInit(void)
{
    bool result = true;

    /* start MAC task */
    APP_FeedDog();
    result = Mac_Init(gDevFlash.config.dtype & DTYPE_BITS_RFO);

    /**
     * we should opearte AT command when other task is OK,
     * so we start AT command task at the end.
     */
    if(result){
        APP_FeedDog();
        result = AppATTask();
    }

    /* init radio settings, example */
    gDevFlash.config.txpow = 20;
    gDevFlash.config.txfreq = 475500000;
    gDevFlash.config.rxfreq = 475500000;
    gDevFlash.config.txsf = RF_SF_12;
    gDevFlash.config.rxsf = RF_SF_12;
    gDevFlash.config.tiq = 0;
    gDevFlash.config.riq = 0;
    gDevFlash.config.netmode = NET_MODE_NONE;

    return result;
}

/****
Global Functions
****/

/*!
 * @brief  Create the App task
 */
bool AppTaskCreate(void)
{
    bool success = false;

    /* FIXME: watchdog disable if input zero, user can redefine WDOG function */
    if(RJ_ERR_OK != PlatformInit(0)){
        return false;
    }

    /* Low Energy Timer and DeepSleep init */
    if(false == BSP_LPowerInit(false)){
        return false;
    }

    success = UserDebugInit(false, gDevFlash.config.baudrate, gDevFlash.config.pari);

    printk("LoRa %s SDK, HAL V%u:%u, XTL:%d, Firmware V%s\r\n", MODULE_NAME,
           RADIO_HAL_VERSION, AT_VER, gParam.dev.extl, gCodeVers);

    if(success) {
        success = AppTaskInit();
    }

    return success;
}

void AppTaskExtends(void)
{
    uint32_t status = AT_STATUS_NONE;
    while (1) {
        APP_FeedDog();
#ifdef USE_MODE_RX
        if(gEnableRadioRx){
            status = MacRadio_RxProcess(RX_MODE_NONE);
            if(status == AT_STATUS_OK){
                RadioPrintRecv((RX_MODE_FACTORY == gDevRam.rx_mode));
            } else if(status == AT_STATUS_RX_ERR) {
                printk("CRC ERR,SNR:%d, RSSI:%ddBm,Calc:%d\r\n",
                       gMacParam.qos.snr, gMacParam.qos.rssi, gMacParam.qos.freqerr);
            }
            status = AT_STATUS_NONE;
        }
        osDelayMs(1);
#else
        status = MacRadio_TxProcess("TEST0123456789\r\n", strlen("TEST0123456789\r\n"));
        PlatformSleep(2);
#endif
    }
}

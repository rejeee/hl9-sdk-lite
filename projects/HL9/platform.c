/*******************************************************************************
 * @file    platform.c
 * @brief   The platform interface implementation
 *
 * @version 1.0.0
 *******************************************************************************
 * @license Refer License or other description Docs
 * @author  Felix
 ******************************************************************************/
#include "platform/platform.h"
#include <stdarg.h>
#include "at/at_config.h"
#include "radio/sx12xx_common.h"

#define APP_DEV_VER        1

/****
Global Variables
****/
osSemaphoreDef(debugSemaphore);
#define DEBUG_SEM_NAME      osSemaphore(debugSemaphore)

DeviceFlash_t  gDevFlash;

struct global_param_t gParam = {
    .dev        = {0},
    .mainid     = NULL,
    .appid      = NULL,
    .mutex      = NULL,
    .dtime      = 0,
    .rst        = {0},
    .mode       = 0,
    .aswitch    = 1
};

/****
Local Variables
****/

static BSP_ADC_TypeDef sADCConfig;

/* UART user callback */
static void DebugCallback(uint32_t userData)
{
    BSP_LPowerStop();
    /* TODO: */
}

static void UserInitGPIO(void)
{
    stc_gpio_config_t gpioCfg;
    uint8_t pin = 0;

    DDL_ZERO_STRUCT(gpioCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    gpioCfg.enDir = GpioDirIn;
    gpioCfg.enPuPd = GpioPd;
    Gpio_Init(AT_GPIO, AT_PIN, &gpioCfg);
    Gpio_Init(UKEY_GPIO, UKEY_PIN, &gpioCfg);

    /* IRQ on rising edge trigger */
    EnableNvic(UKEY_PORT_IRQ, IrqLevel3, TRUE);
    Gpio_EnableIrq(UKEY_GPIO, UKEY_PIN, GpioIrqRising);

    /* unused GPIO diabled */
    Gpio_Init(GpioPortA, GpioPin11, &gpioCfg);
    Gpio_Init(GpioPortB, GpioPin4, &gpioCfg);
    Gpio_Init(UNUSED_GPIO, UNUSED_PIN, &gpioCfg);

    /* PA GPIO */
    gpioCfg.enPuPd = GpioPu;
    Gpio_Init(UPA_GPIO, UPA_PIN, &gpioCfg);
    osDelayMs(1);
    if(0 == GPIO_READ(UPA_GPIO,UPA_PIN)){
        gPaEnable = true;
        gDevFlash.config.txpow = 0;
        gDevFlash.config.dtype = (0x01 << TYPE_BITS_RFO);
    }
    gpioCfg.enPuPd = GpioPd;
    Gpio_Init(UPA_GPIO, UPA_PIN, &gpioCfg);
    Gpio_Init(GpioPortD, GpioPin0, &gpioCfg);
    Gpio_Init(GpioPortD, GpioPin1, &gpioCfg);

    /* not exists GPIO diabled */
    Gpio_Init(GpioPortA, GpioPin0, &gpioCfg);
    Gpio_Init(GpioPortA, GpioPin1, &gpioCfg);

    Gpio_Init(GpioPortB, GpioPin2, &gpioCfg);
    Gpio_Init(GpioPortB, GpioPin8, &gpioCfg);
    Gpio_Init(GpioPortB, GpioPin9, &gpioCfg);

    for(pin = 0; pin < 14; pin++){
        Gpio_Init(GpioPortC, pin, &gpioCfg);
    }

    for(pin = 2; pin < 8; pin++){
        Gpio_Init(GpioPortD, pin, &gpioCfg);
    }
}

/****
Global Functions
****/

/**
 * @brief IO interrupt service rutine
 */
void Gpio_IRQHandler(uint8_t u8Param)
{
    if(0 == u8Param) {
        DevRadioIRQHandler(u8Param);
    }

    /* User Key IRQ */
    if(1 == u8Param) {
        if(TRUE == Gpio_GetIrqStatus(UKEY_GPIO, UKEY_PIN)){
            Gpio_ClearIrq(UKEY_GPIO, UKEY_PIN);
            BSP_LPowerStop();
        }
    }
}

/**
 * @brief radio hal API implemetation
 *
 * function declarations refer radio common definitions
 */
void RadioDelay(uint32_t ms)
{
    osDelayMs(ms);
}

/* Ant control is empty implemetation to be compatible different designs */
void RadioAntLowPower(uint8_t spiIdx, uint8_t status){ }
void RadioAntSwitch(uint8_t spiIdx, bool rx) { }

/**
 * @brief user project special implementation
 * function declarations refer platform common definitions
 */
void UserExternalGPIO(bool enable)
{
    /** @todo: */
}

bool UserDebugInit(bool reinit, uint32_t baudrateType, uint8_t pariType)
{
    BSP_UART_TypeDef uart = {
        .cb = DebugCallback,
        .gpio = DBG_GPIO,
        .tx_pin = DBG_TX_PIN,
        .rx_pin = DBG_RX_PIN,
        .af = DBG_AF,
        .pd = GpioPu,
        .idx = DBG_UART_IDX,
        .bdtype = baudrateType,
        .pri = pariType
    };

    BSP_UartInit(&uart);

    return true;
}

void UserDebugWrite(const uint8_t *data, uint32_t len)
{
    BSP_UartSend(DBG_UART_IDX, data, len);
}

/**
 *  FIXME:
 *  User should redefine config display function to reduce code size
 *  This function is called only by AT task.
 */
void DevCfg_Display(uint8_t uartIdx)
{
    rps_t rps = gDevFlash.config.rps;
    uint32_t freq  = gDevFlash.config.txfreq;
    char* bwstr = NULL;
    uint8_t ldrtx = 0;
    uint8_t ldrrx = 0;
    int8_t txpow = gDevFlash.config.txpow;
    bool notAes = binIsTag(0x00, gDevFlash.config.appKey, 16);
    ldrtx = (gDevFlash.config.rps.lowRate >> 2)&0x03;
    ldrrx = gDevFlash.config.rps.lowRate&0x03;

    switch(rps.bw){
    case RF_BANDWIDTH_7D8:
        bwstr = "7.8kHz";
        break;
    case RF_BANDWIDTH_10D4:
        bwstr = "10.4kHz";
        break;
    case RF_BANDWIDTH_15D6:
        bwstr = "15.6kHz";
        break;
    case RF_BANDWIDTH_20D8:
        bwstr = "20.8kHz";
        break;
    case RF_BANDWIDTH_31D2:
        bwstr = "31.2kHz";
        break;
    case RF_BANDWIDTH_41D7:
        bwstr = "41.7kHz";
        break;
    case RF_BANDWIDTH_62D5:
        bwstr = "62.5kHz";
        break;
    case RF_BANDWIDTH_125:
        bwstr = "125kHz";
        break;
    case RF_BANDWIDTH_250:
        bwstr = "250kHz";
        break;
    case RF_BANDWIDTH_500:
        bwstr = "500kHz";
        break;
    default:
        bwstr = "UNKNOWN";
        break;
    }

    /* Use RFO, limit power +14dBm */
    if((gDevFlash.config.dtype >> TYPE_BITS_RFO) & 0x01){
        if(txpow > 14){
            txpow = 14;
        }
    } else {
        if(txpow < 2){
            txpow = 2;
        }
    }

    if(gPaEnable){
        txpow = 30;
    }

    osSaveCritical();
    osEnterCritical();

    printk("NET:\t%s\r\nTFREQ:\t%ukHz\r\nRFREQ:\t%ukHz\r\n",
           gDevFlash.config.prop.netmode ? "Node to Gateway":"Node to Node",
           freq/1000,gDevFlash.config.rxfreq/1000);

    printk("POW:\t%ddBm\r\nBW:\t%s\r\n"
           "TSF:\t%u\r\nRSF:\t%u\r\nCR:\t4/%u\r\nMODE:\t%s\r\nSYNC:\t0x%X\r\n"
           "PREM:\t%u,%u\r\nFIX:\t%u,%u\r\nCRC:\t%s\r\nTIQ:\t%s\r\nRIQ:\t%s\r\n",
           txpow,bwstr,
           gDevFlash.config.txsf,gDevFlash.config.rxsf,rps.cr + 4,
           gDevFlash.config.rps.modem?"LORA":"FSK",
           gDevFlash.config.syncword,
           gDevFlash.config.tprem,gDevFlash.config.rprem,
           gDevFlash.config.tfix,gDevFlash.config.rfix,
           rps.crc?"ON":"OFF",
           gDevFlash.config.rps.tiq?"ON":"OFF",
           gDevFlash.config.rps.riq?"ON":"OFF");

    printk("SEQ:\t%s\r\nIP:\t%s\r\nAES:\t%s\r\nACK:\t%s\r\n"
           "LDR:\t%u,%u\r\nLCP:\t%u\r\nLFT:\t%u\r\n"
           "FNB:\t0x%02X\r\nTYPE:\t0x%02X\r\n",
           gDevFlash.config.prop.seqMode?"ON":"OFF",
           gDevFlash.config.prop.ipMode?"ON":"OFF",
           notAes?"OFF":"ON",
           gDevFlash.config.prop.ack?"ON":"OFF",ldrtx,ldrrx,
           gDevFlash.config.lcp, gDevFlash.config.lftime,
           gDevFlash.config.fnb,
           gDevFlash.config.dtype);

    osExitCritical();

    return;
}

void DevCfg_UserDefault(uint8_t opts)
{
    if(gPaEnable){
        gDevFlash.config.txpow = 0;
        gDevFlash.config.dtype = (0x01 << TYPE_BITS_RFO);
    }
}

bool DevCfg_UserUpdate(uint8_t *data, uint32_t len)
{
    /** @todo: */
    return true;
}

bool DevUserInit(void)
{
    bool success = false;

    /* init default parameters */
    memset(&sADCConfig, 0, sizeof(sADCConfig));
    gParam.dev.ver = APP_DEV_VER;

    success = UserDebugInit(false, UART_BRATE_9600, UART_PARI_NONE);

    UserInitGPIO();

    return success;
}

void DevGetVol(uint32_t param1, uint16_t param2)
{
    uint32_t adc = 0;

    sADCConfig.ref = RefVolSelInBgr1p5;

    BSP_ADC_Enable(&sADCConfig);
    adc = BSP_ADC_Sample(0, AdcAVccDiV3Input);
    gParam.dev.vol = (adc*1500*3)/4095;

    /** example */
    /*
    adc = BSP_ADC_Sample(0, AdcExInputCH2);
    adc = (adc*1500)/4095;
    */
    BSP_ADC_Disable(NULL);

    gParam.dev.level = calc_level(32, MIN_VOL_LEVEL, MAX_VOL_LEVEL, gParam.dev.vol);
    gParam.dev.res = (gParam.dev.vol+50)/100;

    return;
}

/**
 *  NOTE:
 *  User should redefine print function to reduce code size
 *  by disable IAR print Automatic choice of formatter.
 */
int printk(const char *fmt_s, ...)
{
    static char outbuf[128] = {0};

    va_list ap;
    int result;
    uint32_t len = sizeof(outbuf);

    va_start(ap, fmt_s);
    result = vsnprintf(outbuf, len, fmt_s, ap);
    va_end(ap);

    if(result > 0){
        BSP_UartSend(DBG_UART_IDX, (uint8_t *)outbuf, result);
    }

    return result;
}

void BSP_WatchdogFeed(void)
{
    /** @todo: */
}
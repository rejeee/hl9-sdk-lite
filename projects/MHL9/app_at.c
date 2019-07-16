/*******************************************************************************
 * @file    app_at.c
 * @brief   AT Command task
 *
 * @version 0.0.1
 *******************************************************************************
 * @license Refer License or other description Docs
 * @author  Felix
 ******************************************************************************/
#include "app_at.h"
#include "at/at.h"
#include "at/at_config.h"
#include "mac/node/mac_radio.h"

/****
Global Variables
****/

/****
Global Functions
****/
void AT_Printf(const char *str)
{
    UserDebugWrite((uint8_t *)str, strlen(str));
    return;
}

void AT_LOG(const uint8_t *ptr, size_t len)
{
    UserDebugWrite(ptr, len);
    return;
}

/* return refer @AT_STATUS */
uint32_t AT_TxProcess(bool forward, uint8_t *buf, uint32_t len)
{
    uint32_t status = AT_STATUS_UNUSED;
    if(forward){
        /* FIXME: If necessary */
        /* MacRadio_TxProcess(recv_buf, rev_len, Dev_GetVol); */
        status = MacRadio_TxProcess(buf, len, NULL);
    } else {
        /* TODO: set customer parameters with AT+CMD =<x> */
    }

    return status;
}

/*!
 * @brief  Create the AT task
 */
bool AppATTask(void)
{
    gAtEnableType = AT_EN_NODE;

    return true;
}

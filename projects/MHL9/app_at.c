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
void AT_Printf(const char *str, size_t len)
{
    UserDebugWrite((uint8_t *)str, len);
    return;
}

/* return refer @AT_STATUS */
uint32_t AT_TxProcess(bool forward, uint8_t *buf, uint32_t len)
{
    if(forward){
        return MacRadio_TxProcess(buf, len);
    } else {
        /* TODO: set customer parameters */
        return AT_STATUS_UNUSED;
    }
}

/*!
 * @brief  Create the AT task
 */
bool AppATTask(void)
{
    gAtEnableType = AT_EN_NODE;

    return true;
}

/*******************************************************************************
 * @file    app_mac.c
 * @brief   the MAC task
 *
 * @version 0.0.1
 *******************************************************************************
 * @license Refer License or other description Docs
 * @author  Felix
 ******************************************************************************/
#include "app_mac.h"
#include "at/at_config.h"

/****
Global Variables
****/

/****
Local Variables
****/

/****
Global Functions
****/

/*!
 * @brief  Create the MAC task
 */
bool AppMacTask(void)
{
    bool result = Mac_Init(gDevFlash.config.dtype & DTYPE_BITS_RFO);

    return result;
}

/*******************************************************************************
 * @file    at_api.h
 * @brief   The common API
 *
 * @version 1.0.0
 *******************************************************************************
 * @license Refer License or other description Docs
 * @author  Felix
 ******************************************************************************/
#ifndef AT_API_H
#define AT_API_H

/****
Include Files
****/
#include <stdint.h>
#include <string.h>
#include "hal_def.h"

/** @name AT command version */
/*@{*/
/** @brief AT command version */
#define AT_VER                          11
/*@}*/

/** @brief AT OK response */
#define AT_STR_OK                       "\r\nOK\r\n"

/** @brief AT command minimal length */
#define AT_CMD_MIN_LEN                  4U

/** @brief AT command maximal length */
#define AT_CMD_MAX_LEN                  512U

/** @brief AT command max prefix length, example AT+XXXX= */
#define AT_CMD_PREFIX_LEN               10U

/** @brief AT command max parameter length, example AT+AK= XXXX */
#define AT_CMD_PARAM_LEN                32U

/** @brief the byte length of AT parameter. */
enum {
    CFG_1BYTE_LEN  = 1U,    /**< 1 Byte  */
    CFG_2BYTE_LEN  = 2U,    /**< 2 Byte  */
    CFG_4BYTE_LEN  = 4U,    /**< 4 Byte  */
    CFG_8BYTE_LEN  = 8U,    /**< 8 Byte  */
    CFG_16BYTE_LEN = 16U    /**< 16 Byte */
};

enum {
    LOWRATE_OP_AUTO = 0x00,
    LOWRATE_OP_ON,
    LOWRATE_OP_OFF
};

/* the address mode */
enum {
    SIP_MODE_NONE  = 0U,
    SIP_ADDR_MASK  = 0x0FU,
    SIP_SEQ_MASK   = 0xF0U
};

/* Device RX printf format */
enum {
    RX_MODE_NONE   = 0U,
    RX_MODE_FACTORY,
    RX_MODE_NUM
};

/**
 * @enum AT_STATUS
 * @brief return result definition
 */
typedef enum
{
    AT_STATUS_NONE     = 0x00,  /**< no response need print */
    AT_STATUS_OK,               /**< respone "OK" */
    AT_STATUS_UNKNOWN,          /**< ER00 command unknown */
    AT_STATUS_PARAM,            /**< ER01 parameter is error */
    AT_STATUS_ERR,              /**< ER02 execute command fail */
    AT_STATUS_BUSY,             /**< ER03 radio channel is busy */
    AT_STATUS_LEN_ERR,          /**< ER04 length is error(overlap or empty) */
    AT_STATUS_FLASH_ERR,        /**< ER05 R/W flash fail */
    AT_STATUS_MEM_ERR,          /**< ER06 no free memory */
    AT_STATUS_TIMEOUT,          /**< ER07 TX timeout */
    AT_STATUS_UNUSED,           /**< ER08 cmd unsupport */
    AT_STATUS_ACCESS,           /**< ER09 cannot access */
    AT_STATUS_RX_ERR,           /**< radio rx error */
    AT_STATUS_CAD_ERR,          /**< CAD error */
    /**> NOTE: the follow code depend user customer funtion */
    AT_STATUS_H,                /**< AT command list */
    AT_STATUS_I,                /**< print device version */
    AT_STATUS_Z,                /**< restore factory configuration */
    AT_STATUS_R,                /**< software reset */
    AT_STATUS_T,                /**< switch AT mode */
    AT_STATUS_CFG,              /**< print configuration */
    AT_STATUS_CSQ,              /**< print rssi and snr */
    AT_STATUS_UART,             /**< change UART settings */
    AT_STATUS_SET_RF,
    AT_STATUS_SET_SYNC,         /**< transpond sync config into TX module */
    AT_STATUS_SET_LDR,
    AT_STATUS_SLEEP,            /**< delay ok for sleep */
    AT_STATUS_RX_MODE           /**< need change RX printf style */
} AT_STATUS;

extern const uint32_t gDevFlashSize;

/** @name the follow function need customer implement */

/*@{*/

/**
 * @brief  save customer configuration
 *
 * return true if save successful else false.
 */
bool Flash_WriteParam(void);

/*@}*/

#endif

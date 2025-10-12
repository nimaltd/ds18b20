
/*
 * @file        ds18b20.h
 * @brief       ds18b20 driver
 * @author      Nima Askari
 * @version     2.0.0
 * @license     See the LICENSE file in the root folder.
 *
 * @note        All my libraries are dual-licensed. 
 *              Please review the licensing terms before using them.
 *              For any inquiries, feel free to contact me.
 *
 * @github      https://www.github.com/nimaltd
 * @linkedin    https://www.linkedin.com/in/nimaltd
 * @youtube     https://www.youtube.com/@nimaltd
 * @instagram   https://instagram.com/github.nimaltd
 *
 * Copyright (C) 2025 Nima Askari - NimaLTD. All rights reserved.
 */

#ifndef _DS18B20_H_
#define _DS18B20_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************************************/
/** Includes **/
/*************************************************************************************************/

#include <stdbool.h>
#include "main.h"
#include "ow.h"

/*************************************************************************************************/
/** Definitions **/
/*************************************************************************************************/

#define DS18B20_ID          0x28

/*************************************************************************************************/
/** Typedef/Struct/Enum **/
/*************************************************************************************************/

/*************************************************************************************************/
/* DS18B20 Command Codes */
typedef enum
{
  DS18B20_CMD_CONV   = 0x44,  /* Start temperature conversion */
  DS18B20_CMD_CONF   = 0x4E,  /* Write to configuration register */
  DS18B20_CMD_READ   = 0xBE,  /* Read scratchpad */
  DS18B20_CMD_RECALL = 0xB8,  /* Recall EEPROM */
  DS18B20_CMD_POWER  = 0xB4   /* Check power supply (parasite power mode) */

} ds18b20_cmd_t;

/*************************************************************************************************/
/* DS18B20 Resolution (conversion bits) */
typedef enum
{
  DS18B20_CNV_BIT_9   = 9,
  DS18B20_CNV_BIT_10  = 10,
  DS18B20_CNV_BIT_11  = 11,
  DS18B20_CNV_BIT_12  = 12

} ds18b20_cnv_bit_t;

/*************************************************************************************************/
/* DS18B20 Conversion Times (in milliseconds) for each resolution */
typedef enum
{
  DS18B20_CNV_TIM_9   = 100UL,
  DS18B20_CNV_TIM_10  = 200UL,
  DS18B20_CNV_TIM_11  = 400UL,
  DS18B20_CNV_TIM_12  = 800UL

} ds18b20_cnv_tim_t;

/*************************************************************************************************/
/* DS18B20 Configuration Structure */
typedef struct
{
  int8_t              alarm_low;  /* Low temperature alarm threshold */
  int8_t              alarm_high; /* High temperature alarm threshold */
  ds18b20_cnv_bit_t   cnv_bit;    /* Temperature resolution (9–12 bit) */

} ds18b20_config_t;

/*************************************************************************************************/
/* DS18B20 Driver Handle */
typedef struct
{
  ow_t                  ow;       /* One-Wire interface handle */
  uint32_t              time;     /* Start conversion timestamp */
  ds18b20_cnv_tim_t     cnv_time; /* Conversion time based on resolution */

} ds18b20_t;

/*************************************************************************************************/
/** API Functions **/
/*************************************************************************************************/

/* Initialize ds18b20 driver */
void      ds18b20_init(ds18b20_t *handle, ow_init_t *init);

/* Check if ds18b20 bus is busy */
bool      ds18b20_is_busy(ds18b20_t *handle);

/* Check if ds18b20 bus is busy */
ow_err_t  ds18b20_last_error(ds18b20_t *handle);

/* Start search to update all ROM IDs on the 1-Wire bus */
ow_err_t  ds18b20_update_rom_id(ds18b20_t *handle);

/* Send Start Conversation to All Devices */
ow_err_t  ds18b20_cnv(ds18b20_t *handle);

/* Set new configuration */
ow_err_t  ds18b20_conf(ds18b20_t *handle, ds18b20_config_t *config);

/* Check if ds18b20 conversation is done */
bool      ds18b20_is_cnv_done(ds18b20_t *handle);

#if (OW_MAX_DEVICE == 1)
/* Send read temperature request */
ow_err_t  ds18b20_req_read(ds18b20_t *handle);
#else
/* Send read temperature request */
ow_err_t  ds18b20_req_read(ds18b20_t *handle, uint8_t rom_id);
#endif

/* Read temperature in Celsius */
int16_t   ds18b20_read_c(ds18b20_t *handle);

/* Convert temperature from Celsius to Fahrenheit */
int16_t   ds18b20_cnv_to_f(int16_t temp_c);

/*************************************************************************************************/
/** End of File **/
/*************************************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* _DS18B20_H_ */

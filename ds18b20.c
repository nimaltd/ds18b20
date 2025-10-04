
/*
 * @file        ds18b20.c
 * @brief       OneWire communication driver
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

/*************************************************************************************************/
/** Includes **/
/*************************************************************************************************/

#include "ds18b20.h"

/*************************************************************************************************/
/** Function Implementations **/
/*************************************************************************************************/

/*************************************************************************************************/
/**
 * @brief Initialize ds18b20 handle with one-wire configuration.
 * @param[in] handle: Pointer to the ds18b20 handle to initialize.
 * @param[in] init: Pointer to initialization data (GPIO, pin, timer, callback).
 * @retval None
 */
void ds18b20_init(ds18b20_handle_t *handle, ow_init_t *init)
{
  assert_param(handle != NULL);
  assert_param(init != NULL);

  /* Set Default value after power-up */
  handle->cnv_time = DS18B20_CNV_TIM_12;

  /* Initialize one-wire */
  ow_init(&handle->ow, init);
}

/*************************************************************************************************/
/**
 * @brief Check if ds18b20 bus is busy.
 * @param[in] handle: Pointer to the ds18b20 handle.
 * @retval true if busy, false if idle
 */
__INLINE bool ds18b20_is_busy(ds18b20_handle_t *handle)
{
  assert_param(handle != NULL);
  return ow_is_busy(&handle->ow);
}

/*************************************************************************************************/
/**
 * @brief Get the last ds18b20 error.
 * @param[in] handle: Pointer to the ds18b20 handle.
 * @retval Last error code (ow_err_t)
 */
__INLINE ow_err_t ds18b20_last_error(ds18b20_handle_t *handle)
{
  assert_param(handle != NULL);
  return ow_last_error(&handle->ow);
}

/*************************************************************************************************/
/**
 * @brief Start search to update all ROM IDs on the 1-Wire bus.
 * @param[in] handle: Pointer to the ds18b20 handle.
 * @retval Last error code (ow_err_t)
 */
__INLINE ow_err_t ds18b20_update_rom_id(ds18b20_handle_t *handle)
{
  assert_param(handle != NULL);
  return ow_update_rom_id(&handle->ow);
}

/*************************************************************************************************/
/**
 * @brief Send Start Conversation to All Devices.
 * @param[in] handle: Pointer to the ds18b20 handle.
 * @retval Last error code (ow_err_t)
 */
ow_err_t ds18b20_cnv(ds18b20_handle_t *handle)
{
  assert_param(handle != NULL);

  /* Save Start time */
  handle->time = HAL_GetTick();

  /* Send Command */
  return ow_xfer(&handle->ow, DS18B20_CMD_CONV, NULL, 0, 0);
}

/*************************************************************************************************/
/**
 * @brief Set new configuration.
 * @param[in] handle: Pointer to the ds18b20 handle.
 * @param[in] config: Pointer to New configuration.
 * @retval Last error code (ow_err_t)
 */
ow_err_t ds18b20_conf(ds18b20_handle_t *handle, ds18b20_config_t *config)
{
  uint8_t w_data[3];
  /* base config for 12-bit */
  uint8_t conf_reg = 0x7F;

  assert_param(handle != NULL);
  assert_param(config != NULL);

  switch (config->cnv_bit)
  {
    case DS18B20_CNV_BIT_9:
      conf_reg = 0x1F;
      handle->cnv_time = DS18B20_CNV_TIM_9;
      break;
    case DS18B20_CNV_BIT_10:
      conf_reg = 0x3F;
      handle->cnv_time = DS18B20_CNV_TIM_10;
      break;
    case DS18B20_CNV_BIT_11:
      conf_reg = 0x5F;
      handle->cnv_time = DS18B20_CNV_TIM_11;
      break;
    default:
      conf_reg = 0x7F;
      handle->cnv_time = DS18B20_CNV_TIM_12;
      break;
  }

  /* Write data: TH, TL, config register */
  if (config->alarm_high > 125)
  {
    config->alarm_high = 125;
  }
  if (config->alarm_high < -55)
  {
    config->alarm_high = -55;
  }
  if (config->alarm_low > 125)
  {
    config->alarm_low = 125;
  }
  if (config->alarm_low < -55)
  {
    config->alarm_low = -55;
  }
  w_data[0] = config->alarm_high;
  w_data[1] = config->alarm_low;
  w_data[2] = conf_reg;

  /* Send command over 1-Wire: Skip ROM + Write Scratchpad */
  return ow_xfer(&handle->ow, DS18B20_CMD_CONF, w_data, 3, 0);
}

/*************************************************************************************************/
/**
 * @brief Check if ds18b20 conversation is done.
 * @param[in] handle: Pointer to the ds18b20 handle.
 * @retval True if Ready, otherwise false.
 */
bool ds18b20_is_cnv_done(ds18b20_handle_t *handle)
{
  assert_param(handle != NULL);

  return ((HAL_GetTick() - handle->time >= handle->cnv_time) ? true : false);
}

#if (OW_MAX_DEVICE == 1)
/*************************************************************************************************/
/**
 * @brief Send read temperature request.
 * @param[in] handle: Pointer to the ds18b20 handle.
 * @retval Last error code (ow_err_t)
 */
ow_err_t ds18b20_req_read(ds18b20_handle_t *handle)
{
  assert_param(handle != NULL);

  /* Send Read Command */
  return ow_xfer(&handle->ow, DS18B20_CMD_READ, NULL, 0, 9);
}
#else
/*************************************************************************************************/
/**
 * @brief Send read temperature request.
 * @param[in] handle: Pointer to the ds18b20 handle.
 * @param[in] rom_id: Selected ROM ID index.
 * @retval Last error code (ow_err_t)
 */
ow_err_t ds18b20_req_read(ds18b20_handle_t *handle, uint8_t rom_id)
{
  assert_param(handle != NULL);

  /* Send Read Command */
  return ow_xfer_by_id(&handle->ow, rom_id, DS18B20_CMD_READ, NULL, 0, 9);
}
#endif

/*************************************************************************************************/
/**
 * @brief Read temperature from buffer.
 * @param[in] handle: Pointer to the ds18b20 handle.
 * @retval retval Temperature in Celsius * 100 (e.g. 1025 = 10.25°C), return -10000 if error
 */
int16_t ds18b20_read_c(ds18b20_handle_t *handle)
{
  uint8_t r_data[9];
  uint16_t raw;
  int32_t temp_x100 = -10000;   // use 32-bit for intermediate math
  int8_t sign = 1;
  uint8_t resolution;

  assert_param(handle != NULL);

  do
  {
    /* Check received data length */
    if (ow_read_resp(&handle->ow, r_data, sizeof(r_data)) != sizeof(r_data))
    {
      break;
    }

    /* Check CRC */
    if (ow_crc(r_data, 8) != r_data[8])
    {
      break;
    }

    /* Read Raw data */
    raw = (uint16_t) (r_data[0] | (r_data[1] << 8));

    /* Check Sign */
    if (raw & 0x8000)
    {
      raw = (~raw + 1);
      sign = -1;
    }

    /* Determine resolution (9–12 bits) */
    resolution = ((r_data[4] & 0x60) >> 5) + 9;
    switch (resolution)
    {
    case DS18B20_CNV_BIT_9:
      /* 0.5°C per bit → 50 per bit in x100 units */
      temp_x100 = (raw >> 3) * 50;
      break;
    case DS18B20_CNV_BIT_10:
      /* 0.25°C per bit → 25 per bit in x100 units */
      temp_x100 = (raw >> 2) * 25;
      break;
    case DS18B20_CNV_BIT_11:
      /* 0.125°C per bit → 12.5 per bit in x100 units ≈ 125 / 10 */
      temp_x100 = (raw >> 1) * 125 / 10;
      break;
    case DS18B20_CNV_BIT_12:
      /* 0.0625°C per bit → 6.25 per bit in x100 units = 625 / 100 */
      temp_x100 = raw * 625 / 100;
      break;
    default:
      sign = 1;
      break;
    }

    temp_x100 *= sign;

  }
  while (0);

  return (int16_t) temp_x100;
}

/*************************************************************************************************/
/**
 * @brief Convert temperature from Celsius to Fahrenheit.
 * @param[in] temp_c: Temperature in Celsius * 100 (e.g., 1025 = 10.25°C)
 * @retval Temperature in Fahrenheit * 100 (e.g., 7234 = 72.34°F),
 *         returns -10000 if error
 */
int16_t ds18b20_cnv_to_f(int16_t temp_c)
{
  /* Check for error */
  if (temp_c == -10000)
  {
    return -10000;
  }

  /* Convert to Fahrenheit: F = C * 1.8 + 32 */
  /* Multiply everything by 100 to keep hundredths */
  /* F_x100 = (C_x100 * 9 / 5) + 3200 */
  return (int16_t)((int32_t) temp_c * 9) / 5 + 3200;
}

/*************************************************************************************************/
/** End of File **/
/*************************************************************************************************/

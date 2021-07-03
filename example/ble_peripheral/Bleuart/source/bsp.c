/**
 * @file       bsp.c
 * @copyright  Copyright (C) 2020 Thuan Le. All rights reserved.
 * @license    This project is released under the Thuan Le License.
 * @version    1.0.0
 * @date       2021-04-21
 * @author     Thuan Le
 * @brief      Board Support Package (BSP)
 * @note       None
 */

/* Includes ----------------------------------------------------------- */
#include "bsp.h"
#include "log.h"
#include "flash.h"
#include "sys.h"
#include "adc.h"
#include "damos_ram.h"

/* Private defines ---------------------------------------------------- */
#define BATT_VOLTAGE_MAX      (0.9)
#define BATT_VOLTAGE_MIN      (0.6)

static float m_batt_value = 0;

static int bsp_battery_init(void);
static void batt_adc_evt(adc_Evt_t *pev);

/* Function definitions ----------------------------------------------- */
void bsp_init(void)
{
  hal_gpio_pin_init(LED_INDICATE, GPIO_OUTPUT);
  hal_gpio_pin_init(BUZZER, GPIO_OUTPUT);
  hal_gpio_pin_init(HALL_SENSOR_PWM, GPIO_OUTPUT);

  hal_gpio_pin_init(HALL_SENSOR_LOGIC, GPIO_INPUT);
  hal_gpio_pin_init(USER_BUTTON, GPIO_INPUT);

  hal_gpio_pull_set(LED_INDICATE, PULL_DOWN);
  hal_gpio_pull_set(LED_INDICATE, GPIO_PULL_UP);

  hal_gpio_write(LED_INDICATE, 0);
  hal_gpio_write(HALL_SENSOR_PWM, 1);

  hal_gpioin_register(USER_BUTTON, NULL, bsp_pin_event_handler);
  hal_gpioin_register(HALL_SENSOR_LOGIC, NULL, bsp_pin_event_handler);

  // bsp_battery_init();
}

 /**
 * @brief           Battery init
 *
 * @param[in]       None 
 * @attention       None
 * 
 * @return          PPlus_SUCCESS or PPlus_ERR_INVALID_PARAM
 */
static int bsp_battery_init(void)
{
  int ret;
  adc_Cfg_t cfg = 
  {
    .is_continue_mode = FALSE,
    .is_differential_mode = FALSE,
    .is_high_resolution = TRUE,
  };
      
  ret = hal_adc_config_channel(cfg, batt_adc_evt);
  if(ret)
    return ret;

  hal_adc_start();
  return true;
}

uint8_t bsp_battery_get_level(void)
{
  float batt_voltage;
  uint8_t percent;

  hal_adc_start();
  WaitMs(10);
  batt_voltage = m_batt_value;

  if (BATT_VOLTAGE_MAX <= batt_voltage)
    percent = 100;
  else if (BATT_VOLTAGE_MIN >= batt_voltage)
    percent = 0;
  else
    percent = ((batt_voltage - BATT_VOLTAGE_MIN) / (BATT_VOLTAGE_MAX - BATT_VOLTAGE_MIN)) * 100;

  LOG("Battery percent is: %d \n", percent);
  return percent;
}

uint8_t bsp_flash_erase(uint32_t section_to_erase)
{
  return hal_flash_erase_sector(section_to_erase);
}

uint8_t bsp_flash_write(uint32_t addr, uint8_t *data , uint32_t len)
{
  return hal_flash_write(addr, data, len);
}

uint8_t bsp_flash_read(uint32_t addr, uint8_t *data, uint32_t len)
{
  return hal_flash_read(addr, data, len);
}

/**
 * @brief           Battery adc event
 *
 * @param[in]       <pev>     Pointer to adc event
 *
 * @attention       None 
 *
 * @return          None
 */
static void batt_adc_evt(adc_Evt_t *pev)
{
  if (pev->type == HAL_ADC_EVT_DATA)
    m_batt_value = hal_adc_value_cal(1, pev->data, pev->size, TRUE, FALSE);
}

/* End of file -------------------------------------------------------- */

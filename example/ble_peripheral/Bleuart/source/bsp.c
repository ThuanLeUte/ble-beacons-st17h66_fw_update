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
#include "damos_ram.h"

/* Private defines ---------------------------------------------------- */
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
  hal_gpio_write(LED_INDICATE, 1);
  hal_gpio_write(HALL_SENSOR_PWM, 1);

  hal_gpioin_register(USER_BUTTON, NULL, bsp_pin_event_handler);
  hal_gpioin_register(HALL_SENSOR_LOGIC, NULL, bsp_pin_event_handler);
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

/* End of file -------------------------------------------------------- */

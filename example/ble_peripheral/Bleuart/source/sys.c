/**
 * @file       sys.c
 * @copyright  Copyright (C) 2020 Thuan Le. All rights reserved.
 * @license    This project is released under the Thuan Le License.
 * @version    1.0.0
 * @date       2021-04-21
 * @author     Thuan Le
 * @brief      System file
 * @note       None
 */

/* Includes ----------------------------------------------------------- */
#include "sys.h"
#include "ble_misc_services.h"
#include "bsp.h"
#include "damos_ram.h"
#include "ble_timer.h"
#include "bleuart.h"

/* Private defines ---------------------------------------------------- */
/* Private variable --------------------------------------------------- */
static const uint8_t MODE_TABLE[5] = { 0, 1, 5, 10, 20 };

/* Function definitions ----------------------------------------------- */
static void m_sys_ble_click_count_notify(void)
{
  attHandleValueNoti_t click_count;
  click_count.len = 1;
  click_count.value[0] =  g_dispenser.click_count;
  mcs_notify(MCS_ID_CHAR_CLICK_COUNT, gapConnHandle, &click_count);
}

static void m_sys_ble_mode_selected_notify(void)
{
  attHandleValueNoti_t mode_selected;
  mode_selected.len = 1;
  mode_selected.value[0] =  g_dispenser.mode_selected;
  mcs_notify(MCS_ID_CHAR_MODE_SELECTION, gapConnHandle, &mode_selected);
}

static void m_sys_ble_identification_notify(void)
{
  attHandleValueNoti_t identification;
  identification.len = 4;
  osal_memcpy(identification.value, &g_dispenser.identification, identification.len);
  mcs_notify(MCS_ID_CHAR_IDENTIFICATON, gapConnHandle, &identification);
}

static void m_sys_ble_bottle_replacement_notify(void)
{
  attHandleValueNoti_t bottle_replacement;
  bottle_replacement.len = 1;
  bottle_replacement.value[0] =  g_dispenser.bottle_replacement;
  mcs_notify(MCS_ID_CHAR_BOTTLE_REPLACEMENT, gapConnHandle, &bottle_replacement);
}

static void m_sys_ble_send_notify(void)
{
  m_sys_ble_click_count_notify();
  m_sys_ble_mode_selected_notify();
  m_sys_ble_identification_notify();

  g_dispenser.click_count = 0;
  ble_timer_stop(TIMER_EXPIRED_CLICK_EVT);
}

void bsp_pin_event_handler(GPIO_Pin_e pin, IO_Wakeup_Pol_e type)
{
  hal_gpio_write(LED_INDICATE, 1);

  if (USER_BUTTON == pin)
  {
    LOG("Button is pressed. Start advertising !\n");

    g_dispenser.bottle_replacement++;
    ble_adv_enable(true);
    LOG("Bottle replacement = %d\n", g_dispenser.bottle_replacement);
  }
  else if (HALL_SENSOR_LOGIC == pin)
  {
    ble_timer_start(TIMER_EXPIRED_CLICK_EVT);

    if (++g_dispenser.click_count == MODE_TABLE[g_dispenser.mode_selected])
    {
      LOG("Start advertising !, Click: %d, Mode: %d\n", g_dispenser.click_count, g_dispenser.mode_selected);
      ble_adv_enable(true);
      ble_timer_stop(TIMER_EXPIRED_CLICK_EVT);
    }

    LOG("Mode select = %d, max click = %d\n", g_dispenser.mode_selected, MODE_TABLE[g_dispenser.mode_selected]);
    LOG("Click count = %d\n", g_dispenser.click_count);
  }

  hal_gpio_write(LED_INDICATE, 0);
}

void ble_timer_callback(void)
{
  LOG("Start advertising by timer, Click: %d, Mode: %d\n", g_dispenser.click_count, g_dispenser.mode_selected);
  ble_adv_enable(true);
}

void sys_on_ble_mcs_service_evt(mcs_evt_t *pev)
{
  switch (pev->evt_id)
  {
  case MSC_EVT_IDENTIFICATON_RECEIVED:
    mcs_get_parameter(MCS_ID_CHAR_IDENTIFICATON, &g_dispenser.identification);
    LOG("Write MCS_UUID_CHAR_IDENTIFICATION: %lu\n", g_dispenser.identification);

    bsp_flash_write(IDENTIFICATION_FLASH_ADDR, (uint8_t *) &g_dispenser.identification, 4);
    LOG("Reset device !!!\n");
    hal_system_soft_reset();
    break;

  case MSC_EVT_MODE_RECEIVED:
    mcs_get_parameter(MCS_ID_CHAR_MODE_SELECTION, &g_dispenser.mode_selected);

    if (g_dispenser.mode_selected < SYS_TRANSMIT_1_CLICK)
      g_dispenser.mode_selected = SYS_TRANSMIT_1_CLICK;
    else if (g_dispenser.mode_selected > SYS_TRANSMIT_20_CLICK)
      g_dispenser.mode_selected = SYS_TRANSMIT_20_CLICK;

    LOG("Write MCS_UUID_CHAR_MODE_SELECTION: %d\n", g_dispenser.mode_selected);
    bsp_flash_write(MODE_SELECTED_FLASH_ADDR, (uint8_t *) &g_dispenser.mode_selected, 1);
    LOG("Reset device !!!\n");
    hal_system_soft_reset();
    break;

  case MSC_EVT_IDENTIFICATON_READ:
    mcs_set_parameter(MCS_ID_CHAR_IDENTIFICATON,(uint8_t *) &g_dispenser.identification);
    LOG("Read MCS_UUID_CHAR_IDENTIFICATION: %lu\n", g_dispenser.identification);
    break;

  case MSC_EVT_MODE_READ:
    mcs_set_parameter(MCS_ID_CHAR_MODE_SELECTION, (uint8_t *) &g_dispenser.mode_selected);
    LOG("Read MCS_UUID_CHAR_MODE_SELECTION: %d\n", g_dispenser.mode_selected);
    break;

  case MSC_EVT_CLICK_COUNT_READ:
    mcs_set_parameter(MCS_ID_CHAR_CLICK_COUNT, (uint8_t *)&g_dispenser.click_count);
    LOG("Read MCS_ID_CHAR_CLICK_COUNT: %d\n", g_dispenser.click_count);
    g_dispenser.click_count = 0;
    break;

  case MSC_EVT_BOTTLE_REPLACEMENT_READ:
    mcs_set_parameter(MCS_ID_CHAR_BOTTLE_REPLACEMENT, (uint8_t *)&g_dispenser.bottle_replacement);
    LOG("Read MCS_ID_CHAR_BOTTLE_REPLACEMENT: %d\n", g_dispenser.bottle_replacement);
    g_dispenser.bottle_replacement = 0;
    break;

  default:
    break;
  }
}

/* End of file -------------------------------------------------------- */

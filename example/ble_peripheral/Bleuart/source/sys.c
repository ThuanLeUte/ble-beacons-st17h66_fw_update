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
#include <stdio.h>
#include "sys.h"
#include "ble_misc_services.h"
#include "ble_dvs_services.h"
#include "bsp.h"
#include "damos_ram.h"
#include "ble_timer.h"
#include "bleuart.h"
#include "osal_snv.h"

/* Private defines ---------------------------------------------------- */
/* Private variable --------------------------------------------------- */
static const uint8_t MODE_TABLE[5] = { 0, 1, 5, 10, 20 };
static uint16_t m_hall_time        = 0;
static uint16_t m_btn_time         = 0;

/* Function prototypes ------------------------------------------------ */
static void m_sys_witch_to_case(sys_device_case_t _case, bool reset);
static void m_sys_led_blink(uint8_t blink, uint8_t time);

/* Function definitions ----------------------------------------------- */
void sys_init(void)
{
  osal_snv_read(DEVICE_STORE_DATA_FS_ID, sizeof(device_t) / sizeof(uint8_t), &g_dispenser);

  if (g_dispenser.identification != 0)
    sprintf((char *)g_dispenser.device_name, "DISP-%d", g_dispenser.identification);

  // Set range for mode selected
  if (g_dispenser.mode_selected != SYS_TRANSMIT_1_CLICK &&
      g_dispenser.mode_selected != SYS_TRANSMIT_5_CLICK &&
      g_dispenser.mode_selected != SYS_TRANSMIT_10_CLICK &&
      g_dispenser.mode_selected != SYS_TRANSMIT_20_CLICK)
  {
    g_dispenser.mode_selected = SYS_TRANSMIT_1_CLICK;
  }

  // Log print
  LOG("Device name setting  : %s\n",  g_dispenser.device_name);
  LOG("Device case          : %d\n",  g_dispenser.device_case);
  LOG("Device FS flag       : %2x\n", g_dispenser.fs_flag);
  LOG("Device mode selected : %d\n",  g_dispenser.mode_selected);
  LOG("Device click count   : %d\n",  g_dispenser.click_count);
  LOG("Device identification: %lu\n", g_dispenser.identification);
  LOG("Device bottle        : %lu\n", g_dispenser.bottle_replacement);

  // Write default value to flash
  if (g_dispenser.fs_flag != DEVICE_FS_FLAG_DATA)
  {
    g_dispenser.fs_flag = DEVICE_FS_FLAG_DATA;
    osal_snv_write(DEVICE_STORE_DATA_FS_ID, sizeof(device_t) / sizeof(uint8_t), &g_dispenser);
    LOG("Write default device data to flash\n");
  }

  ble_set_device_name(g_dispenser.device_name, strlen((const char *)g_dispenser.device_name));
}

void sys_ble_disconneted_state(void)
{
  switch (g_dispenser.device_case)
  {
  case SYS_DEV_CASE_1:
    break;

  case SYS_DEV_CASE_2:
    m_sys_witch_to_case(SYS_DEV_CASE_1, true);
    break;

  case SYS_DEV_CASE_3:
    m_sys_witch_to_case(SYS_DEV_CASE_1, true);
    break;

  default:
    break;
  }
}

void bsp_pin_event_handler(GPIO_Pin_e pin, IO_Wakeup_Pol_e type)
{
  if (USER_BUTTON == pin)
  {
    LOG("Button is pressed \n");
    m_btn_time = 5000;
    ble_timer_start(TIMER_BUTTON_HANDLER_EVT);
  }
  else if (HALL_SENSOR_LOGIC == pin)
  {
    LOG("Hall is pressed \n");
    m_hall_time = 5000;
    ble_timer_start(TIMER_HALL_HANDLER_EVT);
  }
}

void ble_timer_button_handler(void)
{
  if (m_btn_time != 0)
  {
    m_btn_time -= 100;

    if (hal_gpio_read(USER_BUTTON) == 1)
    {
      ble_timer_stop(TIMER_HALL_HANDLER_EVT);
    }
    else if (m_btn_time == 0)
    {
      LOG("Bottle available \n");
      g_dispenser.bottle_replacement = 1;

      m_sys_led_blink(2, 2);
      m_sys_witch_to_case(SYS_DEV_CASE_3, true);
    }
  }
}

void ble_timer_hall_handler(void)
{
  if (m_hall_time != 0)
  {
    m_hall_time -= 100;

    if (hal_gpio_read(HALL_SENSOR_LOGIC) == 1)
    {
      m_sys_led_blink(1, 1);

      ble_timer_stop(TIMER_HALL_HANDLER_EVT);
      ble_timer_start(TIMER_EXPIRED_CLICK_EVT);

      if (++g_dispenser.click_count >= MODE_TABLE[g_dispenser.mode_selected])
      {
        LOG("Click: %d, Mode: %d\n", g_dispenser.click_count, g_dispenser.mode_selected);
        if (g_dispenser.device_case != SYS_DEV_CASE_3)
        {
          m_sys_witch_to_case(SYS_DEV_CASE_3, true);
        }
        else
        {
          osal_snv_write(DEVICE_STORE_DATA_FS_ID, sizeof(device_t) / sizeof(uint8_t), &g_dispenser);
        }
      }

      LOG("Mode select = %d, max click = %d\n", g_dispenser.mode_selected, MODE_TABLE[g_dispenser.mode_selected]);
      LOG("Click count = %d\n", g_dispenser.click_count);
    }
  }
  else if (m_hall_time == 0)
  {
    g_dispenser.bottle_replacement = 0;
    g_dispenser.click_count        = 0;

    m_sys_led_blink(3, 2);
    m_sys_witch_to_case(SYS_DEV_CASE_2, true);
  }
}

void ble_timer_expired_click(void)
{
  LOG("Time expired, Click: %d, Mode: %d\n", g_dispenser.click_count, g_dispenser.mode_selected);
  m_sys_witch_to_case(SYS_DEV_CASE_3, true);
}

void sys_on_ble_mcs_service_evt(mcs_evt_t *pev)
{
  switch (pev->evt_id)
  {
  case MSC_EVT_IDENTIFICATON_READ:
    mcs_set_parameter(MCS_ID_CHAR_IDENTIFICATON,(uint8_t *) &g_dispenser.identification);
    LOG("Read MCS_ID_CHAR_IDENTIFICATON: %lu\n", g_dispenser.identification);
    break;

  case MSC_EVT_MODE_READ:
    mcs_set_parameter(MCS_ID_CHAR_MODE_SELECTION, (uint8_t *) &g_dispenser.mode_selected);
    LOG("Read MCS_ID_CHAR_MODE_SELECTION: %d\n", g_dispenser.mode_selected);
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

void sys_on_ble_dss_service_evt(dss_evt_t *pev)
{
switch (pev->evt_id)
  {
  case DSS_EVT_IDENT_SETTING_RECEIVED:
    dss_get_parameter(DSS_ID_CHAR_IDENT_SETTING, &g_dispenser.identification);
    LOG("Write DSS_ID_CHAR_IDENT_SETTING: %lu\n", g_dispenser.identification);

    osal_snv_write(DEVICE_STORE_DATA_FS_ID, sizeof(device_t) / sizeof(uint8_t), &g_dispenser);
    break;

  case DSS_EVT_MODE_SETTING_RECEIVED:
    dss_get_parameter(DSS_ID_CHAR_MODE_SETTING, &g_dispenser.mode_selected);

    if (g_dispenser.mode_selected < SYS_TRANSMIT_1_CLICK)
      g_dispenser.mode_selected = SYS_TRANSMIT_1_CLICK;
    else if (g_dispenser.mode_selected > SYS_TRANSMIT_20_CLICK)
      g_dispenser.mode_selected = SYS_TRANSMIT_20_CLICK;

    LOG("Write DSS_ID_CHAR_MODE_SETTING: %d\n", g_dispenser.mode_selected);
    osal_snv_write(DEVICE_STORE_DATA_FS_ID, sizeof(device_t) / sizeof(uint8_t), &g_dispenser);
    break;

  case DSS_EVT_IDENT_SETTING_READ:
    dss_set_parameter(DSS_ID_CHAR_IDENT_SETTING, (uint8_t *)&g_dispenser.identification);
    LOG("Read DSS_ID_CHAR_IDENT_SETTING: %d\n", g_dispenser.identification);
    break;

    case DSS_EVT_MODE_SETTING_READ:
    dss_set_parameter(DSS_ID_CHAR_MODE_SETTING, (uint8_t *)&g_dispenser.mode_selected);
    LOG("Read DSS_ID_CHAR_MODE_SETTING: %d\n", g_dispenser.mode_selected);
    break;

  default:
    break;
  }
}

/* Private function definitions --------------------------------------- */
static void m_sys_witch_to_case(sys_device_case_t _case, bool reset)
{
  g_dispenser.device_case = _case;
  osal_snv_write(DEVICE_STORE_DATA_FS_ID, sizeof(device_t) / sizeof(uint8_t), &g_dispenser);
  LOG("Device switch to case: %d\n", _case);

  if (reset)
  {
    LOG("Reset device\n");
    hal_system_soft_reset();
  }
}

static void m_sys_led_blink(uint8_t blink, uint8_t time)
{
  for (uint8_t j = 0; j < time; j++)
  {
    for (uint8_t i = 0; i < blink; i++)
    {
      hal_gpio_write(LED_INDICATE, 1);
      WaitMs(50);
      hal_gpio_write(LED_INDICATE, 0);
      WaitMs(50);
    }
    WaitMs(200);
  }
}

/* End of file -------------------------------------------------------- */

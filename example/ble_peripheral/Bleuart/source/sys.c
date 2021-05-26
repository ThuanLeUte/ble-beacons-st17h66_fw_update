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
/* Function definitions ----------------------------------------------- */
static void m_ble_notify_humi(void)
{
  attHandleValueNoti_t humi_meas;
  humi_meas.len = 1;
  humi_meas.value[0] =  g_dispenser.click_count;
  mcs_notify(MCS_ID_CHAR_CLICK_COUNT, gapConnHandle, &humi_meas);
}

void bsp_pin_event_handler(GPIO_Pin_e pin, IO_Wakeup_Pol_e type)
{
  hal_gpio_write(LED_INDICATE, 1);
  if (USER_BUTTON == pin)
  {
    if (++g_dispenser.click_count == 5)
    {
      LOG("Send notify click count: %d\n", g_dispenser.click_count);
      m_ble_notify_humi();
      g_dispenser.click_count = 0;
    }
  }
  else if (HALL_SENSOR_LOGIC == pin)
  {
    if (++g_dispenser.click_count == 5)
    {
      LOG("Send notify click count: %d\n", g_dispenser.click_count);
      m_ble_notify_humi();
      g_dispenser.click_count = 0;
    }
  }

  LOG("Pin event handler: %d\n", pin);
  hal_gpio_write(LED_INDICATE, 0);
}

void periodic_1s_callback(void)
{

}

/* End of file -------------------------------------------------------- */

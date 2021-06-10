/**
 * @file       ble_timer.c
 * @copyright  Copyright (C) 2020 Thuan Le. All rights reserved.
 * @license    This project is released under the Thuan Le License.
 * @version    2.0.0
 * @date       2020-06-11
 * @author     Thuan Le
 * @brief      Timer for smart gadget.
 * @note       None
 */

/* Includes ----------------------------------------------------------- */
#include "ble_timer.h"
#include "OSAL.h"

/* Private define ----------------------------------------------------- */
#define TIMER_EXPIRED_CLICK_TIME         (60 * 1000) // 60 Seconds
#define TIMER_BUTTON_HANDLER_TIME        (100)       // 0.1 Seconds
#define TIMER_HALL_HANDLER_TIME          (100)       // 0.1 Seconds

/* Private variable --------------------------------------------------- */
static uint8_t m_task_id = 0;

/* Function definitions ----------------------------------------------- */
void ble_timer_init(uint8_t task_id)
{
  m_task_id = task_id;
}

void ble_timer_start(uint16_t event_id)
{
  if (event_id == TIMER_EXPIRED_CLICK_EVT)
  {
    osal_start_timerEx(m_task_id, event_id, TIMER_EXPIRED_CLICK_TIME);
  }
  else if(event_id == TIMER_BUTTON_HANDLER_EVT)
  {
    osal_start_reload_timer(m_task_id, event_id, TIMER_BUTTON_HANDLER_TIME);
  }
  else if(event_id == TIMER_HALL_HANDLER_EVT)
  {
    osal_start_reload_timer(m_task_id, event_id, TIMER_HALL_HANDLER_TIME);
  }
}

void ble_timer_stop(uint16_t event_id)
{
  osal_stop_timerEx(m_task_id, event_id);
}

uint16_t ble_timer_process_event(uint8_t task_id, uint16_t events)
{
  if (events & TIMER_EXPIRED_CLICK_EVT)
  {
    ble_timer_expired_click();
    return (events ^ TIMER_EXPIRED_CLICK_EVT);
  }
  else if (events & TIMER_BUTTON_HANDLER_EVT)
  {
    ble_timer_button_handler();
    return (events ^ TIMER_BUTTON_HANDLER_EVT);
  }
  else if (events & TIMER_HALL_HANDLER_EVT)
  {
    ble_timer_hall_handler();
    return (events ^ TIMER_HALL_HANDLER_EVT);
  }

  return 0;
}

/* End of file -------------------------------------------------------- */

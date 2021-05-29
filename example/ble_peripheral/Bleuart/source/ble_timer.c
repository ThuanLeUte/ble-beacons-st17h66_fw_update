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
#define TIMER_EXPIRED_CLICK     (60 * 1000) // 60 Seconds

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
    osal_start_timerEx(m_task_id, event_id, TIMER_EXPIRED_CLICK);
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
    ble_timer_callback();
    return (events ^ TIMER_EXPIRED_CLICK_EVT);
  }
  return 0;
}

/* End of file -------------------------------------------------------- */

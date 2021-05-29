/**
 * @file       ble_timer.h
 * @copyright  Copyright (C) 2020 Thuan Le. All rights reserved.
 * @license    This project is released under the Thuan Le License.
 * @version    1.0.0
 * @date       2020-06-11
 * @author     Thuan Le
 * @brief      Timer for smart gadget.
 * @note       None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __BLE_TIMER_H
#define __BLE_TIMER_H

/* Includes ----------------------------------------------------------- */
#include "stdint.h"

/* Public defines ---------------------------------------------------- */
#define TIMER_EXPIRED_CLICK_EVT   (60) // 60 Seconds

/* Public Callbacks -------------------------------------------------- */
/**
 * @brief           Function is called in timer 1s to execute all the functions.
 * 
 * @param           None
 * 
 * @attention       None
 * 
 * @return          None
 */
void ble_timer_callback(void); 

/* Public function prototypes ----------------------------------------- */
/**
 * @brief           Timer 1s init
 *
 * @param[in]       <task_id>  Task ID.
 *  
 * @attention       None
 *
 * @return          None
 */
void ble_timer_init(uint8_t task_id);

/**
 * @brief           Timer stop
 *
 * @param[in]       <task_id>   Task ID.
 *                  <event_id>  Event ID.
 *  
 * @attention       None
 *
 * @return          None
 */
void ble_timer_stop( uint16_t event_id);
void ble_timer_start( uint16_t event_id);

/**
 * @brief           Timer process event 
 *
 * @param[in]       <task_id>   Task ID.
 *                  <events>    Task Event
 *  
 * @attention       None
 *
 * @return          None
 */
uint16_t ble_timer_process_event(uint8_t task_id, uint16_t events);

#endif // __BLE_TIMER_H

/* End of file -------------------------------------------------------- */

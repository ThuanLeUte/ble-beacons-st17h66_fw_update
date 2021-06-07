/**
 * @file       sys.h
 * @copyright  Copyright (C) 2020 Thuan Le. All rights reserved.
 * @license    This project is released under the Thuan Le License.
 * @version    1.0.0
 * @date       2021-04-21
 * @author     Thuan Le
 * @brief      System file
 * @note       None
 */

/* Define to prevent recursive inclusion ----------------------------- */
#ifndef __SYS_H
#define __SYS_H

/* Includes ---------------------------------------------------------- */
#include "stdint.h"
#include "gpio.h"
#include "ble_misc_services.h"
#include "ble_dvs_services.h"

/* Private define ----------------------------------------------------------- */
#define SECTOR_ADDR                 (0x1103F000)
#define IDENTIFICATION_FLASH_ADDR   (SECTOR_ADDR)
#define MODE_SELECTED_FLASH_ADDR    (SECTOR_ADDR + 4)

/* Private Macros ----------------------------------------------------------- */
typedef enum
{
   SYS_TRANSMIT_1_CLICK  = 1
  ,SYS_TRANSMIT_5_CLICK  = 2
  ,SYS_TRANSMIT_10_CLICK = 3
  ,SYS_TRANSMIT_20_CLICK = 4
}
sys_transmit_mode_t;

typedef enum
{
   SYS_DEV_CASE_1 = 1
  ,SYS_DEV_CASE_2 = 2
  ,SYS_DEV_CASE_3 = 3
}
sys_device_case_t;

/* Public defines ---------------------------------------------------- */
/* Public function prototypes ---------------------------------------- */
/**
 * @brief           Board support package init
 *
 * @param[in]       None
 *  
 * @attention       None
 *
 * @return          None
 */
void sys_on_ble_mcs_service_evt(mcs_evt_t *pev);
void sys_on_ble_dss_service_evt(dss_evt_t *pev);
void sys_init(void);
void sys_ble_disconneted_state(void);


#endif // __SYS_H

/* End of file ------------------------------------------------------- */

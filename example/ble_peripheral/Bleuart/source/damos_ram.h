/**
 * @file       damos_ram.h
 * @copyright  Copyright (C) 2020 Thuan Le. All rights reserved.
 * @license    This project is released under the Thuan Le License.
 * @version    1.0.0
 * @date       2020-06-11
 * @author     Thuan Le
 * @brief      Damos RAM
 * @note       None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __DAMOS_RAM_H
#define __DAMOS_RAM_H

/* Includes ----------------------------------------------------------- */
#include "stdint.h"

/* Public defines ----------------------------------------------------- */
#define DEVICE_STORE_DATA_FS_ID      (0x88)
#define DEVICE_FS_FLAG_DATA          (0xAA)

/* Public enumerate/structure ----------------------------------------- */
typedef struct
{
  uint8_t  device_name[21];      // Device name
  uint8_t  device_case;          // Device case
  uint8_t  fs_flag;              // FS flag
  uint8_t  mode_selected;        // Mode selected
  uint8_t  click_count;          // Click availble value
  uint32_t identification;       // Identification value;
  uint8_t  bottle_replacement;   // Bottle replacement value;
}
device_t;

/* Public variables --------------------------------------------------- */
extern device_t g_dispenser;

/* Public Callbacks -------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */

#endif // __DAMOS_RAM_H

/* End of file -------------------------------------------------------- */

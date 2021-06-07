/**
 * @file       damos_ram.c
 * @copyright  Copyright (C) 2020 Thuan Le. All rights reserved.
 * @license    This project is released under the Thuan Le License.
 * @version    2.0.0
 * @date       2020-06-11
 * @author     Thuan Le
 * @brief      Damos Ram
 * @note       None
 */

/* Includes ----------------------------------------------------------- */
#include "damos_ram.h"

/* Function definitions ----------------------------------------------- */
/* Public variables --------------------------------------------------- */
device_t g_dispenser =
{
  .identification     = 0,
  .mode_selected      = 1,
  .click_count        = 0,
  .bottle_replacement = 0,
  .device_name        = {'D', 'I', 'S', 'P'},
  .fs_flag            = 0,
  .device_case        = 1,
};

/* End of file -------------------------------------------------------- */

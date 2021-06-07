/**
 * @file       ble_dvs_services.h
 * @copyright  Copyright (C) 2020 ThuanLe. All rights reserved.
 * @license    This project is released under the ThuanLe License.
 * @version    2.0.0
 * @date       2020-06-02
 * @author     Thuan Le
 * @brief      DSS (Device Setting Service)
 * @note       None
 */

/* Includes ----------------------------------------------------------- */
#include "ble_dvs_services.h"
#include "log.h"

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "damos_ram.h"

/* Private Defines ---------------------------------------------------------- */
#define DSS_UUID_SERV                      (0xFFF0)
#define DSS_UUID_CHAR_IDENT_SETTING        (0xFFF1)
#define DSS_UUID_CHAR_MODE_SETTING         (0xFFF2)

#define CHAR_IDENT_SETTING_VALUE_POS      (2)
#define CHAR_MODE_SETTING_VALUE_POS       (4)

/* Private Macros ----------------------------------------------------------- */
/* Private Defines ---------------------------------------------------------- */
// GATT Profile Device Setting Service UUID
static CONST uint8 DSS_UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(DSS_UUID_SERV), HI_UINT16(DSS_UUID_SERV)
};

// Characteristic Identification Setting UUID
static CONST uint8 DSS_CHAR_IDENT_SETTING_UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(DSS_UUID_CHAR_IDENT_SETTING), HI_UINT16(DSS_UUID_CHAR_IDENT_SETTING)
};

// Characteristic Mode Setting UUID
static CONST uint8 DSS_CHAR_MODE_SELECTION_UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(DSS_UUID_CHAR_MODE_SETTING), HI_UINT16(DSS_UUID_CHAR_MODE_SETTING)
};

// Characterictic property
uint8_t DSS_CHAR_PROPS[] =
{
  0,
  GATT_PROP_READ | GATT_PROP_WRITE |GATT_PROP_NOTIFY,
  GATT_PROP_READ | GATT_PROP_WRITE |GATT_PROP_NOTIFY,
};

// Profile Service attribute
static CONST gattAttrType_t dss_service = { ATT_BT_UUID_SIZE, DSS_UUID };

// Profile struct
static struct
{
  dss_cb_t app_cb;
  struct
  {
    struct
    {
      uint8_t ident_setting[4];     // Charaterictic identification setting value;
      uint8_t mode_setting[1];      // Charaterictic mode setting value;
    }
    value;
  }
  chars;
}
m_dss;

// Profile atrribute
static gattAttribute_t dss_atrr_tbl[] =
{
  // Profile Service
  {
    {ATT_BT_UUID_SIZE, primaryServiceUUID}, /* type */
    GATT_PERMIT_READ,                       /* permissions */
    0,                                      /* handle */
    (uint8 *)&dss_service                   /* p_value */
  },

  // Characteristic Identification Setting Declaration
  {
    {ATT_BT_UUID_SIZE, characterUUID},
    GATT_PERMIT_READ,
    0,
    &DSS_CHAR_PROPS[DSS_ID_CHAR_IDENT_SETTING]
  },

  // Characteristic Identification Setting Value
  {
    {ATT_BT_UUID_SIZE, DSS_CHAR_IDENT_SETTING_UUID},
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,
    0,
    m_dss.chars.value.ident_setting
  },

  // Characteristic Mode Setting Declaration
  {
    {ATT_BT_UUID_SIZE, characterUUID},
    GATT_PERMIT_READ,
    0,
    &DSS_CHAR_PROPS[DSS_ID_CHAR_MODE_SETTING]
  },

  // Characteristic Mode Setting Value
  {
    {ATT_BT_UUID_SIZE, DSS_CHAR_MODE_SELECTION_UUID},
    GATT_PERMIT_READ | GATT_PERMIT_WRITE,
    0,
    m_dss.chars.value.mode_setting
  },
};

/* Private function prototypes ---------------------------------------- */
static bStatus_t dss_read_attr_cb(uint16           conn_handle,
                                  gattAttribute_t *p_attr,
                                  uint8           *p_value, 
                                  uint8           *p_len, 
                                  uint16           offset, 
                                  uint8            max_len);

static bStatus_t dss_write_attr_cb(uint16          conn_handle, 
                                  gattAttribute_t *p_attr,
                                  uint8           *p_value, 
                                  uint8            len, 
                                  uint16           offset);

/*********************************************************************
 * PROFILE CALLBACKS
 */
static CONST gattServiceCBs_t dss_callbacks =
{
  dss_read_attr_cb,
  dss_write_attr_cb,
  NULL
};

/* Public function ----------------------------------------- */
bStatus_t dss_add_service(dss_cb_t cb)
{
  uint8 status = SUCCESS;

  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService(dss_atrr_tbl,
                                       GATT_NUM_ATTRS(dss_atrr_tbl),
                                       &dss_callbacks);

  LOG("dss_add_service:%x\n", status);

  m_dss.app_cb = cb;

  return ( status );
}

bStatus_t dss_set_parameter(dss_id_t char_id, void *value)
{
  bStatus_t ret = SUCCESS;

  switch (char_id)
  {
  case DSS_ID_CHAR_IDENT_SETTING:
    osal_memcpy(m_dss.chars.value.ident_setting, value, sizeof(m_dss.chars.value.ident_setting));
    break;

  case DSS_ID_CHAR_MODE_SETTING:
    osal_memcpy(m_dss.chars.value.mode_setting, value, sizeof(m_dss.chars.value.mode_setting));
    break;

  default:
    ret = INVALIDPARAMETER;
    break;
  }

  return (ret);
}

bStatus_t dss_get_parameter(dss_id_t char_id, void *value)
{
  bStatus_t ret = SUCCESS;

  switch (char_id)
  {
  case DSS_ID_CHAR_IDENT_SETTING:
    osal_memcpy(value, m_dss.chars.value.ident_setting, sizeof(m_dss.chars.value.ident_setting));
    break;

  case DSS_ID_CHAR_MODE_SETTING:
    osal_memcpy(value, m_dss.chars.value.mode_setting, sizeof(m_dss.chars.value.mode_setting));
    break;

  default:
    ret = INVALIDPARAMETER;
    break;
  }

  return (ret);
}

bStatus_t dss_notify(dss_id_t char_id, uint16 conn_handle, attHandleValueNoti_t *p_noti)
{
  bStatus_t ret;

  // Set the handle
  switch (char_id)
  {
  case DSS_ID_CHAR_IDENT_SETTING:
    p_noti->handle = dss_atrr_tbl[CHAR_IDENT_SETTING_VALUE_POS].handle;
    break;

  case DSS_ID_CHAR_MODE_SETTING:
    p_noti->handle = dss_atrr_tbl[CHAR_MODE_SETTING_VALUE_POS].handle;
    break;
 
  default:
    break;
  }

  ret = GATT_Notification(conn_handle, p_noti, FALSE);

  if (SUCCESS == ret)
  {
    LOG("dss_notify success\n");
  }

  return ret; 
}

/* Private Function definitions ----------------------------------------------- */
/**
 * @brief       Write an attribute.
 *
 * @param[in]   conn_handle     Connection message was received on
 *              p_attr          Pointer to attribute
 *              p_value         Pointer to data to be read
 *              p_len           Length of data to be read
 *              offset          Offset of the first octet to be read
 *
 * @return      Success or Failure
 */
static bStatus_t dss_write_attr_cb(uint16          conn_handle, 
                                  gattAttribute_t *p_attr,
                                  uint8           *p_value, 
                                  uint8            len, 
                                  uint16           offset)
{
  bStatus_t status = SUCCESS;
  dss_evt_t evt;

  LOG("Device Setting write callback\n");

  // If attribute permissions require authorization to write, return error
  if (gattPermitAuthorWrite(p_attr->permissions))
    return (ATT_ERR_INSUFFICIENT_AUTHOR); // Insufficient authorization

  if (p_attr->type.len == ATT_BT_UUID_SIZE)
  {
    uint16 uuid = BUILD_UINT16(p_attr->type.uuid[0], p_attr->type.uuid[1]);

    switch (uuid)
    {
    case DSS_UUID_CHAR_IDENT_SETTING:
      osal_memcpy(m_dss.chars.value.ident_setting, p_value, 4);

      evt.evt_id = DSS_EVT_IDENT_SETTING_RECEIVED;
      m_dss.app_cb(&evt);
      break;

    case DSS_UUID_CHAR_MODE_SETTING:
      osal_memcpy(m_dss.chars.value.mode_setting, p_value, 1);

      evt.evt_id = DSS_EVT_MODE_SETTING_RECEIVED;
      m_dss.app_cb(&evt);
      break;

    default:
      break;
    }
  }

  return (status);
}

/**
 * @brief       Read an attribute.
 *
 * @param[in]   conn_handle     Connection message was received on
 *              p_attr          Pointer to attribute
 *              p_value         Pointer to data to be read
 *              p_len           Length of data to be read
 *              offset          Offset of the first octet to be read
 *              max_len         Maximum length of data to be read
 *
 * @return      Success or Failure
 */
static uint8 dss_read_attr_cb(uint16 conn_handle,
                              gattAttribute_t *p_attr,
                              uint8 *p_value,
                              uint8 *p_len,
                              uint16 offset,
                              uint8 max_len)
{
  bStatus_t status = SUCCESS;
  dss_evt_t evt;

  LOG("Device setting read callback\n");

  // If attribute permissions require authorization to write, return error
  if (gattPermitAuthorWrite(p_attr->permissions))
    return (ATT_ERR_INSUFFICIENT_AUTHOR); // Insufficient authorization

  if (p_attr->type.len == ATT_BT_UUID_SIZE)
  {
    uint16 uuid = BUILD_UINT16(p_attr->type.uuid[0], p_attr->type.uuid[1]);

    switch (uuid)
    {
    case DSS_UUID_CHAR_IDENT_SETTING:
      evt.evt_id = DSS_EVT_IDENT_SETTING_READ;
      m_dss.app_cb(&evt);

      *p_len = 4;
      osal_memcpy(p_value, (uint8_t *) m_dss.chars.value.ident_setting, 4);
      break;

    case DSS_UUID_CHAR_MODE_SETTING:
      evt.evt_id = DSS_EVT_MODE_SETTING_READ;
      m_dss.app_cb(&evt);

      *p_len = 1;
      osal_memcpy(p_value, m_dss.chars.value.mode_setting, 1);
      break;

    default:
      break;
    }
  }

  return (status);
}

/* End of file -------------------------------------------------------- */

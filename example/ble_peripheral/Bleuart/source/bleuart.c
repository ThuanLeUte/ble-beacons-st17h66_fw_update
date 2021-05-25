
/**************************************************************************************************
  Filename:       bleuart.c
  Revised:        
  Revision:       

  Description:    This file contains the ble uart rawpass application
                  

**************************************************************************************************/

#include "types.h"
#include "bcomdef.h"
#include "bleuart_service.h"
#include "rf_phy_driver.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "gatt.h"
#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "bleuart_service.h"

#include "peripheral.h"
#include "gapbondmgr.h"

#include "bleuart.h"
#include "bleuart_service.h"
#include "log.h"
#include "osal_snv.h"
#include "flash.h"
#include "ota_app_service.h"
#include "bsp.h"
#include "ble_misc_services.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 */

// How often to perform periodic event
#define BUP_PERIODIC_EVT_PERIOD 5000

#define DEVINFO_SYSTEM_ID_LEN 8
#define DEVINFO_SYSTEM_ID 0

#define DEFAULT_DISCOVERABLE_MODE GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL 8

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL 10

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY 0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT 600

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL 6

// Length of bd addr as a string
#define B_ADDR_STR_LEN 15

// Default passcode
#define DEFAULT_PASSCODE 0

// Default GAP pairing mode
#define DEFAULT_PAIRING_MODE GAPBOND_PAIRING_MODE_WAIT_FOR_REQ

// Default MITM mode (TRUE to require passcode or OOB when pairing)
#define DEFAULT_MITM_MODE TRUE

// Default bonding mode, TRUE to bond
#define DEFAULT_BONDING_MODE TRUE

// Default GAP bonding I/O capabilities
#define DEFAULT_IO_CAPABILITIES GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT

uint8 Modify_BLEDevice_Data = 0;

uint16 advInt[6] = {80, 160, 320, 800, 1600, 3200};
uint8 advint = 2;

uint8 AT_bleuart_auto = 0x59;
uint8 AT_bleuart_sleep = 0;

uint8 AT_Tx_Power[8] = {0x1f, 0x1d, 0x17, 0x15, 0x0d, 0x0a, 0x06, 0x03};
uint8 AT_bleuart_txpower = 4;

uint8 AT_cnt_advdata = 9;

uint8 bleuart_TaskID; // Task ID for internal task/event processing

uint16 gapConnHandle;

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
    {
        // complete name
        21, // length of this data
        GAP_ADTYPE_LOCAL_NAME_COMPLETE,
        'U', 'a', 'r', 't', '-', 'T', 'e', 's', 't', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',

};

// advert data for bleuart
static uint8 advertData[] =
    {
        0x02, // length of this data
        GAP_ADTYPE_FLAGS,
        DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

        0x03,
        0x03, //Complete list of 16-bit UUIDs
        0xf0, 0xff,

        0x09,
        0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //reserved data
};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "Uart-Test           ";

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void bleuart_StateNotificationCB(gaprole_States_t newState);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t bleuart_PeripheralCBs =
    {
        bleuart_StateNotificationCB, // Profile State Change Callbacks
        NULL                         // When a valid RSSI is read from controller (not used by application)
};



/*********************************************************************
 * PUBLIC FUNCTIONS
 */

void on_bleuartServiceEvt(bleuart_Evt_t *pev)
{
  LOG("on_bleuartServiceEvt:%d\n", pev->ev);
  switch (pev->ev)
  {
  case bleuart_EVT_TX_NOTI_DISABLED:
    break;
  case bleuart_EVT_TX_NOTI_ENABLED:
    osal_set_event(bleuart_TaskID, BUP_OSAL_EVT_NOTIFY_DATA);
    break;
  case bleuart_EVT_BLE_DATA_RECIEVED:
    LOG("%s,%d\n", pev->data, pev->param);
    break;
  default:
    break;
  }
}

void bleuart_Init(uint8 task_id)
{
  bleuart_TaskID = task_id;

  // Setup the GAP
  VOID GAP_SetParamValue(TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL);

  // Setup the GAP Peripheral Role Profile
  {
    // device starts advertising upon initialization
    uint8 initial_advertising_enable = TRUE;

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint8 advChnMap = GAP_ADVCHAN_37 | GAP_ADVCHAN_38 | GAP_ADVCHAN_39;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    uint8 peerPublicAddr[] = {
        0x01,
        0x02,
        0x03,
        0x04,
        0x05,
        0x06};

    /***************************************************************************************************************/
    uint8 AT_mac_address[6];
    hal_flash_read(0x4004, AT_mac_address, 2);
    hal_flash_read(0x4000, AT_mac_address + 2, 4);

    /***************************************************************************************************************/

    GAPRole_SetParameter(GAPROLE_ADV_DIRECT_ADDR, sizeof(peerPublicAddr), peerPublicAddr);
    // set adv channel map
    GAPRole_SetParameter(GAPROLE_ADV_CHANNEL_MAP, sizeof(uint8), &advChnMap);

    // Set the GAP Role Parameters
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);
    GAPRole_SetParameter(GAPROLE_ADVERT_OFF_TIME, sizeof(uint16), &gapRole_AdvertOffTime);

    GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);
    GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);

    GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE, sizeof(uint8), &enable_update_request);
    GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16), &desired_min_interval);
    GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16), &desired_max_interval);
    GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY, sizeof(uint16), &desired_slave_latency);
    GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER, sizeof(uint16), &desired_conn_timeout);
  }

  // Set the GAP Characteristics
  osal_memcpy(attDeviceName, &scanRspData[2], 20);
  GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName);

  // Set advertising interval
  {
    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, advInt[advint]);
    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, advInt[advint]);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, advInt[advint]);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, advInt[advint]);
  }

  // Initialize GATT attributes
  GGS_AddService(GATT_ALL_SERVICES);         // GAP
  GATTServApp_AddService(GATT_ALL_SERVICES); // GATT attributes
  DevInfo_AddService();                      // Device Information Service

  // bleuart_AddService(on_bleuartServiceEvt);
  mcs_add_service();
  bsp_init();

  // Setup a delayed profile startup
  osal_set_event(bleuart_TaskID, BUP_OSAL_EVT_START_DEVICE);
}

static void BUP_ProcessOSALMsg(osal_event_hdr_t *pMsg)
{
  switch (pMsg->event)
  {
  default:
    // do nothing
    break;
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 bleuart_ProcessEvent(uint8 task_id, uint16 events)
{
  VOID task_id; // OSAL required parameter that isn't used in this function

  if (events & SYS_EVENT_MSG)
  {
    uint8 *pMsg;

    if ((pMsg = osal_msg_receive(bleuart_TaskID)) != NULL)
    {
      BUP_ProcessOSALMsg((osal_event_hdr_t *)pMsg);

      // Release the OSAL message
      VOID osal_msg_deallocate(pMsg);
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if (events & BUP_OSAL_EVT_START_DEVICE)
  {
    // Start the Device
    VOID GAPRole_StartDevice(&bleuart_PeripheralCBs);

    return (events ^ BUP_OSAL_EVT_START_DEVICE);
  }

  // enable adv
  if (events & BUP_OSAL_EVT_RESET_ADV)
  {
    uint8 initial_advertising_enable = TRUE;

    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8), &initial_advertising_enable);

    return (events ^ BUP_OSAL_EVT_RESET_ADV);
  }

  if (events & BUP_OSAL_EVT_NOTIFY_DATA)
  {
    LOG("BUP_OSAL_EVT_NOTIFY_DATA\n");
    return (events ^ BUP_OSAL_EVT_NOTIFY_DATA);
  }

  if (events & BUP_OSAL_EVT_BLE_TIMER)
  {
    LOG("BUP_OSAL_EVT_BLE_TIMER\n");
    return (events ^ BUP_OSAL_EVT_BLE_TIMER);
  }
  if (events & BUP_OSAL_EVT_UARTRX_TIMER)
  {
    LOG("BUP_OSAL_EVT_UARTRX_TIMER\n");
    return (events ^ BUP_OSAL_EVT_UARTRX_TIMER);
  }

  if (events & BUP_OSAL_EVT_UART_TX_COMPLETE)
  {
    LOG("BUP_OSAL_EVT_UART_TX_COMPLETE\n");
    return (events ^ BUP_OSAL_EVT_UART_TX_COMPLETE);
  }

  if (events & BUP_OSAL_EVT_UART_TO_TIMER)
  {
    LOG("RX TO\n");
    return (events ^ BUP_OSAL_EVT_UART_TO_TIMER);
  }

  if (events & BUP_OSAL_EVT_RF433_KEY)
  {
    return (events ^ BUP_OSAL_EVT_RF433_KEY);
  }

  if (events & BUP_OSAL_EVT_AT)
  {
    return (events ^ BUP_OSAL_EVT_AT);
  }

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */

static void bleuart_StateNotificationCB(gaprole_States_t newState)
{
  switch (newState)
  {
  case GAPROLE_STARTED:
  {
    uint8 ownAddress[B_ADDR_LEN];
    uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

    GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

    // use 6 bytes of device address for 8 bytes of system ID value
    systemId[0] = ownAddress[0];
    systemId[1] = ownAddress[1];
    systemId[2] = ownAddress[2];

    // set middle bytes to zero
    systemId[4] = 0x00;
    systemId[3] = 0x00;

    // shift three bytes up
    systemId[7] = ownAddress[5];
    systemId[6] = ownAddress[4];
    systemId[5] = ownAddress[3];

    DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

  }
  break;

  case GAPROLE_ADVERTISING:
    hal_gpio_write(UART_INDICATE_LED, 0);
    LOG("advertising!\n");
    break;

  case GAPROLE_CONNECTED:
    GAPRole_GetParameter(GAPROLE_CONNHANDLE, &gapConnHandle);
    hal_gpio_write(UART_INDICATE_LED, 1);
    LOG("connected handle[%d]!\n", gapConnHandle);
    break;

  case GAPROLE_CONNECTED_ADV:
    break;
  case GAPROLE_WAITING:
    break;

  case GAPROLE_WAITING_AFTER_TIMEOUT:
    break;

  case GAPROLE_ERROR:
    break;

  default:
    break;
  }
  gapProfileState = newState;

  VOID gapProfileState;
}

/*********************************************************************
*********************************************************************/

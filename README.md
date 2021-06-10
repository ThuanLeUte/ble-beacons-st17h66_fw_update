# ble-beacons-st17h66_fw_update

After flashing the firmware, the device will have the name "DISP".

The user will set the ID for the device then the device will change the name to "DISP-ID".

ESP32 will filter the device have the name "DISP" and get the ID from device name: "DISP-ID".

The device will have 3 CASE to broadcast the advertising service:

+ **CASE 1.** Broadcast without custom service.
+ **CASE 2.** Broadcast with Device Setting custom service.
+ **CASE 3.** Broadcast with Miscellaneous custom service.

## BEHAVIOR

1. **CASE 1:** 
  + The device is in the idle state, broadcast without custom service.
  + The device power up will be in CASE 1.

2. **CASE 2:** 
  + For setting identification, mode.
  + Enter to CASE 2 by pressing hall sensor for more than 5 seconds.
  + Connect phone to set the IDENFICATION_SET and MODE_SET.
  + IDENFICATION_SET: Format is 4 Byte Unsigned Int Little Endian.
  + MODE_SET: Format is 1 Byte Unsigned Int Little Endian.
  + When the device is disconnected to phone, device will go to "CASE 1".

3. **CASE 3:**
  + The device will change to CASE 3 when the click count (hall sensor) is equal to mode setting value.
  + The device will change to CASE 3 when the button is press more than 5 seconds (bottle available).
  + The ESP32 will connect and get the value from device.
  + ESP32 will read out the MODE_SELECTED, CLICK_COUNT, BOTTLE_REPLACEMENT. 
  + Device will reset CLICK_COUNT, BOTTLE_REPLACEMENT to 0 after read succeed.
  + ESP32 disconnects with the device, the device will go to the  "CASE 1".

## LED BEHAVIOR
+ The LED should be off in CASE 1.
+ The LED should be BLINK one time when click happen (hall sensor) (for troubleshooting).
+ The LED should be double BLINK twice when bottle available.
+ The LED should be triple BLINK twice when go to CASE 2
## BLE Service: Device Setting (0xFFF0)

### BLE Characteristics

+ IDENFICATION_SET    (0XFFF1)
+ MODE_SET            (0XFFF2)

## BLE Service: Miscellaneous (0xFFF3)

### BLE Characteristics

+ IDENFICATION         (0XFFF4) (Backup)
+ MODE_SELECTED        (0XFFF5)
+ CLICK_COUNT          (0XFFF6)
+ BOTTLE_REPLACEMENT   (0XFFF7)

## MODE

There will be 4 operational modes - default mode 1

+ **Mode 1.** Advertising Miscellaneous service after the 1th click
+ **Mode 2.** Advertising Miscellaneous service after the 5th click
+ **Mode 3.** Advertising Miscellaneous service after the 10th click
+ **Mode 4.** Advertising Miscellaneous service after the 20th click


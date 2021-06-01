# ble-beacons-st17h66_fw_update

After flashing the firmware, device will have the name is "disp".
User will set the id for device then device will be change the name to "disp-id"

ESP32 will filter device have the name "disp" and get the id from device name: "disp-id"

Device will have 3 case to broadcast the advertising service:

+ **Case 1.** Broadcast without custom service
+ **Case 2.** Broadcast with Device Setting custom service
+ **Case 3.** Broadcast with Miscellaneous custom service

## BEHAVIOR

1. When device power up, device will be in the "Case 1"

2. Case 2

+ To set IDENFICATION_SET, MODE_SET to device pressed button long (5+ second) and one click short , device advertising "Case 2", then connect phone and set IDENFICATION and MODE to device

+ IDENFICATION_SET: Format is 4 Byte Unsigned Int Little Endian.
+ MODE_SET: Format is 1 Byte Unsigned Int Little Endian.

+ When set success, phone disconnect to the device, device will go to "Case 1"

3. When device have click the same at mode, or bottle available. Device will be change to "Case 3". The ESP32 will connect and get the value then reset the data.
When ESP32 connected to device. It will read out the MODE_SELECTED, CLICK_COUNT,
BOTTLE_REPLACEMENT. Device will reset CLICK_COUNT, BOTTLE_REPLACEMENT to 0 after read success.
ESP32 disconnected to device. Device go to the  "Case 1"

## BLE Service: Device Setting (0xFFF0)

### BLE Characteristics

+ IDENFICATION_SET    (0XFFF1)
+ MODE_SET            (0XFFF2)

## BLE Service: Miscellaneous (0xFFF3)

### BLE Characteristics

+ IDENFICATION        (0XFFF4) (Backup)
+ MODE_SELECTED       (0XFFF5)
+ CLICK_COUNT         (0XFFF6)
+ BOTTLE_REPLACEMENT  (0XFFF7)

## MODE

There will be 4 operational modes - default mode 1

+ **Mode 1.** Advertising Miscellaneous service after the 1th click
+ **Mode 2.** Advertising Miscellaneous service after the 5th click
+ **Mode 3.** Advertising Miscellaneous service after the 10th click
+ **Mode 4.** Advertising Miscellaneous service after the 20th click


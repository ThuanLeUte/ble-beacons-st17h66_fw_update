# ble-beacons-st17h66_fw_update



## BLE Service: Miscellaneous (0xFFF0)
### BLE Characterictic:
 + IDENFICATION  (0XFFF1)
 + MODE_SELECTED      (0XFFF2)
 + CLICK_COUNT        (0XFFF3)
 + BOTTLE_REPLACEMENT (0XFFF4)

## MODE:
There will be 4 operational modes - default mode 1
- Mode 1- Advertising after the 1th click
- Mode 2- Advertising after the 5th click
- Mode 3- Advertising after the 10th click
- Mode 4- Advertising after the 20th click

## BEHAVIOR:
- To set IDENFICATION to device pressed button 1 time, device advertising, then connect phone and set IDENFICATION to device
Format is 4 Byte Unsigne Int Little Edian. When set success device will reset and new IDENFICATION was set even device POWER OFF

- To set MODE_SELECTED to device pressed button 1 time, device advertising, then connect phone and set MODE_SELECTED to device
Format is 1 Byte Unsigne Int Little Edian. When set success device will reset and new MODE_SELECTED was set even device POWER OFF

- When device power up device is in the NORMAL MODE (no advertised). When click reach the click of the mode, or button pressed
device start advertised. If hall still pressed click still count. Button still pressed bottle still count

- When ESP32 conneted to device. It will read out the IDENFICATION, MODE_SELECTED, CLICK_COUNT, 
BOTTLE_REPLACEMENT. Device will reset CLICK_COUNT, BOTTLE_REPLACEMENT to 0 after read success.
ESP32 disconneted to device. Device go to the NORMAL MODE ( no advertised)

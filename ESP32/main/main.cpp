#include <Arduino.h>
#include <stdio.h>

extern "C" {
#include <ps3.h>
}

#include "esp32-hal-bt.h"

void setup()
{
  // Too much slower and the bluetooth callback will trigger the watch dog
  // timer. Connect the serial TX pin of the ESP32 to D2 of the Teensy 2.0
  // (Serial1 RX).
  Serial.begin(115200);
}

uint8_t to_unsigned(int8_t value)
{
  return value < 0
    ? (uint8_t)(value + 128U)
    : (((uint8_t)value) + 128U);
}

uint8_t get_pov(bool up, bool down, bool left, bool right)
{
  // 8 1 2
  // 7 0 3
  // 6 5 4

  if (up && right)
    return 2;
  else if (right && down)
    return 4;
  else if (down && left)
    return 6;
  else if (up && left)
    return 8;
  else if (up)
    return 1;
  else if (right)
    return 3;
  else if (down)
    return 5;
  else if (left)
    return 7;
  else
    return 0;
}

uint8_t get_buttons(bool b1 = false, bool b2 = false,
                    bool b3 = false, bool b4 = false,
                    bool b5 = false, bool b6 = false,
                    bool b7 = false, bool b8 = false)
{
  return
    (uint8_t)((b1 ? 1 : 0) << 0) |
    (uint8_t)((b2 ? 1 : 0) << 1) |
    (uint8_t)((b3 ? 1 : 0) << 2) |
    (uint8_t)((b4 ? 1 : 0) << 3) |
    (uint8_t)((b5 ? 1 : 0) << 4) |
    (uint8_t)((b6 ? 1 : 0) << 5) |
    (uint8_t)((b7 ? 1 : 0) << 6) |
    (uint8_t)((b8 ? 1 : 0) << 7);
}

uint8_t get_checksum (uint8_t buffer[], const size_t buffer_len)
{
  uint8_t checksum = 0x55;
  for (size_t i = 0; i < buffer_len; ++i)
    checksum = (checksum + 1) ^ buffer[i];
  return checksum;
}

const uint8_t buffer_len = 14;
uint8_t buffer[buffer_len];

void controller_event_cb(ps3_t ps3, ps3_event_t event)
{
  // 0-1: "$G"
  // 2: X Axis (LX Axis)      (-128 -> 127 mapped to 0 -> 255)
  // 3: Y Axis (LY Axis)      (-128 -> 127 mapped to 0 -> 255)
  // 4: Z Axis (RX Axis)      (-128 -> 127 mapped to 0 -> 255)
  // 5: Z Rotation (RY Axis)  (-128 -> 127 mapped to 0 -> 255)
  // 6: Slider 1 (L Trigger)  (-128 -> 127 mapped to 0 -> 255)
  // 7: Slider 2 (R Trigger)  (-128 -> 127 mapped to 0 -> 255)
  // 8: POV Hat               (0 -> 8)
  // 9: Button 1-8
  // 9-0: Cross
  // 9-1: Circle
  // 9-2: Square
  // 9-3: Triangle
  // 9-4: L1
  // 9-5: R1
  // 9-6: Select
  // 9-7: Start
  // 10: Button 9-16
  // 10-0: L3
  // 10-1: R3
  // 10-2: Home
  // 11: Button 17-24      (bit field)
  // 12: Button 25-32      (bit field)
  // 13: Checksum

  buffer[0] = '$';
  buffer[1] = 'G';
  buffer[2] = to_unsigned(ps3.analog.stick.lx);
  buffer[3] = to_unsigned(ps3.analog.stick.ly);
  buffer[4] = to_unsigned(ps3.analog.stick.rx);
  buffer[5] = to_unsigned(ps3.analog.stick.ry);
  buffer[6] = ps3.analog.button.l2;
  buffer[7] = ps3.analog.button.r2;
  buffer[8] = get_pov(ps3.button.up, ps3.button.down,
                      ps3.button.left, ps3.button.right);
  buffer[9] = get_buttons(ps3.button.cross, ps3.button.circle,
                          ps3.button.square, ps3.button.triangle,
                          ps3.button.l1, ps3.button.r1,
                          ps3.button.select, ps3.button.start);
  buffer[10] = get_buttons(ps3.button.l3, ps3.button.r3, ps3.button.ps);
  buffer[11] = 0;
  buffer[12] = 0;
  buffer[13] = get_checksum(buffer, 10);

  #if 1
  // Binary output for real transmission.
  Serial.write(buffer, buffer_len);
  #else
  // Human readable output for debugging.
  for (uint8_t b : buffer)
  {
    Serial.print("0x");
    if (b <= 0xF)
      Serial.print("0");
    Serial.print((unsigned long)b, 16);
    Serial.print(" ");
  }
  Serial.println();
  #endif
}

int is_ps3_enabled = 0;

void loop()
{
  if (ps3IsConnected())
  {
    if (!is_ps3_enabled)
    {
      ps3Enable();
      is_ps3_enabled = 1;
    }
  }
  else
  {
    if (is_ps3_enabled) is_ps3_enabled = 0;
  }
}

#include <esp_bt.h>

// Use a tool such as "SixasixPairTool" to set the PS3 controllers MAC adress
uint8_t ps3PairedMacAddress[6] = {1, 2, 3, 4, 5, 6};

extern "C" void app_main()
{
  // Have to call something from esp32-hal-bt.c otherwise btInUse() returns
  // false, causing initArduino() to free the memory for bluetooth
  btStarted();

  initArduino();
  setup();

  ps3SetBluetoothMacAddress(ps3PairedMacAddress);
  ps3SetEventCallback(controller_event_cb);
  ps3Init();

  for (;;)
  {
    loop();
    vTaskDelay(1);
  }
}

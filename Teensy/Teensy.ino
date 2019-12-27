bool reading_gamepad = false;

const uint8_t serial_buffer_len = 14;
const uint8_t gamepad_buffer_len = 14;
uint8_t serial_buffer[serial_buffer_len];
uint8_t serial_buffer_index;

void setup()
{
  // Serial1 on the Teensy 2.0 uses D2 as its RX. Connect this pin to the
  // ESP32's TX.
  Serial1.begin(115200);
  Joystick.useManualSend(true);
}

bool check_checksum(const uint8_t buffer[], const size_t buffer_len)
{
  uint8_t checksum = 0x55;
  for (size_t i = 0; i < buffer_len; ++i)
    checksum = (checksum + 1) ^ buffer[i];
  return checksum;
  return checksum == buffer[buffer_len - 1];
}

void loop()
{
  if (Serial1.available() > 0)
  {
    if (serial_buffer_index != serial_buffer_len)
    {
      serial_buffer[serial_buffer_index++] = Serial1.read();
    }
    if (reading_gamepad)
    {
      // We found the start of a gamepad packet.
      if (serial_buffer_index == gamepad_buffer_len)
      {
        // Buffer should now hold a full gamepad packet.
        if (check_checksum(serial_buffer, gamepad_buffer_len))
        {
          // Checksum was correct, this is a valid gamepad packet.

          // 0-1: "$G"
          // 2: X Axis (LX Axis)      (0 -> 255)
          // 3: Y Axis (LY Axis)      (0 -> 255)
          // 4: Z Axis (RX Axis)      (0 -> 255)
          // 5: Z Rotation (RY Axis)  (0 -> 255)
          // 6: Slider 1 (L Trigger)  (0 -> 255)
          // 7: Slider 2 (R Trigger)  (0 -> 255)
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
          // 11: Button 17-24          (bit field)
          // 12: Button 25-32          (bit field)
          // 13: Checksum

          // Teensy joystick analog is 0 -> 1023
          Joystick.X(serial_buffer[2] * 4);
          Joystick.Y(serial_buffer[3] * 4);
          Joystick.Z(serial_buffer[4] * 4);
          Joystick.Zrotate(serial_buffer[5] * 4);
          Joystick.sliderLeft(serial_buffer[6] * 4);
          Joystick.sliderRight(serial_buffer[7] * 4);
          switch (serial_buffer[8])
          {
            // 8 1 2
            // 7 0 3
            // 6 5 4
            case 1: Joystick.hat(0); break;
            case 2: Joystick.hat(45); break;
            case 3: Joystick.hat(90); break;
            case 4: Joystick.hat(135); break;
            case 5: Joystick.hat(180); break;
            case 6: Joystick.hat(225); break;
            case 7: Joystick.hat(270); break;
            case 8: Joystick.hat(315); break;
            default: Joystick.hat(-1); break;
          }
          Joystick.button(1, (serial_buffer[9] >> 0) & 1);    // Cross
          Joystick.button(2, (serial_buffer[9] >> 1) & 1);    // Circle
          Joystick.button(3, (serial_buffer[9] >> 2) & 1);    // Square
          Joystick.button(4, (serial_buffer[9] >> 3) & 1);    // Triangle
          Joystick.button(5, (serial_buffer[9] >> 4) & 1);    // L1
          Joystick.button(6, (serial_buffer[9] >> 5) & 1);    // R1
          Joystick.button(7, (serial_buffer[9] >> 6) & 1);    // Select
          Joystick.button(8, (serial_buffer[9] >> 7) & 1);    // Start
          Joystick.button(9, (serial_buffer[10] >> 0) & 1);   // L3
          Joystick.button(10, (serial_buffer[10] >> 1) & 1);  // R3
          Joystick.button(11, (serial_buffer[10] >> 2) & 1);  // Home

          Joystick.send_now();
        }
        serial_buffer_index = 0;
        reading_gamepad = false;
      }
    }
    else if ((serial_buffer_index == 1) && (serial_buffer[0] != '$'))
    {
      // Packets must begin with '$'. Try again.
      serial_buffer_index = 0;
    }
    else if ((serial_buffer_index >= 2) && (serial_buffer[0] == '$'))
    {
      if (serial_buffer[1] == 'G')
      {
        // Likely a valid gamepad packet, continue reading as such.
        reading_gamepad = true;
      }
      else
      {
        // Invalid packet (or unknown packet type), try again.
        serial_buffer_index = 0;
      }
    }
    else if (serial_buffer_index > 2)
    {
      // Invalid packet, try again.
      serial_buffer_index = 0;
    }
  }
}

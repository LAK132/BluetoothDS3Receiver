# Bluetooth DS3 Receiver

This project makes use of the ESP32's ability to talk to a DS3 with the Teensy
2.0's ability to talk USB HID to a computer.

The only hardware requirement is that the ESP32 and the Teensy have their
grounds connected, and that the ESP32's Serial TX pin is connected to the
Teensy 2.0's Serial1 RX pin (D2).

I think my ESP32 board is 3.3v and the Teensy is 5v, but this hasn't been a
problem. If your ESP32 pin voltage is higher than what the Teensy can handle,
you will fry something. Make sure to check this before connecting them!

The ESP32 portion of this project requires the
[ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/)
to build.

The Teenys portion of this project requires
[Teensyduino](https://www.pjrc.com/teensy/teensyduino.html) to build.

Make sure to run `git submodule update --init` after cloning this repo to get
the dependencies for the ESP32 code.

TODO:
* Remove the Arduino dependency from the ESP32 code.
* Extend the serial packet protocol to support other HID devices
(mouse, keyboard, etc.)
* Design an all-in-one board to do this. Maybe even with a pair cable?
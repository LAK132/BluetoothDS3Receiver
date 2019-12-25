#include <Arduino.h>
#include "SPI.h"
#include <stdio.h>

extern "C" {
#include <ps3.h>
}

#include "esp32-hal-bt.h"

// Use a tool such as "SixasixPairTool" to set the PS3 controllers MAC adress
uint8_t ps3PairedMacAddress[6] = {1, 2, 3, 4, 5, 6};

uint8_t stick_deadzone = 10;

int in_deadzone(int8_t value, uint8_t deadzone)
{
    return value == deadzone ||
        (value > 0 && value < deadzone) ||
        (value < 0 && value > -deadzone);
}

void controller_event_cb(ps3_t ps3, ps3_event_t event)
{
    const float deadzone = 20.0f;
}

void setup()
{
    Serial.begin(115200);
}

int is_ps3_enabled = 0;

void loop()
{
    if (ps3IsConnected())
    {
        if (!is_ps3_enabled)
        {
            printf("Connected!\n");
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

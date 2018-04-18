#include <Arduino.h>

#include <actuators.h>
#include <config.h>
#include <info.h>
#include <defaults.h>
#include <interface.h>
#include <sensors.h>

#include <TimeAlarms.h>

void setup()
{
    config::setup();
    sensors::setup();
    actuators::setup();
    interface::setup();
}

void loop()
{
    sensors::loop();
    actuators::loop();
    interface::loop();

    Alarm.delay(0);
}

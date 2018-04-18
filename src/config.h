#pragma once

#include <Arduino.h>

#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#include <DS1307RTC.h>
#include <Time.h>
#include <TimeAlarms.h>

#include <actuators.h>
#include <defaults.h>
#include <sensors.h>

namespace config {

extern float high_temperature_alarm;
extern float low_temperature_alarm;

extern actuators::LedStrip white;
extern actuators::LedStrip red;
extern actuators::LedStrip green;
extern actuators::LedStrip blue;
extern actuators::LedStrip uv;

extern actuators::Feeding feeding;

extern struct PHCalibrationValue ph_calibration;

extern LCD* lcd;

void update_eeprom();
bool load_from_eeprom();

void setup();
}

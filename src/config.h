#pragma once

namespace config {

extern float high_temperature_alarm;
extern float low_temperature_alarm;

extern bool co2_automatic;
extern bool lights_automatic;

extern int tds_address;

// extern struct PHCalibrationValue ph_calibration;

void update_eeprom();
bool load_from_eeprom();

void setup();

} // namespace config

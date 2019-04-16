#pragma once

#include <Arduino.h>

namespace sensors {

// Sensor aggregated data
struct SensorsData {

    struct {
        bool sensor_ok;
        unsigned long last_read; // in millis
        unsigned long time_between_reads;

        float external;
        float internal;
    } ds18b20;

    struct {
        bool sensor_ok;
        unsigned long last_read; // in millis
        unsigned long time_between_reads;

        word tds;
    } tds;
};

void setup();
void loop();

SensorsData& get_sensors_data();

} // namespace sensors

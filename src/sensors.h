#pragma once
#include <Arduino.h>
#include <AnalogPHMeter.h>

namespace sensors {

struct SensorConf {
    uint8_t pin;
    unsigned long time_between_reads;
};

struct Bmp280Conf {
    int address;
    unsigned long time_between_reads;
    float altitude_at_your_locality;
};

struct TdsConf {
    uint8_t pin;
    unsigned long time_between_reads;
    float adc_ref;
    float adc_range;
};

// Sensor aggregated data
struct SensorsData {
    struct {
        bool sensor_ok;
        unsigned long last_read; // in millis

        float pressure;
        float temperature;
        float altitude;
    } bmp280t;

    struct {
        bool sensor_ok;
        unsigned long last_read; // in millis

        float humidity;
        float temperature;
    } dht11;

    struct {
        bool sensor_ok;
        unsigned long last_read; // in millis

        float external;
        float internal;
    } ds18b20;

    struct {
        bool sensor_ok;
        unsigned long last_read; // in millis

        float ph;
    } ph;

    struct {
        bool sensor_ok;
        unsigned long last_read; // in millis

        uint16_t tds;
    } tds;
};

void setup();
void loop();

SensorsData& get_sensors_data();

namespace PH {

void calibrate_ph4();
void calibrate_ph7();
void calibrate_ph10();

float get_ph();

} // namespace PH

} // namespace sensors

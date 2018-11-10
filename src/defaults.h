#pragma once

#include <Arduino.h>

namespace defaults {

const unsigned long serial_baud = 115200;
const byte n_phases = 8;
const bool co2_automatic = true;
const bool lights_automatic = true;

// Actuators configuration
const float high_temperature_alarm = 29.5; // Celsius
const float low_temperature_alarm = 23.0;  // Celsius

// Interface
// const byte lcd_addr = 0x27;
// const byte screen_cols = 20;
// const byte screen_rows = 4;
const byte lcd_addr = 0x3F;
const byte screen_cols = 16;
const byte screen_rows = 2;
const byte screen_size = screen_cols * screen_rows;

// Tiempo que esperamos presionar una tecla hasta apagar el lcd
const unsigned long max_inactive_millis = 60000;

struct SensorConf {
    byte pin;
    unsigned long time_between_reads;
};

struct TdsConf {
    byte pin;
    unsigned long time_between_reads;

    float adc_ref;   // reference voltage on ADC, default 5.0V on Arduino UNO
    float adc_range; // 1024 for 10bit ADC;4096 for 12bit ADC
};

struct HourMinute {
    byte hour;
    byte minute;

    word minutes() const;
};

struct Period {
    HourMinute init, end;
    bool in_period(const HourMinute& hm) const;
};

struct PhotoPeriod {
    Period period;
    byte phases_on;
};

// Photoperiods
const PhotoPeriod photo_periods[] = {
    {{{7, 30}, {11, 0}}, 1}, // Photoperiodo 1
    {{{11, 1}, {12, 0}}, 2}, // Photoperiodo 2
    {{{12, 1}, {13, 0}}, 4}, // Photoperiodo 3
    {{{13, 1}, {14, 0}}, 5}, // Photoperiodo 4
    {{{14, 1}, {15, 0}}, 7}, // Photoperiodo 5
    {{{15, 1}, {16, 0}}, 8}, // Photoperiodo 6
    {{{16, 1}, {17, 0}}, 7}, // Photoperiodo 7
    {{{17, 1}, {18, 0}}, 5}, // Photoperiodo 8
    {{{18, 1}, {19, 0}}, 4}, // Photoperiodo 9
    {{{19, 1}, {21, 0}}, 2}, // Photoperiodo 10
    {{{21, 1}, {22, 30}}, 1} // Photoperiodo 11
};

struct Co2Period {
    Period period;
    bool on;
};

const Co2Period co2_period[] = {{{{6, 0}, {12, 0}}, true}};

// Temperature
// https://naylampmechatronics.com/blog/46_Tutorial-sensor-de-temperatura-DS18B20.html
const SensorConf ds18b20_conf{A0, 15000};

// TDS once per hour
const TdsConf tds_conf{A4, 3600000, 5.0, 1024};

// pines
const struct {

    byte co2;
    byte filter;
    byte heater;
    byte pump;
    byte fan;

    byte warn_led;

    byte btn_co2;
    byte btn_pump;
    byte btn_filter;
    byte btn_lights;

    byte encoder_btn;
    byte encoder_dt;
    byte encoder_sw;

    byte phases[n_phases];

} pines = {
    0, // co2
    0, // filter
    0, // heater
    0, // pump
    0, // fan

    0, // warn_led

    0, // btn_co2
    0, // btn_pump
    0, // btn_filter
    0, // btn_lights

    0, // encoder_btn
    0, // encoder_dt
    0, // encoder_sw

    {0, 0, 0, 0, 0, 0, 0, 0}, // phases[n_phases]
};

} // namespace defaults

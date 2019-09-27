#pragma once

#include <Arduino.h>
#include <pins_arduino.h>

namespace defaults {

///////////////////////////////////////////////
// Some structs needed to group sensors and actuators in configuration
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
    HourMinute init;
    HourMinute end;

    bool in_period(const HourMinute& hm) const;
};

struct PhotoPeriod {
    Period period;
    byte phases_on;
};

///////////////////////////////////////////////
// Defaults start here
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

// Photoperiods
const PhotoPeriod photo_periods[] = {
    // Each photoperiod define an interval and a number of phases on
    {{{8, 0}, {11, 0}}, 1},
    {{{11, 1}, {12, 0}}, 3},
    {{{12, 1}, {13, 0}}, 5},
    {{{13, 1}, {18, 0}}, 8},
    {{{18, 1}, {19, 0}}, 5},
    {{{19, 1}, {21, 0}}, 3},
    {{{21, 1}, {22, 00}}, 1},
    {{{0, 0}, {0, 0}}, 0}
};

const Period co2_period = {{7, 0}, {14, 0}};

// https://naylampmechatronics.com/blog/46_Tutorial-sensor-de-temperatura-DS18B20.html
// Temperature once per minute
const SensorConf ds18b20_conf{A1, 30000};

// TDS once per 10 minutes
const TdsConf tds_conf{A2, 30000L, 5.0, 1024.0};

// pines
const struct {
    byte co2;
    byte filter;
    byte heater;
    byte pump;
    byte fan;

    byte warn_led;

    //	byte btn_pump;
    //	byte btn_filter;

    // https://howtomechatronics.com/tutorials/arduino/rotary-encoder-works-use-arduino/
    // Interrrupt pins: Mega, Mega2560, MegaADK	2, 3, 18, 19, 20, 21
    byte encoder_sw;
    byte encoder_dt;
    byte encoder_clk; 
    byte phases[n_phases];
} pines = {
    // setup pines here

    46, // co2
    52, // filter
    50, // heater
    48, // pump
    0,  // fan

    7, // warn_led

    //		49, // btn_pump
    //		47, // btn_filter
    // // Interrrupt pins: Mega, Mega2560, MegaADK	2, 3, 18, 19, 20, 21
    4, // encoder_sw
    3, // encoder_dt
    2, // encoder_clk
    {31, 33, 35, 37, 39, 41, 43, 45}  // phases[n_phases]
};

}
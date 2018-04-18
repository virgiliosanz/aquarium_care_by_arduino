#pragma once

#include <config.h>
#include <actuators.h>
#include <sensors.h>

namespace defaults {

const unsigned long serial_baud = 115200;

// Sensor Configuration
const sensors::SensorConf dht_conf{A1, 20000};         // Humidity
const sensors::SensorConf ds18b20_conf{A0, 15000};     // Temperature
const sensors::Bmp280Conf bmp280_conf{0x76, 20000, 1001.1f}; // Pressure - Madrid: 1001.1f Noja: 10.0f
const sensors::SensorConf ph_conf{A2, 3600000}; // PH, once per hour
const sensors::SensorConf tds_conf{A4, 3600000}; // TDS once per hour

// Actuators configuration
const float high_temperature_alarm = 29.5; // Celsius
const float low_temperature_alarm = 23.0;  // Celsius

const struct {
    uint8_t white_strip;
    uint8_t red_strip;
    uint8_t green_strip;
    uint8_t blue_strip;
    uint8_t uv_strip;

    uint8_t fan;
    uint8_t filter;
    uint8_t heater;
    uint8_t air_pump;

    uint8_t warning_led;

    uint8_t stepper1;
    uint8_t stepper2;
    uint8_t stepper3;
    uint8_t stepper4;

    uint8_t btn_feed;
    uint8_t btn_filter;
    uint8_t btn_lights;

    uint8_t btn_up;
    uint8_t btn_down;
    uint8_t btn_left;
    uint8_t btn_right;
    uint8_t btn_ok;
    uint8_t btn_esc;

} pines = {6, 2, 3, 4, 5, 24, 26, 28, 30, 52, 40, 38,
           36, 34, 50, 46, 48,  A7, A9,  A8, A11, A6, A10};

using PeriodType = actuators::PhotoPeriod::Type;

const actuators::LedStrip white = {{{{7, 0}, {12, 0}, PeriodType::rise},
                                    {{12, 0}, {17, 0}, PeriodType::on},
                                    {{17, 0}, {22, 0}, PeriodType::fall}},
                                   pines.white_strip};

const actuators::LedStrip red = {{{{6, 30}, {9, 0}, PeriodType::rise},
                                  {{9, 0}, {15, 0}, PeriodType::on},
                                  {{15, 0}, {17, 0}, PeriodType::fall}},
                                 pines.red_strip};

const actuators::LedStrip green = {{{{13, 0}, {14, 0}, PeriodType::rise},
                                    {{14, 0}, {15, 0}, PeriodType::on},
                                    {{15, 0}, {17, 0}, PeriodType::fall}},
                                   pines.green_strip};

const actuators::LedStrip blue = {{{{16, 0}, {18, 0}, PeriodType::rise},
                                   {{18, 0}, {20, 0}, PeriodType::on},
                                   {{20, 0}, {23, 0}, PeriodType::fall}},
                                  pines.blue_strip};

const actuators::LedStrip uv = {{{{17, 0}, {22, 0}, PeriodType::rise},
                                   {{22, 0}, {23, 59}, PeriodType::on},
                                   {{0, 1}, {2, 0}, PeriodType::fall}},
                                  pines.uv_strip};

const actuators::Feeding feeding = {{32, 150, 1},
                                    {8, 0}, {14, 0}, {20, 0},
                                    dowSaturday,
                                    false};

// Interface

const uint8_t lcd_addr = 0x27;
const uint8_t screen_cols = 20;
const uint8_t screen_rows = 4;
const uint8_t screen_size = screen_cols * screen_rows;

// Tiempo que esperamos presionar una tecla hasta apagar el lcd
const unsigned long max_inactive_millis = 60000;
}

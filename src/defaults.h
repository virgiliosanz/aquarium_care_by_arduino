#pragma once

#include <actuators.h>
#include <config.h>
#include <sensors.h>

namespace defaults {

const unsigned long serial_baud = 115200;

// Sensor Configuration
const sensors::SensorConf dht_conf{A1, 20000};     // Humidity
const sensors::SensorConf ds18b20_conf{A0, 15000}; // Temperature
const sensors::Bmp280Conf bmp280_conf{
    0x76, 20000, 1001.1f}; // Pressure - Madrid: 1001.1f Noja: 10.0f
const sensors::SensorConf ph_conf{A2, 3600000};  // PH, once per hour
const sensors::SensorConf tds_conf{A4, 3600000}; // TDS once per hour

// Actuators configuration
const float high_temperature_alarm = 29.5; // Celsius
const float low_temperature_alarm = 23.0;  // Celsius

const struct {
  // Controlling: Ultraviolet leds (Blue), White Leds (Green) & Fan (Red)
  byte p9813_data_pin_1;
  byte p9813_clock_pin_1;

  // Controlling RGB Led Strip
  byte p9813_data_pin_2;
  byte p9813_clock_pin_2;

  byte co2;
  byte filter;
  byte heater;
  byte air_pump;

  byte warning_led;

  byte stepper1;
  byte stepper2;
  byte stepper3;
  byte stepper4;

  byte btn_feed;
  byte btn_filter;
  byte btn_lights;

  byte btn_up;
  byte btn_down;
  byte btn_left;
  byte btn_right;
  byte btn_ok;
  byte btn_esc;

} pines = {11, 10, 5,  4,  24, 26, 28, 30, 52,  38, 36,
           34, 32, 50, 46, 48, A7, A9, A8, A11, A6, A10};

using PeriodType = actuators::PhotoPeriod::Type;

const actuators::LedStrip white = {{{{7, 0}, {12, 0}, PeriodType::rise},
                                    {{12, 0}, {17, 0}, PeriodType::on},
                                    {{17, 0}, {22, 0}, PeriodType::fall}}};

const actuators::LedStrip uv = {{{{17, 0}, {22, 0}, PeriodType::rise},
                                 {{22, 0}, {23, 59}, PeriodType::on},
                                 {{0, 1}, {2, 0}, PeriodType::fall}}};

const actuators::LedStrip red = {{{{6, 30}, {9, 0}, PeriodType::rise},
                                  {{9, 0}, {15, 0}, PeriodType::on},
                                  {{15, 0}, {17, 0}, PeriodType::fall}}};

const actuators::LedStrip green = {{{{13, 0}, {14, 0}, PeriodType::rise},
                                    {{14, 0}, {15, 0}, PeriodType::on},
                                    {{15, 0}, {17, 0}, PeriodType::fall}}};

const actuators::LedStrip blue = {{{{16, 0}, {18, 0}, PeriodType::rise},
                                   {{18, 0}, {20, 0}, PeriodType::on},
                                   {{20, 0}, {23, 0}, PeriodType::fall}}};

const actuators::Feeding feeding = {{32, 150, 1}, {8, 0},      {14, 0},
                                    {20, 0},      dowSaturday, false};

// Interface

const byte lcd_addr = 0x27;
const byte screen_cols = 20;
const byte screen_rows = 4;
const byte screen_size = screen_cols * screen_rows;

// Tiempo que esperamos presionar una tecla hasta apagar el lcd
const unsigned long max_inactive_millis = 60000;
} // namespace defaults

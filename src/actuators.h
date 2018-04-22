#pragma once

#include <Arduino.h>
#include <Time.h>
#include <ChainableLED.h>

namespace actuators {

void setup();
void loop();

void switch_lights();
void switch_filter_and_heater();
void switch_feed_status();

class Rele {
  public:
    Rele(byte _pin);
    void toggle();
    void on();
    void off();
    bool is_on() const;
    bool is_off() const;

  private:
    uint8_t pin;
    uint8_t value;
};

extern Rele fan;
extern Rele filter;
extern Rele heater;
extern Rele air_pump;

struct HourMinute {
    uint8_t hour;
    uint8_t minute;

    uint16_t minutes() const;
};

struct PhotoPeriod {
    HourMinute init, end;
    enum class Type { on, rise, fall } type;
    bool in_period(const HourMinute& hm) const;
};

struct LedStrip {
    PhotoPeriod periods[3];
};

struct LedStripStatus {
    uint8_t pwm;
    bool automatic;
};

LedStripStatus get_white();
LedStripStatus get_red();
LedStripStatus get_green();
LedStripStatus get_blue();
LedStripStatus get_uv();

struct Feeding {
    struct {
        uint8_t steps_per_revolution;
        uint8_t speed;
        uint8_t revolutions;
    } stepper;
    HourMinute breakfast, lunch, dinner;
    uint8_t fasting_day;
    bool active;
};
extern Feeding feeding;

} // namespace actuators

#include <Arduino.h>

#include <actuators.h>
#include <config.h>
#include <defaults.h>
#include <info.h>
#include <sensors.h>

#include <ChainableLED.h>
#include <OneButton.h>
#include <Stepper.h>
#include <Time.h>
#include <TimeAlarms.h>

namespace actuators {

static OneButton btn_lights{defaults::pines.btn_lights, true};
static OneButton btn_filter{defaults::pines.btn_filter, true};
static OneButton btn_feed{defaults::pines.btn_feed, true};

static uint8_t warning;

Rele air_pump = {defaults::pines.air_pump};
Rele heater = {defaults::pines.heater};
Rele filter = {defaults::pines.filter};

// TODO Change this to use ChainableLED!!
Rele fan = {defaults::pines.air_pump};

class LedStripImpl {
  public:
    LedStripImpl(LedStrip* led);

    void setup();
    void loop();

    void toggle();
    LedStripStatus get_status() { return status_; }

  private:
    LedStripStatus status_;
    LedStripStatus prev_status_;
    LedStrip* led_;
};

static const unsigned long time_between_led_updates = 300000; // 5 minutos
static unsigned long last_led_update = 0;

ChainableLED uwf{defaults::pines.p9813_clock_pin_1,
                 defaults::pines.p9813_data_pin_1, 1};
ChainableLED rgb{defaults::pines.p9813_clock_pin_2,
                 defaults::pines.p9813_data_pin_2, 1};

static LedStripImpl uv_led{&config::uv};
static LedStripImpl white_led{&config::white};

static LedStripImpl red_led{&config::red};
static LedStripImpl green_led{&config::green};
static LedStripImpl blue_led{&config::blue};

struct FeedImpl {
    FeedImpl(Feeding& f, Stepper& s);
    void setup();

    void feed();

    Feeding* feed_;
    Stepper* stepper_;
};
static Stepper stepper(config::feeding.stepper.steps_per_revolution,
                       defaults::pines.stepper1, defaults::pines.stepper2,
                       defaults::pines.stepper3, defaults::pines.stepper4);
static FeedImpl feed{config::feeding, stepper};

bool is_too_warm();
bool is_too_cold();

void setup()
{
    p(F("actuators::setup()"));

    uwf.init();
    rgb.init();

    feed.setup();

    air_pump.off();
    heater.on();
    filter.on();

    btn_filter.attachClick(actuators::switch_filter_and_heater);
    btn_lights.attachClick(actuators::switch_lights);
    btn_feed.attachClick(actuators::switch_feed_status);
}

void loop()
{
    // Tratando de evitar el parpadeo. Checkeamos cada time_between_led_updates
    if ((0 == last_led_update) ||
        (millis() > (last_led_update + time_between_led_updates))) {
        white_led.loop();
        uv_led.loop();

        uwf.setColorRGB(0, 0, white_led.get_status().pwm,
                        uv_led.get_status().pwm);

        red_led.loop();
        green_led.loop();
        blue_led.loop();

        rgb.setColorRGB(0, red_led.get_status().pwm, green_led.get_status().pwm,
                        blue_led.get_status().pwm);

        last_led_update = millis();
    }

    warning = LOW;
    if (is_too_warm()) {
        warning = HIGH;
        fan.on();
        air_pump.on();
    }
    else {
        // If not next condition we are in the middle of high and low
        if (is_too_cold()) {
            fan.off();
            air_pump.off();
        }
    }

    btn_feed.tick();
    btn_filter.tick();
    btn_lights.tick();

    if (filter.is_off() || heater.is_off())
        warning = HIGH;

    digitalWrite(defaults::pines.warning_led, warning);
}

void switch_lights() { white_led.toggle(); }
void switch_filter_and_heater()
{
    filter.toggle();
    heater.toggle();
}
void switch_feed_status()
{
    config::feeding.active = !config::feeding.active;
    config::update_eeprom();
}

bool is_too_warm()
{
    auto data = sensors::get_sensors_data();
    return data.ds18b20.internal >= config::high_temperature_alarm;
}

bool is_too_cold()
{
    auto data = sensors::get_sensors_data();
    return data.ds18b20.internal <= config::low_temperature_alarm;
}

LedStripStatus get_white() { return white_led.get_status(); }
LedStripStatus get_red() { return red_led.get_status(); }
LedStripStatus get_green() { return green_led.get_status(); }
LedStripStatus get_blue() { return blue_led.get_status(); }
LedStripStatus get_uv() { return uv_led.get_status(); }

/* Rele implementation */
Rele::Rele(byte _pin)
{
    pin = _pin;
    pinMode(pin, OUTPUT);
    off();
}

void Rele::off()
{
    value = HIGH;
    digitalWrite(pin, value);
}

void Rele::on()
{
    value = LOW;
    digitalWrite(pin, value);
}

void Rele::toggle()
{
    value = !value;
    digitalWrite(pin, value);
}

bool Rele::is_on() const { return value != HIGH; }

bool Rele::is_off() const { return !is_on(); }

/* LedStrip Implementation */
LedStripImpl::LedStripImpl(LedStrip* led_conf)
{
    led_ = led_conf;
    status_.pwm = 0;
    status_.automatic = true;
    prev_status_ = status_;
}

void LedStripImpl::setup() {}

void LedStripImpl::loop()
{
    if (status_.automatic) {
        status_.pwm = 0; // Off by default
        HourMinute ahora = {(uint8_t)hour(), (uint8_t)minute()};
        for (auto period : led_->periods) {
            if (period.in_period(ahora)) {
                switch (period.type) {
                case (actuators::PhotoPeriod::Type::fall):
                    status_.pwm =
                        255 - (uint8_t)map(ahora.minutes(),
                                           period.init.minutes(),
                                           period.end.minutes(), 0, 255);
                    break;

                case (actuators::PhotoPeriod::Type::rise):
                    status_.pwm =
                        (uint8_t)map(ahora.minutes(), period.init.minutes(),
                                     period.end.minutes(), 0, 255);
                    break;

                case (actuators::PhotoPeriod::Type::on):
                    status_.pwm = 255;
                    break;
                }
            }
        }
    }
}

void LedStripImpl::toggle()
{
    if (status_.automatic) {
        status_.pwm = 255;
    }
    status_.automatic = !status_.automatic;
}

uint16_t HourMinute::minutes() const
{
    uint16_t m = (hour * 60) + minute;
    return m;
}

bool PhotoPeriod::in_period(const HourMinute& hm) const
{
    uint16_t m = hm.minutes();
    return (init.minutes() < m && m < end.minutes());
}

void feed_now() { feed.feed(); }

FeedImpl::FeedImpl(Feeding& f, Stepper& s)
{
    feed_ = &f;
    stepper_ = &s;
}

void FeedImpl::feed()
{
    if (!feed_->active || feed_->fasting_day == weekday())
        return;

    stepper.step(-config::feeding.stepper.steps_per_revolution * 64 *
                 config::feeding.stepper.revolutions);
}

void FeedImpl::setup()
{
    stepper_->setSpeed(config::feeding.stepper.speed);
    Alarm.alarmRepeat(feed_->breakfast.hour, feed_->breakfast.minute, 0,
                      feed_now);
    Alarm.alarmRepeat(feed_->lunch.hour, feed_->lunch.minute, 0, feed_now);
    Alarm.alarmRepeat(feed_->dinner.hour, feed_->dinner.minute, 0, feed_now);
}
} // namespace actuators

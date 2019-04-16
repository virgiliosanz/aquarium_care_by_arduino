#include "actuators.h"
#include "config.h"
#include "defaults.h"
#include "info.h"
#include "sensors.h"
#include <TimeLib.h>
#include <WString.h>
#include <stdint.h>

namespace actuators {

static byte warning;

static bool co2_is_automatic;
static bool lights_are_automatic;

static bool lights_on[defaults::n_phases];
static byte n_lights_on = 0;

// Rele fan{defaults::pines.fan};
Rele filter{defaults::pines.filter};
Rele heater{defaults::pines.heater};
Rele pump{defaults::pines.pump};
Rele co2{defaults::pines.co2};
Rele phases[defaults::n_phases];

static void switch_lights();
static void switch_co2();
static void check_temperature();

void setup()
{
    p(F("actuators::setup()"));

    // p(F("Setting fan Off"));
    // fan.off();

    p(F("Setting filter On"));
    filter.on();

    p(F("Setting heater On"));
    heater.on();

    p(F("Setting pump Off"));
    pump.off();

    p(F("Setting co2 Off"));
    co2.off();
    // air_pump();

    p(F("Inicializando fases: %d"), defaults::n_phases);
    for (byte i = 0; i < defaults::n_phases; i++) {
        phases[i].set_pin(defaults::pines.phases[i]);
        phases[i].off();
    }

    co2_is_automatic = config::co2_automatic;
    lights_are_automatic = config::lights_automatic;
}

void loop()
{
    warning = LOW;

    if (lights_are_automatic)
        switch_lights();

    if (co2_is_automatic)
        switch_co2();

    check_temperature();

    // btn_filter.tick();
    // btn_pump.tick();

    warning = (warning || (filter.is_off() || heater.is_off())) ? HIGH : LOW;
    digitalWrite(defaults::pines.warn_led, warning);
}

// ------------------------------- Temp -------------------------------
void check_temperature()
{
    auto data = sensors::get_sensors_data();
    if (data.ds18b20.internal >= config::high_temperature_alarm) { // TOO WARM
        warning = HIGH;

        // p(F("Water is too WARM!!"));
        // fan.on();
        // air_pump.on();
    }
    /*
    else {
        // If not next condition we are in the middle of high and low
        if (data.ds18b20.internal <= config::low_temperature_alarm) {
            fan.off();
            air_pump.off();
        }
    }
    */
}

// ------------------------------- LIGHTS -------------------------------
void lights_behaviour(bool automatic)
{
    p(F("Lights automatic = %d"), automatic);
    lights_are_automatic = automatic;
}

bool lights_automatic() { return lights_are_automatic; }

static const defaults::PhotoPeriod* find_current_period()
{
    const defaults::PhotoPeriod* period = NULL;

    defaults::HourMinute now = {(byte)hour(), (byte)minute()};

    for (int8_t i = 0; i < defaults::n_periods; i++) {
        if (defaults::photo_periods[i].period.in_period(now)) {
            period = &defaults::photo_periods[i];
            break;
        }
    }

    return period;
}

static void switch_lights()
{
    // Ensure to signaling only when the status change

    // Search for current Period
    const defaults::PhotoPeriod* current_period = find_current_period();

    bool lights_to_switch[defaults::n_phases] = {false};
    if (NULL != current_period) {
        // Check if we have the number of lights needed
        if (current_period->phases_on == n_lights_on) {
            return;
        }

        // Build a random array of lights to switch
        byte current = 0;
        byte next = 0;

        // Look for a new phase randomly... as many as 1024 iterations
        for (unsigned i = 0; i < 1024; i++) {
            next = random(defaults::n_phases);

            if (!lights_to_switch[next]) {
                lights_to_switch[next] = true;
                current++;
            }

            if (current == current_period->phases_on)
                break;
        }

        // Swritch on new light phase groups
        switch_lights(lights_to_switch);
    }
    else {
        // If no current lights return
        if (0 == n_lights_on) {
            return;
        }
        else {
            // switch off the lights
            switch_lights(lights_to_switch);
        }
    }
}

void switch_lights(bool on[])
{
    char c_lights_on[defaults::n_phases + 1] = {0};

    n_lights_on = 0;
    for (byte i = 0; i < defaults::n_phases; i++) {
        lights_on[i] = on[i];
        if (on[i]) {
            n_lights_on++;
            c_lights_on[i] = '*';
            if (phases[i].is_off())
                phases[i].on();
        }
        else {
            c_lights_on[i] = '-';
            if (phases[i].is_on())
                phases[i].off();
        }
    }

    // p(F("%d Luces encendidas: %s"), n_lights_on, lights_on);
    p(F("%d Luces encendidas: %s"), n_lights_on, c_lights_on);
}

bool* phases_on() { return lights_on; }

// ------------------------------- CO2 -------------------------------
void co2_behaviour(bool automatic)
{
    p(F("CO2 automatic is: %d"), automatic);
    co2_is_automatic = automatic;
}
bool co2_automatic() { return co2_is_automatic; }

void switch_co2(bool on)
{
    if (on && co2.is_off()) {
        co2.on();
    }

    if (!on && co2.is_on()) {
        co2.off();
    }
}

void switch_co2()
{
    defaults::HourMinute now = {(byte)hour(), (byte)minute()};
    switch_co2(defaults::co2_period.in_period(now));
}

// ------------------------------- Other Reles -------------------------------
void switch_filter_and_heater()
{
    filter.toggle();
    heater.toggle();
}
void switch_pump() { pump.toggle(); }

// ------------------------------- Rele Class -------------------------------
Rele::Rele(byte _pin) : pin(_pin), value(-1) { pinMode(pin, OUTPUT); }
Rele::Rele() : pin(0), value(-1) {}

void Rele::set_pin(byte _pin)
{
    pin = _pin;
    value = -1;
    pinMode(pin, OUTPUT);
}

void Rele::off()
{
    if (0 == pin)
        return;

    if (HIGH == value)
        return;

    value = HIGH;
    digitalWrite(pin, value);

    p(F("off() -> Rele (%d) - %d"), pin, value);
}

void Rele::on()
{
    if (0 == pin)
        return;

    if (LOW == value)
        return;

    value = LOW;
    digitalWrite(pin, value);

    p(F("on() -> Rele (%d) - %d"), pin, value);
}

void Rele::toggle()
{
    if (0 == pin)
        return;
    value = !value;
    digitalWrite(pin, value);

    p(F("togle() -> Rele (%d) - %d"), pin, value);
}

bool Rele::is_on() const { return value != HIGH; }
bool Rele::is_off() const { return !is_on(); }

} // namespace actuators

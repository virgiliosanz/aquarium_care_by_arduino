#include <actuators.h>
#include <config.h>
#include <defaults.h>
#include <info.h>
#include <OneButton.h>
#include <sensors.h>
#include <TimeLib.h>
#include <WString.h>

namespace actuators {

static OneButton btn_pump { defaults::pines.btn_pump, true };
static OneButton btn_filter { defaults::pines.btn_filter, true };

static byte warning;

static bool co2_is_automatic;
static bool lights_are_automatic;

Rele fan { defaults::pines.fan };
Rele filter { defaults::pines.filter };
Rele heater { defaults::pines.heater };
Rele pump { defaults::pines.pump };
Rele co2 { defaults::pines.co2 };
Rele phases[defaults::n_phases];

static bool is_too_warm();
static bool is_too_cold();
static void switch_lights();
static void switch_co2();
static void check_temp();

void setup() {
	p(F("actuators::setup()"));

	fan.off();
	filter.on();
	heater.on();
	pump.off();
	co2.off();
	// air_pump();

	for (byte i = 0; i < defaults::n_phases; i++) {
		phases[i].set_pin(defaults::pines.phases[i]);
		phases[i].off();
	}

	co2_is_automatic = config::co2_automatic;
	lights_are_automatic = config::lights_automatic;

	btn_pump.attachClick(switch_pump);
	btn_filter.attachClick(switch_filter_and_heater);

	randomSeed((unsigned long int) analogRead(0));
}

void loop() {
	if (lights_are_automatic)
		switch_lights();

	if (co2_is_automatic)
		switch_co2();

	check_temp();

	btn_filter.tick();
	btn_pump.tick();

	if (filter.is_off() || heater.is_off())
		warning = HIGH;

	digitalWrite(defaults::pines.warn_led, warning);
}

// ------------------------------- Temp -------------------------------
bool is_too_warm() {
	auto data = sensors::get_sensors_data();
	return data.ds18b20.internal >= config::high_temperature_alarm;
}

bool is_too_cold() {
	auto data = sensors::get_sensors_data();
	return data.ds18b20.internal <= config::low_temperature_alarm;
}

void check_temp() {
	warning = LOW;
	if (is_too_warm()) {
		warning = HIGH;
		fan.on();
		// air_pump.on();
	} else {
		// If not next condition we are in the middle of high and low
		if (is_too_cold()) {
			fan.off();
			// air_pump.off();
		}
	}
}

// ------------------------------- LIGHTS -------------------------------
void lights_behaviour(bool automatic) {
	lights_are_automatic = automatic;
}

static void switch_lights() {
	// Buscar en que periodo estamos y ver las fases
	static defaults::HourMinute now = { (byte) hour(), (byte) minute() };
	byte current_period = 0;
	for (byte i = 0; i < defaults::n_phases; i++) {
		if (defaults::photo_periods[i].period.in_period(now)) {
			current_period = i;
			break;
		}
	}

	// Crear un array random del número de fases a encender
	bool lights_to_switch[defaults::n_phases] = { false };
	if (current_period != 0) {
		byte current = 0;
		byte next = 0;
		// No more than 1000 iterations...
		for (byte i = 0; i < 1000; i++) {
			next = random(defaults::n_phases);
			if (!lights_to_switch[next]) {
				lights_to_switch[next] = true;
				current++;
			}
			if (current == defaults::photo_periods[current_period].phases_on)
				break;
		}
	}
	switch_lights(lights_to_switch);
}

static bool lights_on[defaults::n_phases];
void switch_lights(bool on[]) {
	for (byte i = 0; i < defaults::n_phases; i++) {
		lights_on[i] = on[i];

		if (on[i]) {
			phases[i].on();
		} else {
			phases[i].off();
		}
	}
}

bool* phases_on() {
	return lights_on;
}

// ------------------------------- CO2 -------------------------------
void co2_behaviour(bool automatic) {
	co2_is_automatic = automatic;
}
void switch_co2(bool on) {
	// TODO: Implement switch_co2phases_on();
}
void switch_co2() {
	// TODO: Implement switch_co2

	// Buscar en que periodo estamos y ver las fases
	//static defaults::HourMinute now = {(byte)hour(), (byte)minute()};
}

// ------------------------------- Other Reles -------------------------------
void switch_filter_and_heater() {
	filter.toggle();
	heater.toggle();
}
void switch_pump() {
	pump.toggle();
}

// ------------------------------- Rele Class -------------------------------
Rele::Rele(byte _pin) :
		pin(_pin) {
}
Rele::Rele() :
		pin(0) {
}

void Rele::set_pin(byte _pin) {
	pin = _pin;
	pinMode(pin, OUTPUT);
}

void Rele::off() {
	if (0 == pin || HIGH == value)
		return;

	value = HIGH;
	digitalWrite(pin, value);
}

void Rele::on() {
	if (0 == pin || LOW == value)
		return;

	value = LOW;
	digitalWrite(pin, value);
}

void Rele::toggle() {
	if (0 == pin)
		return;
	value = !value;
	digitalWrite(pin, value);
}

bool Rele::is_on() const {
	return value != HIGH;
}
bool Rele::is_off() const {
	return !is_on();
}

} // namespace actuators

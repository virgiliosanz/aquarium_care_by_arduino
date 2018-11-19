#include <config.h>
#include <defaults.h>
#include <DS1307RTC.h>
#include <EEPROM.h>
#include <HardwareSerial.h>
#include <info.h>
#include <string.h>
#include <TimeLib.h>
#include <WString.h>

namespace config {

static const char version[] = "0.5.0";
static const int version_addr = 0;

// Struct to save & read from eeprom
float high_temperature_alarm = defaults::high_temperature_alarm;
static const int high_temperature_alarm_addr = sizeof(version) + 1;

float low_temperature_alarm = defaults::low_temperature_alarm;
static const int low_temperature_alarm_addr = high_temperature_alarm_addr
		+ sizeof(high_temperature_alarm) + 1;

int tds_address = low_temperature_alarm_addr + sizeof(low_temperature_alarm) + 1;

bool co2_automatic = true;
bool lights_automatic = true;

//struct PHCalibrationValue ph_calibration;
//static const int ph_calibration_addr = low_temperature_alarm_addr
//		+ sizeof(low_temperature_alarm) + 1;

// TODO: Calcular tamaño para el resto de temas a guardar: Fotoperiodo, ¿calibración de TDS?

void load_defaults() {
	high_temperature_alarm = defaults::high_temperature_alarm;
	low_temperature_alarm = defaults::low_temperature_alarm;
	co2_automatic = defaults::co2_automatic;
	lights_automatic = defaults::lights_automatic;
}

bool load_from_eeprom() {
	char read_ver[6];
	EEPROM.get(version_addr, read_ver);
	if (0 != strcmp(read_ver, version))
		return false;

	EEPROM.get(high_temperature_alarm_addr, high_temperature_alarm);
	EEPROM.get(low_temperature_alarm_addr, low_temperature_alarm);
//	EEPROM.get(ph_calibration_addr, ph_calibration);

	return true;
}

void update_eeprom() {
	EEPROM.put(version_addr, version);
	EEPROM.put(high_temperature_alarm_addr, high_temperature_alarm);
	EEPROM.put(low_temperature_alarm_addr, low_temperature_alarm);
//	EEPROM.put(ph_calibration_addr, ph_calibration);
}

void setup() {
	Serial.begin(defaults::serial_baud);

	setSyncProvider(RTC.get);
	if (timeStatus() != timeSet) {
		p(F("Unable to sync with the RTC"));
	}

	p(F("Loading setup, ver: %s"), version);

	p(F("Loading from EEPROM"));

	if (!load_from_eeprom()) {
		p(F("No data in EEPROM - saving defaults"));
		load_defaults();
		update_eeprom(); // Si no hay datos guardar los guardamos por defecto.
	}
}
}

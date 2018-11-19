#include <config.h>
#include <defaults.h>
#include <DallasTemperature.h>
#include <GravityTDS.h>
#include <info.h>
#include <OneWire.h>
#include <sensors.h>
#include <WString.h>

namespace sensors {

static OneWire oneWire(defaults::ds18b20_conf.pin);

static DallasTemperature sensors(&oneWire);
const byte insideThermometer_id = 0;
const byte outsideThermometer_id = 1;
static DeviceAddress insideThermometer, outsideThermometer;

static GravityTDS tds;

static SensorsData sensors_data { { 0 } };

void setup() {
	p(F("Sensors::setup()"));

	// Temperature: ds128b20
	sensors_data.ds18b20.sensor_ok = true;
	if (!sensors.getAddress(insideThermometer, insideThermometer_id)) {
		p(F("Unable to find address for Device 0"));
		sensors_data.ds18b20.sensor_ok = false;
	}
	if (!sensors.getAddress(outsideThermometer, outsideThermometer_id)) {
		p(F("Unable to find address for Device 1"));
		sensors_data.ds18b20.sensor_ok = false;
	}
	sensors_data.ds18b20.time_between_reads =
			defaults::ds18b20_conf.time_between_reads;

	// Tds_ GravityTDS
	tds.setPin((int) defaults::tds_conf.pin);
	tds.setAref(defaults::tds_conf.adc_ref);
	tds.setAdcRange(defaults::tds_conf.adc_range);
	tds.setKvalueAddress(config::tds_address);
	tds.begin();

	sensors_data.tds.time_between_reads = defaults::tds_conf.time_between_reads;
	sensors_data.tds.sensor_ok = true;
}

void loop() {
#define SHOULD_READ(last_read, time_between_reads)                             \
    (((0 == last_read)) || (millis() > ((last_read) + (time_between_reads))))

	//  DS18B20
	if (SHOULD_READ(sensors_data.ds18b20.last_read,
			sensors_data.ds18b20.time_between_reads)) {
		sensors_data.ds18b20.last_read = millis();
		sensors.requestTemperatures();
//		sensors_data.ds18b20.external = sensors.getTempCByIndex(outsideThermometer_id);
		sensors_data.ds18b20.external = sensors.getTempCByIndex(
				insideThermometer_id);
		sensors_data.ds18b20.internal = sensors.getTempCByIndex(
				insideThermometer_id);

		p(F("ds18b20 (temperature) %d %d"), (int) sensors_data.ds18b20.internal,
				(int) sensors_data.ds18b20.external);
	}

	// TDS
	if (SHOULD_READ(sensors_data.tds.last_read,
			sensors_data.tds.time_between_reads)) {
		sensors_data.tds.last_read = millis();
		tds.setTemperature(sensors_data.ds18b20.internal);
		tds.update();
		sensors_data.tds.tds = tds.getTdsValue();
		p(F("TDS %d"), sensors_data.tds.tds);

	}
#undef SHOULD_READ
}

SensorsData& get_sensors_data() {
	return sensors_data;
}

} // namespace sensors

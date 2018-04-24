#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <AnalogPHMeter.h>
#include <Arduino.h>
#include <DHT.h>
#include <DallasTemperature.h>
#include <GravityTDS.h>
#include <OneWire.h>
#include <SPI.h>

#include <config.h>
#include <info.h>
#include <sensors.h>

namespace sensors {

static Adafruit_BMP280 bmp280;
static DHT dht(defaults::dht_conf.pin, DHT11);
static OneWire oneWire(defaults::ds18b20_conf.pin);

static DallasTemperature sensors(&oneWire);
const uint8_t insideThermometer_id = 0;
const uint8_t outsideThermometer_id = 1;
static DeviceAddress insideThermometer, outsideThermometer;

static AnalogPHMeter ph_meter(defaults::ph_conf.pin);
static GravityTDS tds;

static SensorsData sensors_data{0};

void setup()
{
    p(F("Sensors::setup()"));
    sensors_data.bmp280t.sensor_ok =
        bmp280.begin(defaults::bmp280_conf.address);
    dht.begin();
    sensors.begin();

    sensors_data.ds18b20.sensor_ok = true;
    if (!sensors.getAddress(insideThermometer, insideThermometer_id)) {
        p(F("Unable to find address for Device 0"));
        sensors_data.ds18b20.sensor_ok = false;
    }
    if (!sensors.getAddress(outsideThermometer, outsideThermometer_id)) {
        p(F("Unable to find address for Device 1"));
        sensors_data.ds18b20.sensor_ok = false;
    }

    ph_meter.initialize(config::ph_calibration);

    tds.setPin(defaults::tds_conf.pin);
    tds.setAref(5.0); // reference voltage on ADC, default 5.0V on Arduino UNO
    tds.setAdcRange(1024); // 1024 for 10bit ADC;4096 for 12bit ADC
    tds.begin();           // initialization
}

void loop()
{
#define SHOULD_READ(last_read, time_between_reads)                             \
    (((0 == last_read)) || (millis() > ((last_read) + (time_between_reads))))

    // BMP280
    if (SHOULD_READ(sensors_data.bmp280t.last_read,
                    defaults::bmp280_conf.time_between_reads)) {

        sensors_data.bmp280t.last_read = millis();
        sensors_data.bmp280t.pressure = bmp280.readPressure() / 100.0f;
        sensors_data.bmp280t.temperature = bmp280.readTemperature();
        sensors_data.bmp280t.altitude = bmp280.readAltitude(
            defaults::bmp280_conf.altitude_at_your_locality);

        p(F("bmp280 (pressure): %d %d %d "), (int)sensors_data.bmp280t.pressure,
          (int)sensors_data.bmp280t.temperature,
          (int)sensors_data.bmp280t.altitude);
    }

    // DHT11
    if (SHOULD_READ(sensors_data.dht11.last_read,
                    defaults::dht_conf.time_between_reads)) {

        sensors_data.dht11.last_read = millis();
        sensors_data.dht11.sensor_ok = dht.read(false);
        sensors_data.dht11.humidity = dht.readHumidity();
        sensors_data.dht11.temperature = dht.readTemperature();

        p(F("dht11 (humidity): %d %d "), (int)sensors_data.dht11.humidity,
          (int)sensors_data.dht11.temperature);
    }

    //  DS18B20
    if (SHOULD_READ(sensors_data.ds18b20.last_read,
                    defaults::ds18b20_conf.time_between_reads)) {

        sensors_data.ds18b20.last_read = millis();
        sensors.requestTemperatures();
        sensors_data.ds18b20.external =
            sensors.getTempCByIndex(insideThermometer_id);
        sensors_data.ds18b20.internal =
            sensors.getTempCByIndex(outsideThermometer_id);

        p(F("ds18b20 (temperature) %d %d"), (int)sensors_data.ds18b20.internal,
          (int)sensors_data.ds18b20.external);
    }

    // PH
    if (SHOULD_READ(sensors_data.ph.last_read,
                    defaults::ph_conf.time_between_reads)) {
        sensors_data.ph.last_read = millis();
        sensors_data.ph.ph = ph_meter.getpH();
        p(F("PH %4.1f"), sensors_data.ph);
    }

    // TDS
    if (SHOULD_READ(sensors_data.tds.last_read,
                    defaults::tds_conf.time_between_reads)) {
        sensors_data.tds.last_read = millis();
        tds.setTemperature(sensors_data.ds18b20.internal);
        tds.update();
        sensors_data.tds.tds = tds.getTdsValue();
        p(F("TDS %4.1f"), sensors_data.tds);
    }
#undef SHOULD_READ
}

SensorsData& get_sensors_data() { return sensors_data; }

namespace PH {
void calibrate_ph4() { ph_meter.calibrationLow(4.0f); }
void calibrate_ph7() { ph_meter.calibrationMid(7.0f); }
void calibrate_ph10() { ph_meter.calibrationHigh(10.0f); }

float get_ph() { return ph_meter.getpH(); }

} // namespace PH

} // namespace sensors

#include <EEPROM.h>
#include <config.h>
#include <info.h>

namespace config
{

static const char version[] = "0.4.0";
static const int version_addr = 0;

// Struct to save & read from eeprom
float high_temperature_alarm = defaults::high_temperature_alarm;
static const int high_temperature_alarm_addr = sizeof(version) + 1;

float low_temperature_alarm = defaults::low_temperature_alarm;
static const int low_temperature_alarm_addr =
    high_temperature_alarm_addr + sizeof(high_temperature_alarm) + 1;

actuators::LedStrip white = defaults::white;
static const int white_addr = low_temperature_alarm_addr + sizeof(low_temperature_alarm) + 1;

actuators::LedStrip red = defaults::red;
static const int red_addr = white_addr + sizeof(white) + 1;

actuators::LedStrip green = defaults::green;
static const int green_addr = red_addr + sizeof(red) + 1;

actuators::LedStrip blue = defaults::blue;
static const int blue_addr = green_addr + sizeof(green) + 1;

actuators::LedStrip uv = defaults::uv;
static const int uv_addr = blue_addr + sizeof(blue) + 1;

actuators::Feeding feeding = defaults::feeding;
static const int feeding_addr = uv_addr + sizeof(uv) + 1;

struct PHCalibrationValue ph_calibration;
static const int ph_calibration_addr = feeding_addr + sizeof(feeding) + 1;

// TODO Calibrate TDS!


static LiquidCrystal_I2C lcd_i2c(defaults::lcd_addr, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
LCD *lcd = &lcd_i2c; // Usamos la cláse genérica

void load_defaults()
{
    high_temperature_alarm = defaults::high_temperature_alarm;
    low_temperature_alarm = defaults::low_temperature_alarm;
    white = defaults::white;
    red = defaults::red;
    green = defaults::green;
    blue = defaults::blue;
    uv = defaults::uv;
    feeding = defaults::feeding;
}

bool load_from_eeprom()
{
    char read_ver[6];
    EEPROM.get(version_addr, read_ver);
    if (0 != strcmp(read_ver, version))
        return false;

    EEPROM.get(high_temperature_alarm_addr, high_temperature_alarm);
    EEPROM.get(low_temperature_alarm_addr, low_temperature_alarm);
    EEPROM.get(white_addr, white);
    EEPROM.get(red_addr, red);
    EEPROM.get(green_addr, green);
    EEPROM.get(blue_addr, blue);
    EEPROM.get(uv_addr, uv);
    EEPROM.get(feeding_addr, feeding);
    EEPROM.get(ph_calibration_addr, ph_calibration);

    return true;
}

void update_eeprom()
{
    EEPROM.put(version_addr, version);
    EEPROM.put(high_temperature_alarm_addr, high_temperature_alarm);
    EEPROM.put(low_temperature_alarm_addr, low_temperature_alarm);
    EEPROM.put(white_addr, white);
    EEPROM.put(red_addr, red);
    EEPROM.put(green_addr, green);
    EEPROM.put(blue_addr, blue);
    EEPROM.put(uv_addr, uv);
    EEPROM.put(feeding_addr, feeding);
    EEPROM.put(ph_calibration_addr, ph_calibration);
}

void setup()
{
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

    lcd->begin(defaults::screen_cols, defaults::screen_rows);
}
}

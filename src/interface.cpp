#include "interface.h"
#include "actuators.h"
#include "info.h"
#include "sensors.h"
#include <ClickEncoder.h>
#include <DS3232RTC.h>
#include <LiquidCrystal_I2C.h>
#include <Printable.h>
#include <TimerOne.h>
#include <WString.h>
#include <avr/pgmspace.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

namespace interface {

// Malpartida library
// static LiquidCrystal_I2C lcd_i2c(
//    defaults::lcd_addr, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Brabander library
static LiquidCrystal_I2C lcd(defaults::lcd_addr, defaults::screen_cols,
                             defaults::screen_rows);

// Initializing interface object
static Interface interface {};
Interface *Screen::interface_ = &interface;

#define INTERFACE(method)                                                      \
    	void method() { interface.method(); }

INTERFACE(setup)
INTERFACE(loop)
INTERFACE(ok)
INTERFACE(left)
INTERFACE(right)

#undef INTERFACE

/* Interface Implementation */
Interface::Interface()
    : encoder_(defaults::pines.encoder_dt, defaults::pines.encoder_clk,
               defaults::pines.encoder_sw) {
    at_lcd_[defaults::screen_size] = {' '};
    to_lcd_[defaults::screen_size] = {' '};
    last_activity_millis_ = 0;
    lcd_is_on_ = false;
    current_ = nullptr;
}

void timerIsr() {
    interface.encoder_.service();
}

void Interface::setup() {
    last_activity_millis_ = millis();

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.on();

    current_ = Home::instance();
    current_->enter();

    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr);
}

void Interface::loop() {

    int16_t val = encoder_.getValue();

    if (val > 0) {
        right();
    } else if (val < 0) {
        left();
    }

    ClickEncoder::Button b = encoder_.getButton();

    if (b == ClickEncoder::Clicked) {
        ok();
    }

    // If inactive switch off the screen
    if (lcd_is_on_) {
        if (inactive_timeout()) {
            lcd.noBacklight();
            lcd_is_on_ = false;
        } else {
            current_->update_screen();
            update_lcd();
        }
    }
}

#define INTERFAZ_ACTION(method)                                                \
    	void Interface::method()                                                   \
    	{                                                                          \
        	if (!lcd_is_on_) {                                                     \
            		lcd_is_on_ = true;                                                 \
            		lcd.backlight();                                                   \
        	}                                                                      \
        	last_activity_millis_ = millis();                                      \
        	current_->method();                                                    \
    	}

INTERFAZ_ACTION(ok)
INTERFAZ_ACTION(left)
INTERFAZ_ACTION(right)

#undef INTERFAZ_ACTION

void Interface::move_to_screen(Screen *s) {
    current_->leave();
    current_ = s;
    current_->enter();
}

void Interface::update_lcd() {
    byte r, c, i;

    for (c = 0; c < defaults::screen_cols; c++) {
        for (r = 0; r < defaults::screen_rows; r++) {
            i = (r * defaults::screen_cols) + c;

            if (to_lcd_[i] != at_lcd_[i]) {
                lcd.setCursor(c, r);
                lcd.print(to_lcd_[i]);
                at_lcd_[i] = to_lcd_[i];
            }
        }
    }
}

void Interface::to_lcd(const __FlashStringHelper *tpl) {
    PGM_P p = reinterpret_cast<PGM_P>(tpl);
    size_t i = 0;

    while (1) {
        unsigned char c = pgm_read_byte(p++);

        if (c == 0) {
            break;
        }

        to_lcd_[i] = (char)c;
        i++;
    }
}

void Interface::to_lcd(const char *tpl) {
    strncpy(to_lcd_, tpl, defaults::screen_size);
}

/* width is signed value, negative for left adjustment.  */
void Interface::to_lcd(const float d, const byte position_at_lcd,
                       const int8_t room_width, const byte precission) {
    char a[defaults::screen_cols + 1];
    dtostrf((double)d, room_width, precission, a);
    to_lcd(a, position_at_lcd, room_width);
}

/* If room_width < 0 align right */
void Interface::to_lcd(const int i, const byte position_at_lcd,
                       const int8_t room_width) {
    char a[defaults::screen_cols + 1];
    itoa(i, a, 10);
    to_lcd(a, position_at_lcd, room_width, '0');
}

/* If room_width < 0 align right */
void Interface::to_lcd(const char *src, const byte position_at_lcd,
                       const int8_t room_width, char fill_with) {
    byte len_s = (byte)strlen(src);
    byte width = (byte)abs(room_width);
    char *dest = &to_lcd_[position_at_lcd];
    byte i;

    // align right
    if (0 > room_width) {
        for (i = 0; i < (width - len_s); i++) {
            *dest = fill_with;
            dest++;
        }

        while (i < width) {
            *dest = *src;
            dest++;
            src++;
            i++;
        }
    } else { // algn left
        for (i = 0; i < len_s; i++) {
            *dest = *src;
            dest++;
            src++;
        }

        while (i < width) {
            *dest = fill_with;
            dest++;
            i++;
        }
    }
}

void Interface::cursor_at(byte c, byte r) {
    lcd.cursor_on();
    lcd.setCursor(c, r);
    lcd.blink_on();
}

void Interface::cursor_off() {
    lcd.blink_off();
    lcd.cursor_off();
}

bool Interface::inactive_timeout() {
    return (millis() > (last_activity_millis_ + defaults::max_inactive_millis));
}

// -----------------------------------------------------------------
/* ConcreteStates (Screens) implementation */
Home *Home::instance() {
    static Home screen;
    return &screen;
}

void Home::left() {
    interface_->move_to_screen(WaterChange::instance());
}
void Home::right() {
    interface_->move_to_screen(Info::instance());
}
void Home::ok() {
    interface_->move_to_screen(HomeEdit::instance());
}

void Home::update_screen() {
    //                    0123456789012345
    interface_->to_lcd(F("ddd,dd/mmm HH:MM"
                         "01234567  234 25"));
    TimeElements tm;
    RTC.read(tm);
    interface_->to_lcd(dayShortStr(tm.Wday), 0, 3);
    interface_->to_lcd(tm.Day, 4, -2);
    interface_->to_lcd(monthShortStr(tm.Month), 7, 3);
    interface_->to_lcd(tm.Hour, 11, -2);
    interface_->to_lcd(tm.Minute, 14, -2);

    bool *phases_on = actuators::phases_on();
    char lights[] = "12345678";

    // Set off light phases
    for (int i = 0; i < defaults::n_phases; i++) {
        if (!phases_on[i]) {
            lights[i] = '*';
        }
    }

    interface_->to_lcd(lights, 16, defaults::n_phases);

    sensors::SensorsData data = sensors::get_sensors_data();
    interface_->to_lcd(data.tds.tds, 16 + 10, -3);
    interface_->to_lcd((int)data.ds18b20.internal, 16 + 14, -2);
}

HomeEdit *HomeEdit::instance() {
    static HomeEdit screen;
    return &screen;
}

void HomeEdit::edit_day() {
    editing_ = Editing::Day;
    interface_->cursor_at(2, 0);
}

void HomeEdit::edit_month() {
    editing_ = Editing::Month;
    interface_->cursor_at(5, 0);
}

void HomeEdit::edit_year() {
    editing_ = Editing::Year;
    interface_->cursor_at(9, 0);
}

void HomeEdit::edit_hour() {
    editing_ = Editing::Hour;
    interface_->cursor_at(5, 1);
}

void HomeEdit::edit_minute() {
    editing_ = Editing::Minute;
    interface_->cursor_at(8, 1);
}

void HomeEdit::enter() {
    RTC.read(tm_);
    edit_day();
}

void HomeEdit::leave() {
    if (!RTC.write(tm_)) {
        p(F("Time set ERROR!"));
        interface_->to_lcd(F("Time set ERROR!"));
    }

    interface_->cursor_off();
}

void HomeEdit::right() {
    switch (editing_) {
    case (Editing::Day):
        tm_.Day = tm_.Day == 31 ? 1 : tm_.Day + 1;
        break;

    case (Editing::Month):
        tm_.Month = tm_.Month == 12 ? 1 : tm_.Month + 1;
        break;

    case (Editing::Year):
        tm_.Year = tm_.Year == 255 ? 0 : tm_.Year + 1;
        break;

    case (Editing::Hour):
        tm_.Hour = tm_.Hour == 23 ? 0 : tm_.Hour + 1;
        break;

    case (Editing::Minute):
        tm_.Minute = tm_.Minute == 59 ? 0 : tm_.Minute + 1;
        break;
    }
}

void HomeEdit::left() {
    switch (editing_) {
    case (Editing::Day):
        tm_.Day = tm_.Day == 1 ? 31 : tm_.Day - 1;
        break;

    case (Editing::Month):
        tm_.Month = tm_.Month == 1 ? 12 : tm_.Month - 1;
        break;

    case (Editing::Year):
        tm_.Year = tm_.Year == 0 ? 255 : tm_.Year - 1;
        break;

    case (Editing::Hour):
        tm_.Hour = tm_.Hour == 0 ? 23 : tm_.Hour - 1;
        break;

    case (Editing::Minute):
        tm_.Minute = tm_.Minute == 0 ? 59 : tm_.Minute - 1;
        break;
    }
}

void HomeEdit::ok() {
    switch (editing_) {
    case (Editing::Day):
        edit_month();
        break;

    case (Editing::Month):
        edit_year();
        break;

    case (Editing::Year):
        edit_hour();
        break;

    case (Editing::Hour):
        edit_minute();
        break;

    case (Editing::Minute):
        interface_->move_to_screen(Home::instance());
        break;
    }
}

void HomeEdit::update_screen() {
    //                    0123456789012345
    interface_->to_lcd(F("  01/Feb/1972   "
                         "     15:20      "));
    interface_->to_lcd((int)tm_.Day, 2, -2);
    interface_->to_lcd(monthShortStr(tm_.Month), 5, 3);
    interface_->to_lcd(tmYearToCalendar(tm_.Year), 9, -4);
    interface_->to_lcd((int)tm_.Hour, 16 + 5, -2);
    interface_->to_lcd((int)tm_.Minute, 16 + 8, -2);
}

// -----------------------------------------------------------------
Info *Info::instance() {
    static Info screen;
    return &screen;
}

void Info::left() {
    interface_->move_to_screen(Home::instance());
}
void Info::right() {
    interface_->move_to_screen(Check::instance());
}

void Info::update_screen() {
    sensors::SensorsData data = sensors::get_sensors_data();

    //                    0123456789012345
    interface_->to_lcd(F("T  45   Mem 9999"
                         "CO2 A   Lights A"));

    interface_->to_lcd(data.ds18b20.external, 3, -2, 0);
    interface_->to_lcd(freeRam(), 12, -4);
    interface_->to_lcd(actuators::co2_automatic() ? "A" : "M", 16 + 4, 1);
    interface_->to_lcd(actuators::lights_automatic() ? "A" : "M", 16 + 15, 1);
}

// ----------------------------------------------------------------

// ----------------------------------------------------------------

Check *Check::instance() {
    static Check screen;
    return &screen;
}

void Check::left() {
    interface_->move_to_screen(Info::instance());
}
void Check::right() {
    interface_->move_to_screen(WaterChange::instance());
}
void Check::ok() {
    interface_->move_to_screen(CheckEdit::instance());
}
void Check::update_screen() {
    //                    0123456789012345
    interface_->to_lcd(F(" Click to Check "
                         "      System    "));
}

CheckEdit *CheckEdit::instance() {
    static CheckEdit screen;
    return &screen;
}

void CheckEdit::enter() {
    checking_ = Checking::Phase;
    current_phase = 0;
    actuators::lights_behaviour(false);
}

void CheckEdit::leave() {
    actuators::lights_behaviour(true);
}

void CheckEdit::ok() {
    bool *encendidas;

    switch (checking_) {
    case (Checking::Phase):

        encendidas = actuators::phases_on();

        for (byte i = 0; i < defaults::n_phases; i++) {
            encendidas[i] = (i == current_phase) ? true : false;
        }

        actuators::switch_lights(encendidas);

        current_phase++;

        if (current_phase == defaults::n_phases) {
            checking_ = Checking::AllPhases;
        }

        break;


    case (Checking::AllPhases):
        encendidas = actuators::phases_on();

        for (byte i = 0; i < defaults::n_phases; i++) {
            encendidas[i] = true;
        }

        actuators::switch_lights(encendidas);
        checking_ = Checking::TDS;
        break;

    case (Checking::TDS):
        checking_ = Checking::Temperatures;
        break;

    case (Checking::Temperatures):
        interface_->move_to_screen(WaterChange::instance());
        break;
    }
}

void CheckEdit::update_screen() {
    //                    0123456789012345
    interface_->to_lcd(F("    Checking    "
                         "		  "));

    sensors::SensorsData sdata = sensors::get_sensors_data();

    switch (checking_) {
    case (Checking::Phase):
        interface_->to_lcd("Phase", 16 + 4, 5);
        interface_->to_lcd(current_phase + 1, 16 + 10, -3);
        break;

    case (Checking::AllPhases):
        interface_->to_lcd("All Phases", 16 + 4, 5);
        break;

    case (Checking::TDS):
        interface_->to_lcd("TDS", 16, 7);
        interface_->to_lcd(sdata.tds.sensor_ok ? "Ok " : "Err", 16 + 4, -4);
        interface_->to_lcd(sdata.tds.tds, 16 + 12, -4);
        break;

    case (Checking::Temperatures):
        interface_->to_lcd("Temp", 16, 7);
        interface_->to_lcd(sdata.ds18b20.sensor_ok ? "Ok " : "Err", 16 + 5, -4);
        interface_->to_lcd(sdata.ds18b20.internal, 16 + 12, -4, 1);

        break;
    }
}

// Water Change
WaterChange *WaterChange::instance() {
    static WaterChange screen;
    return &screen;
}

void WaterChange::ok() {
    interface_->move_to_screen(WaterChangeEdit::instance());
}
void WaterChange::left() {
    interface_->move_to_screen(Check::instance());
}
void WaterChange::right() {
    interface_->move_to_screen(Home::instance());
}
void WaterChange::update_screen() {
    //                    0123456789012345
    interface_->to_lcd(F("  Water Change  "
                         " Click to start "));
}

WaterChangeEdit *WaterChangeEdit::instance() {
    static WaterChangeEdit screen;
    return &screen;
}

// Left or right back move to WaterChange main screen
void WaterChangeEdit::left() {
    p(F("WaterChangeEdit:left()"));
    interface_->move_to_screen(WaterChange::instance());
}
void WaterChangeEdit::right() {
    p(F("WaterChangeEdit:right()"));
    interface_->move_to_screen(WaterChange::instance());
}

// When entering: stop filter, heater & CO2
void WaterChangeEdit::enter() {
    p(F("Entering WaterEChangEdit"));
    pump_ = false;
    actuators::switch_filter_and_heater();
    actuators::co2_behaviour(false);
    actuators::switch_co2(false);
}

// When leaving: stop pupm, start filter, heater and CO2
void WaterChangeEdit::leave() {
    p(F("Leaving WaterChangeEdit"));
    actuators::switch_filter_and_heater();
    actuators::co2_behaviour(true);

    if (pump_) {
        actuators::switch_pump();
        pump_ = false;
    }
}

// Click -> water change switch
void WaterChangeEdit::ok() {
    p(F("WaterChangeEdit:ok()"));
    pump_ = !pump_;
    actuators::switch_pump();
}

void WaterChangeEdit::update_screen() {
    //                       0123456789012345
    if (pump_) {
        interface_->to_lcd(F("Pump:  ON       "
                             "Right to go back"));
    } else {
        interface_->to_lcd(F("Pump:  OFF      "
                             "Left to go back "));
    }
}
} // namespace interface

#pragma once

#include <Arduino.h>
#include <defaults.h>
#include <Encoder.h>
#include <OneButton.h>
#include <stdint.h>
#include <TimeLib.h>

namespace interface {

void setup();
void loop();

// control interfaz programatically
void left();
void right();
void ok();

/*
 State Pattern: https://en.wikipedia.org/wiki/State_pattern
 where:
 Context = Interface
 State = Screen
 ConcreteStates = Screen[0-N]
 */
class Screen;
class Interface {
public:
	Interface();

	void setup();
	void loop();

	void left();
	void right();
	void ok();

	void move_to_screen(Screen* s);

public:
	/* room_width is signed value, negative for left adjustment.  */
	void to_lcd(const char* tpl);
	void to_lcd(const __FlashStringHelper* tpl);
	void to_lcd(const float d, const byte position_at_lcd,
	const int8_t room_width, const byte precission);
	void to_lcd(const int i, const byte position_at_lcd,
	const int8_t room_width);
	void to_lcd(const char* s, byte const position_at_lcd,
	const int8_t room_width, char fill_with = ' ');

	void cursor_at(byte c, byte r);
	void cursor_off();
	void update_lcd();
	bool inactive_timeout();

private:
	unsigned long last_activity_millis_;

	Screen* current_;
	bool lcd_is_on_;
	char at_lcd_[defaults::screen_size + 1];
	char to_lcd_[defaults::screen_size + 1];

	OneButton ok_;
	// Encoder:
	// https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
	// Mega interrupt capability pins: 2, 3, 18, 19, 20, 21
	Encoder encoder_;
	long last_encoder_pos_;
};

class Screen {
public:
	virtual void update_screen() = 0;

	// Enter and leave the state
	virtual void enter() {
	}
	virtual void leave() {
	}

	// Button clicked actions
	virtual void left() {
	}
	virtual void right() {
	}
	virtual void ok() {
	}

protected:
	virtual ~Screen() {
	}
	static Interface* interface_;
};

////////////////////////////////////////////////
// Concrete States
struct Home: public Screen {
	static Home* instance();
	void right();
	void ok();
	void update_screen();
};

// Edit Date and Time
class HomeEdit: public Screen {
public:
	static HomeEdit* instance();

	void enter();
	void leave();

	void right();
	void left();
	void ok();
	void update_screen();

protected:
	void edit_day();
	void edit_month();
	void edit_year();
	void edit_hour();
	void edit_minute();

private:
	enum class Editing {
		Day, Month, Year, Hour, Minute
	} editing_;
	TimeElements tm_;
};

struct Info: public Screen {
	static Info* instance();
	void left();
	void right();
	void update_screen();
};

struct Check: public Screen {
	static Check* instance();

	void ok();
	void left();
	void right();
	void update_screen();

};

class CheckEdit: public Screen {
public:

	static CheckEdit* instance();

	void ok();
	void update_screen();

protected:
	void check_phase(byte n);
	void check_tds();
	void check_temperatures();

private:
	enum class Checking {
		Phase, TDS, Temperatures
	} checking_;
};

}

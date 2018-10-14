#pragma once

#include <Arduino.h>
#include <OneButton.h>

#include <config.h>
#include <info.h>

namespace interface {

    void setup();
    void loop();

    // control interfaz programatically
    void up();
    void down();
    void left();
    void right();
    void ok();
    void esc();

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
    	    Interface(LCD* _lcd);

    	    void setup();
    	    void loop();

    	    void up();
    	    void down();
    	    void left();
    	    void right();
    	    void ok();
    	    void esc();

    	    void move_to_screen(Screen* s);

    	    /* room_width is signed value, negative for left adjustment.  */
    	    void to_lcd(const char* tpl);
    	    void to_lcd(const __FlashStringHelper* tpl);
    	    void to_lcd(const float d, const uint8_t position_at_lcd,
                    const int8_t room_width, const uint8_t precission);
    	    void to_lcd(const int i, const uint8_t position_at_lcd,
                    const int8_t room_width);
    	    void to_lcd(const char* s, uint8_t const position_at_lcd,
                    const int8_t room_width, char fill_with = ' ');

    	    void cursor_at(uint8_t c, uint8_t r);
    	    void cursor_off();

    	    void update_lcd();

  	private:
    	    unsigned long last_activity_millis_;

    	    Screen* current_;
    	    LCD* lcd_;
    	    bool lcd_is_on_;
    	    char at_lcd_[defaults::screen_size + 1];
    	    char to_lcd_[defaults::screen_size + 1];

    	    OneButton up_, down_, right_, left_, ok_, esc_;
    };

    class Screen {
  	public:
    	    virtual void update_screen() = 0;

    	    // Enter and leave the state
    	    virtual void enter() {}
    	    virtual void leave() {}

    	    // Button clicked actions
    	    virtual void up() {}
    	    virtual void down() {}
    	    virtual void left() {}
    	    virtual void right() {}
    	    virtual void ok() {}
    	    virtual void esc() {}

  	protected:
    	    virtual ~Screen() {}
    	    static Interface* interface_;
    };

    // Base class for photperiod
    struct ScreenPhotoperiodBase : public Screen {
    	void to_screen(const actuators::LedStrip& ls, const char* name);
    };

    struct ScreenPhotoperiodEdit : public ScreenPhotoperiodBase {
    	ScreenPhotoperiodEdit(actuators::PhotoPeriod p[3]);

    	void enter();
    	void leave();
    	void up();
    	void down();
    	void right();
    	void left();
    	void edit_current();

    	// Derived classes implement ok, esc & Constructor
    	virtual void ok() = 0;
    	virtual void esc() = 0;

    	struct Editing {
            uint8_t* hm;
            uint8_t c;
            uint8_t r;
            uint8_t max;
    	} editing_[12];
    	uint8_t current_;
    };

    ////////////////////////////////////////////////
    // Concrete States
    struct Home : public Screen {
    	static Home* instance();
    	void up();
    	void down();
    	void ok();
    	void update_screen();
    };

    // Edit Date and Time
    class HomeEdit : public Screen {
  	public:
    	    static HomeEdit* instance();

    	    void enter();
    	    void leave();

    	    void up();
    	    void down();
    	    void right();
    	    void left();
    	    void ok();
    	    void esc();
    	    void update_screen();

  	protected:
    	    void from_rtc();

    	    void edit_day();
    	    void edit_month();
    	    void edit_year();
    	    void edit_hour();
    	    void edit_minute();

  	private:
    	    enum class Editing { Day, Month, Year, Hour, Minute } editing_;
    	    TimeElements tm_;
    };

    struct External : public Screen {
    	static External* instance();
    	void up();
    	void down();
    	void update_screen();
    };

    struct Actuators : public Screen {
    	static Actuators* instance();
    	void up();
    	void down();
    	void ok();
    	void update_screen();
    };

    class ActuatorsEdit : public Screen {
  	public:
    	    static ActuatorsEdit* instance();

    	    void enter();
    	    void leave();

    	    void up();
    	    void down();
    	    void right();
    	    void left();
    	    void ok();
    	    void esc();

    	    void update_screen();

  	protected:
    	    void edit_min();
    	    void edit_max();

  	private:
    	    enum class Editing { Min, Max } editing_;
    	    struct {
        	float min;
        	float max;
    	    } values_;
    };

    struct WhiteLed : public ScreenPhotoperiodBase { // Photoperiod White
    	static WhiteLed* instance();
    	void up();
    	void down();
    	void ok();
    	void update_screen();
    };
    class WhiteLedEdit : public ScreenPhotoperiodEdit {
  	public:
    	    static WhiteLedEdit* instance();
    	    void ok();
    	    void esc();
    	    void update_screen();

  	protected:
    	    WhiteLedEdit();
    };
    struct RedLed : public ScreenPhotoperiodBase { // Photoperiod Red
    	static RedLed* instance();
    	void up();
    	void down();
    	void ok();
    	void update_screen();
    };

    class RedLedEdit : public ScreenPhotoperiodEdit {
  	public:
    	    static RedLedEdit* instance();
    	    void ok();
    	    void esc();
    	    void update_screen();

  	protected:
    	    RedLedEdit();
    };
    struct GreenLed : public ScreenPhotoperiodBase { // Photoperiod Green
    	static GreenLed* instance();
    	void up();
    	void down();
    	void ok();
    	void update_screen();
    };
    class GreenLedEdit : public ScreenPhotoperiodEdit {
  	public:
    	    static GreenLedEdit* instance();
    	    void ok();
    	    void esc();
    	    void update_screen();

  	protected:
    	    GreenLedEdit();
    };
    struct BlueLed : public ScreenPhotoperiodBase { // Photoperiod Blue
    	static BlueLed* instance();
    	void up();
    	void down();
    	void ok();
    	void update_screen();
    };
    class BlueLedEdit : public ScreenPhotoperiodEdit {
  	public:
    	    static BlueLedEdit* instance();
    	    void ok();
    	    void esc();
    	    void update_screen();

  	protected:
    	    BlueLedEdit();
    };

    struct UltraVioletLed : public ScreenPhotoperiodBase { // Photoperiod Blue
    	static UltraVioletLed* instance();
    	void up();
    	void down();
    	void ok();
    	void update_screen();
    };
    class UltraVioletLedEdit : public ScreenPhotoperiodEdit {
  	public:
    	    static UltraVioletLedEdit* instance();
    	    void ok();
    	    void esc();
    	    void update_screen();

  	protected:
    	    UltraVioletLedEdit();
    };

    struct Feeding : public Screen {
    	static Feeding* instance();
    	void up();
    	void down();
    	void ok();
    	void update_screen();
    };

    class FeedingEdit : public Feeding {
  	public:
    	    static FeedingEdit* instance();

    	    void enter();
    	    void leave();

    	    void up();
    	    void down();
    	    void right();
    	    void left();
    	    void ok();
    	    void esc();

  	protected:
    	    FeedingEdit();
    	    void edit_current();

    	    struct Editing {
        	uint8_t* hm;
        	uint8_t c;
        	uint8_t r;
        	uint8_t max;
    	    } editing_[7];
    	    uint8_t current_;
    };

    struct PH : public Screen {
    	static PH* instance();
    	void up();
    	void down();
    	void ok();
    	void update_screen();
    };
    class PHEdit : public Screen {
  	public:
    	    static PHEdit* instance();

    	    void enter();
    	    void ok();
    	    void esc();

    	    void update_screen();

  	private:
    	    enum class Editing : uint8_t {
        	Start = 0,
        	ph4 = 4,
        	ph7 = 7,
        	ph10 = 10
    	    } editing_;
    };

    struct TDS : public Screen {
    	static TDS* instance();
    	void up();
    	void down();
    	void update_screen();
    };

    struct System : public Screen {
    	static System* instance();
    	void up();
    	void down();
    	void update_screen();
    };

} // namespace interface

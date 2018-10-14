#include <Arduino.h>
#include <Time.h>
#include <stdlib.h>

#include <interface.h>

namespace interface {

    // The "Script"
    static Interface interface{config::lcd};
    Interface* Screen::interface_ = &interface;

    void setup()
    {
      p(F("Interface::setup()"));
      interface.setup();
    }

    void loop() { interface.loop(); }

    void up() { interface.up(); }
    void down() { interface.down(); }
    void left() { interface.left(); }
    void right() { interface.right(); }
    void ok() { interface.ok(); }
    void esc() { interface.esc(); }

    /* Interface Implementation */
    Interface::Interface(LCD* _lcd)
      : lcd_(_lcd), up_(defaults::pines.btn_up, true),
        down_(defaults::pines.btn_down, true),
        right_(defaults::pines.btn_right, true),
        left_(defaults::pines.btn_left, true), ok_(defaults::pines.btn_ok, true),
        esc_(defaults::pines.btn_esc, true)
    {
      at_lcd_[defaults::screen_size] = {' '};
      to_lcd_[defaults::screen_size] = {' '};
      last_activity_millis_ = 0;
      lcd_is_on_ = false;
      current_ = nullptr;
    }

    void Interface::setup()
    {
      last_activity_millis_ = millis();
      lcd_->on();
      current_ = Home::instance();
      current_->enter();

      up_.attachClick(interface::up);
      down_.attachClick(interface::down);
      left_.attachClick(interface::left);
      right_.attachClick(interface::right);
      ok_.attachClick(interface::ok);
      esc_.attachClick(interface::esc);
    }

    void Interface::loop()
    {
      up_.tick();
      down_.tick();
      left_.tick();
      right_.tick();
      ok_.tick();
      esc_.tick();

      // If inactive switch off the screen
      if ((millis() > (last_activity_millis_ + defaults::max_inactive_millis)) &&
          lcd_is_on_) {

            lcd_->off();
            lcd_is_on_ = false;
      }
      else {
            current_->update_screen();
            update_lcd();
      }
    }

    #define INTERFAZ_ACTION(method)                                                \
        void Interface::method()                                                   \
        {                                                                          \
            lcd_is_on_ = true;                                                     \
            lcd_->on();                                                            \
            last_activity_millis_ = millis();                                      \
            current_->method();                                                    \
        }

    INTERFAZ_ACTION(up)
    INTERFAZ_ACTION(down)
    INTERFAZ_ACTION(left)
    INTERFAZ_ACTION(right)
    INTERFAZ_ACTION(ok)
    INTERFAZ_ACTION(esc)

    #undef INTERFAZ_ACTION

  /*
    void Interface::up()
    {
      lcd_is_on_ = true;
      lcd_->on();
      last_activity_millis_ = millis();
      current_->up();
    }

    void Interface::down()
    {
      lcd_is_on_ = true;
      lcd_->on();
      last_activity_millis_ = millis();
      current_->down();
    }

    void Interface::left()
    {
      lcd_is_on_ = true;
      lcd_->on();
      last_activity_millis_ = millis();
      current_->left();
    }

    void Interface::right()
    {
      lcd_is_on_ = true;
      lcd_->on();
      last_activity_millis_ = millis();
      current_->right();
    }
    void Interface::ok()
    {
      lcd_is_on_ = true;
      lcd_->on();
      last_activity_millis_ = millis();
      current_->ok();
    }
    void Interface::esc()
    {
      lcd_is_on_ = true;
      lcd_->on();
      last_activity_millis_ = millis();
      current_->esc();
    }
  */
    void Interface::move_to_screen(Screen* s)
    {
      current_->leave();
      current_ = s;
      current_->enter();
    }

    void Interface::update_lcd()
    {
      uint8_t r, c, i;
      for (c = 0; c < defaults::screen_cols; c++) {
            for (r = 0; r < defaults::screen_rows; r++) {
              i = (r * defaults::screen_cols) + c;
              if (to_lcd_[i] != at_lcd_[i]) {
                    lcd_->setCursor(c, r);
                    lcd_->print(to_lcd_[i]);
                    at_lcd_[i] = to_lcd_[i];
              }
            }
      }
    }

    void Interface::to_lcd(const __FlashStringHelper* tpl)
    {
      PGM_P p = reinterpret_cast<PGM_P>(tpl);
      size_t i = 0;
      while (1) {
            unsigned char c = pgm_read_byte(p++);
            if (c == 0)
              break;
            to_lcd_[i] = (char)c;
            i++;
      }
    }

    void Interface::to_lcd(const char* tpl)
    {
      strncpy(to_lcd_, tpl, defaults::screen_size);
    }

    /* width is signed value, negative for left adjustment.  */
    void Interface::to_lcd(const float d, const uint8_t position_at_lcd,
            const int8_t room_width, const uint8_t precission)
    {
      char a[defaults::screen_cols + 1];
      dtostrf((double)d, room_width, precission, a);
      to_lcd(a, position_at_lcd, room_width);
    }

    /* If room_width < 0 align right */
    void Interface::to_lcd(const int i, const uint8_t position_at_lcd,
            const int8_t room_width)
    {
      char a[defaults::screen_cols + 1];
      itoa(i, a, 10);
      to_lcd(a, position_at_lcd, room_width, '0');
    }

    /* If room_width < 0 align right */
    void Interface::to_lcd(const char* src, const uint8_t position_at_lcd,
            const int8_t room_width, char fill_with)
    {
      uint8_t len_s = (uint8_t)strlen(src);
      uint8_t width = (uint8_t)abs(room_width);
      char* dest = &to_lcd_[position_at_lcd];
      uint8_t i;

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
      }
      // algn left
      else {
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

    void Interface::cursor_at(uint8_t c, uint8_t r)
    {
      lcd_->cursor();
      lcd_->setCursor(c, r);
      lcd_->blink();
    }

    void Interface::cursor_off()
    {
      lcd_->noBlink();
      lcd_->noCursor();
    }

    /* ConcreteStates (Screens) implementation */
    Home* Home::instance()
    {
      static Home screen;
      return &screen;
    }
    void Home::down() { interface_->move_to_screen(External::instance()); }
    void Home::up() { interface_->move_to_screen(System::instance()); }
    void Home::ok() { interface_->move_to_screen(HomeEdit::instance()); }
    void Home::update_screen()
    {
      sensors::SensorsData data = sensors::get_sensors_data();

      //                    01234567890123456789
      interface_->to_lcd(F("ddd,dd/mmm     HH:MM"
                    " T  ,  TDS    pH ,  "
                    "Leds Sun      UV    "
                    "R     G      B      "));

      TimeElements tm;
      RTC.read(tm);
      interface_->to_lcd(dayShortStr(tm.Wday), 0, 3);
      interface_->to_lcd(tm.Day, 4, -2);
      interface_->to_lcd(monthShortStr(tm.Month), 7, 3);
      interface_->to_lcd(tm.Hour, 15, -2);
      interface_->to_lcd(tm.Minute, 18, -2);

      interface_->to_lcd(data.ds18b20.internal, 20 + 2, -4, 1);
      interface_->to_lcd(data.tds.tds, 20 + 10, -3);
      interface_->to_lcd(data.ph.ph, 20 + 16, -3, 1);

      interface_->to_lcd((int)(actuators::get_white()).pwm, 40 + 9, -3);
      interface_->to_lcd((int)(actuators::get_uv()).pwm, 40 + 17, -3);

      interface_->to_lcd((int)(actuators::get_red()).pwm, 60 + 2, -3);
      interface_->to_lcd((int)(actuators::get_green()).pwm, 60 + 8, -3);
      interface_->to_lcd((int)(actuators::get_blue()).pwm, 60 + 15, -3);
    }

    HomeEdit* HomeEdit::instance()
    {
      static HomeEdit screen;
      return &screen;
    }

    void HomeEdit::edit_day()
    {
      editing_ = Editing::Day;
      interface_->cursor_at(2, 2);
    }

    void HomeEdit::edit_month()
    {
      editing_ = Editing::Month;
      interface_->cursor_at(6, 2);
    }

    void HomeEdit::edit_year()
    {
      editing_ = Editing::Year;
      interface_->cursor_at(11, 2);
    }

    void HomeEdit::edit_hour()
    {
      editing_ = Editing::Hour;
      interface_->cursor_at(15, 2);
    }

    void HomeEdit::edit_minute()
    {
      editing_ = Editing::Minute;
      interface_->cursor_at(18, 2);
    }

    void HomeEdit::from_rtc() { RTC.read(tm_); }

    void HomeEdit::enter()
    {
      from_rtc();
      edit_day();
    }

    void HomeEdit::leave()
    {
      if (!RTC.write(tm_))
            p(F("Time set ERROR!"));

      interface_->cursor_off();
    }

    void HomeEdit::up()
    {
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

    void HomeEdit::down()
    {
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
    void HomeEdit::right()
    {
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
          break;
      }
    }
    void HomeEdit::left()
    {
      switch (editing_) {
          case (Editing::Day):
          break;
          case (Editing::Month):
          edit_day();
          break;
          case (Editing::Year):
          edit_month();
          break;
          case (Editing::Hour):
          edit_year();
          break;
          case (Editing::Minute):
          edit_hour();
          break;
      }
    }

    void HomeEdit::ok() { interface_->move_to_screen(Home::instance()); }
    void HomeEdit::esc()
    {
      from_rtc();
      interface_->move_to_screen(Home::instance());
    }

    void HomeEdit::update_screen()
    {
      //                    01234567890123456789
      interface_->to_lcd(F("                    "
                    "   New Date Time    "
                    " 01/Ene/1970  14:00 "
                    "                    "));
      interface_->to_lcd((int)tm_.Day, 40 + 1, -2);
      interface_->to_lcd(monthShortStr(tm_.Month), 40 + 4, 3);
      interface_->to_lcd(tmYearToCalendar(tm_.Year), 40 + 8, -4);
      interface_->to_lcd((int)tm_.Hour, 40 + 14, -2);
      interface_->to_lcd((int)tm_.Minute, 40 + 17, -2);
    }

    External* External::instance()
    {
      static External screen;
      return &screen;
    }
    void External::up() { interface_->move_to_screen(Home::instance()); }
    void External::down() { interface_->move_to_screen(Actuators::instance()); }
    void External::update_screen()
    {
      sensors::SensorsData data = sensors::get_sensors_data();

      //                    01234567890123456789
      interface_->to_lcd(F("External Conditions "
                    "Temperature   99,9 C"
                    "Humidity      99,9 %"
                    "Pressure    9999 hPa"));

      interface_->to_lcd(data.bmp280t.temperature, 20 + 14, -4, 1);
      interface_->to_lcd(data.dht11.humidity, 40 + 14, -4, 1);
      interface_->to_lcd(data.bmp280t.pressure, 60 + 12, -4, 0);
    }

    Actuators* Actuators::instance()
    {
      static Actuators screen;
      return &screen;
    }
    void Actuators::up() { interface_->move_to_screen(Home::instance()); }
    void Actuators::down() { interface_->move_to_screen(WhiteLed::instance()); }
    void Actuators::ok() { interface_->move_to_screen(ActuatorsEdit::instance()); }
    void Actuators::update_screen()
    {
      //                    01234567890123456789
      interface_->to_lcd(F("Auto Feed     :  ---"
                    "Fans/Air Pump :  ---"
                    "Filter/Heater :  ---"
                    "Mim/Max: --,-C --,-C"));

#define OnOff(v) ((v) ? "On" : "Off")
      interface_->to_lcd(OnOff(config::feeding.active), 17, -3);
      interface_->to_lcd(OnOff(actuators::fan.is_on()), 20 + 17, -3);
      interface_->to_lcd(OnOff(actuators::filter.is_on()), 40 + 17, -3);
      interface_->to_lcd(config::low_temperature_alarm, 60 + 9, -4, 1);
      interface_->to_lcd(config::high_temperature_alarm, 60 + 15, -4, 1);
#undef OnOff
    }

    ActuatorsEdit* ActuatorsEdit::instance()
    {
      static ActuatorsEdit screen;
      return &screen;
    }

    void ActuatorsEdit::enter() { edit_min(); }

    void ActuatorsEdit::leave()
    {
      config::update_eeprom();
      interface_->cursor_off();
    }

    void ActuatorsEdit::up()
    {
      switch (editing_) {
          case (Editing::Min):
          break;
          case (Editing::Max):
          edit_min();
          break;
      }
    }
    void ActuatorsEdit::down()
    {
      switch (editing_) {
          case (Editing::Min):
          edit_max();
          break;
          case (Editing::Max):
          break;
      }
    }
    void ActuatorsEdit::right()
    {
      switch (editing_) {
          case (Editing::Min):
          config::low_temperature_alarm += 0.5f;
          break;
          case (Editing::Max):
          config::high_temperature_alarm += 0.5f;
          break;
      }
    }
    void ActuatorsEdit::left()
    {
      switch (editing_) {
          case (Editing::Min):
          config::low_temperature_alarm -= 0.5f;
          break;
          case (Editing::Max):
          config::high_temperature_alarm -= 0.5f;
          break;
      }
    }
    void ActuatorsEdit::ok() { interface_->move_to_screen(Actuators::instance()); }
    void ActuatorsEdit::esc()
    {
      config::load_from_eeprom();
      interface_->move_to_screen(Actuators::instance());
    }

    void ActuatorsEdit::edit_min()
    {
      editing_ = Editing::Min;
      interface_->cursor_at(18, 1);
    }

    void ActuatorsEdit::edit_max()
    {
      editing_ = Editing::Min;
      interface_->cursor_at(18, 2);
    }

    void ActuatorsEdit::update_screen()
    {
      //                    01234567890123456789
      interface_->to_lcd(F(" Temperature Limits "
                    "Max/Start Fan: 00,0C"
                    "Min/Stop Fan : 00,0C"
                    "                    "));

      interface_->to_lcd(config::low_temperature_alarm, 20 + 15, -4, 1);
      interface_->to_lcd(config::high_temperature_alarm, 40 + 15, -4, 1);
    }

    // -------------------------- FOTOPERIODOS ----------------------------------//

    // ---- Luz Blanca
    WhiteLed* WhiteLed::instance()
    {
      static WhiteLed screen;
      return &screen;
    }
    void WhiteLed::up() { interface_->move_to_screen(Actuators::instance()); }
    void WhiteLed::down() { interface_->move_to_screen(RedLed::instance()); }
    void WhiteLed::ok() { interface_->move_to_screen(WhiteLedEdit::instance()); }
    void WhiteLed::update_screen() { to_screen(config::white, "WHITE"); }

    WhiteLedEdit::WhiteLedEdit() : ScreenPhotoperiodEdit(config::white.periods) {}
    WhiteLedEdit* WhiteLedEdit::instance()
    {
      static WhiteLedEdit screen;
      return &screen;
    }
    void WhiteLedEdit::ok() { interface_->move_to_screen(WhiteLed::instance()); }
    void WhiteLedEdit::esc()
    {
      config::load_from_eeprom();
      interface_->move_to_screen(WhiteLed::instance());
    }
    void WhiteLedEdit::update_screen() { to_screen(config::white, "WHITE"); }

    // ---- Luz Roja
    RedLed* RedLed::instance()
    {
      static RedLed screen;
      return &screen;
    }
    void RedLed::up() { interface_->move_to_screen(WhiteLed::instance()); }
    void RedLed::down() { interface_->move_to_screen(GreenLed::instance()); }
    void RedLed::ok() { interface_->move_to_screen(RedLedEdit::instance()); }
    void RedLed::update_screen() { to_screen(config::red, "RED"); }

    RedLedEdit::RedLedEdit() : ScreenPhotoperiodEdit(config::red.periods) {}
    RedLedEdit* RedLedEdit::instance()
    {
      static RedLedEdit screen;
      return &screen;
    }
    void RedLedEdit::ok() { interface_->move_to_screen(RedLed::instance()); }
    void RedLedEdit::esc()
    {
      config::load_from_eeprom();
      interface_->move_to_screen(RedLed::instance());
    }
    void RedLedEdit::update_screen() { to_screen(config::red, "RED"); }

    // ---- Luz Verde
    GreenLed* GreenLed::instance()
    {
      static GreenLed screen;
      return &screen;
    }
    void GreenLed::up() { interface_->move_to_screen(RedLed::instance()); }
    void GreenLed::down() { interface_->move_to_screen(BlueLed::instance()); }
    void GreenLed::ok() { interface_->move_to_screen(GreenLedEdit::instance()); }
    void GreenLed::update_screen() { to_screen(config::green, "GREEN"); }

    GreenLedEdit::GreenLedEdit() : ScreenPhotoperiodEdit(config::green.periods) {}
    GreenLedEdit* GreenLedEdit::instance()
    {
      static GreenLedEdit screen;
      return &screen;
    }
    void GreenLedEdit::ok() { interface_->move_to_screen(GreenLed::instance()); }
    void GreenLedEdit::esc()
    {
      config::load_from_eeprom();
      interface_->move_to_screen(GreenLed::instance());
    }
    void GreenLedEdit::update_screen() { to_screen(config::green, "GREEN"); }

    // ---- Luz Azul
    BlueLed* BlueLed::instance()
    {
      static BlueLed screen;
      return &screen;
    }
    void BlueLed::up() { interface_->move_to_screen(GreenLed::instance()); }
    void BlueLed::down() { interface_->move_to_screen(UltraVioletLed::instance()); }
    void BlueLed::ok() { interface_->move_to_screen(BlueLedEdit::instance()); }
    void BlueLed::update_screen() { to_screen(config::blue, "BLUE"); }

    BlueLedEdit::BlueLedEdit() : ScreenPhotoperiodEdit(config::blue.periods) {}
    BlueLedEdit* BlueLedEdit::instance()
    {
      static BlueLedEdit screen;
      return &screen;
    }
    void BlueLedEdit::ok() { interface_->move_to_screen(BlueLed::instance()); }
    void BlueLedEdit::esc()
    {
      config::load_from_eeprom();
      interface_->move_to_screen(BlueLed::instance());
    }
    void BlueLedEdit::update_screen() { to_screen(config::blue, "BLUE"); }

    // ---- Luz Ultravioleta
    UltraVioletLed* UltraVioletLed::instance()
    {
      static UltraVioletLed screen;
      return &screen;
    }
    void UltraVioletLed::up() { interface_->move_to_screen(BlueLed ::instance()); }
    void UltraVioletLed::down() { interface_->move_to_screen(Feeding::instance()); }
    void UltraVioletLed::ok()
    {
      interface_->move_to_screen(UltraVioletLedEdit::instance());
    }
    void UltraVioletLed::update_screen() { to_screen(config::uv, "UV"); }

    UltraVioletLedEdit::UltraVioletLedEdit()
      : ScreenPhotoperiodEdit(config::uv.periods)
    {
    }
    UltraVioletLedEdit* UltraVioletLedEdit::instance()
    {
      static UltraVioletLedEdit screen;
      return &screen;
    }
    void UltraVioletLedEdit::ok()
    {
      interface_->move_to_screen(UltraVioletLed::instance());
    }
    void UltraVioletLedEdit::esc()
    {
      config::load_from_eeprom();
      interface_->move_to_screen(UltraVioletLed::instance());
    }
    void UltraVioletLedEdit::update_screen() { to_screen(config::uv, "UV"); }

    // ---- Alimentador Automático
    Feeding* Feeding::instance()
    {
      static Feeding screen;
      return &screen;
    }
    void Feeding::up() { interface_->move_to_screen(UltraVioletLed::instance()); }
    void Feeding::down() { interface_->move_to_screen(PH::instance()); }
    void Feeding::ok() { interface_->move_to_screen(FeedingEdit::instance()); }
    void Feeding::update_screen()
    {
      //                    01234567890123456789
      interface_->to_lcd(F("Breakfast:     --:--"
                    "Lunch    :     --:--"
                    "Dinner   :     --:--"
                    "Fasting  :      None"));

      interface_->to_lcd((int)config::feeding.breakfast.hour, 15, -2);
      interface_->to_lcd((int)config::feeding.breakfast.minute, 18, -2);
      interface_->to_lcd((int)config::feeding.lunch.hour, 20 + 15, -2);
      interface_->to_lcd((int)config::feeding.lunch.minute, 20 + 18, -2);
      interface_->to_lcd((int)config::feeding.dinner.hour, 40 + 15, -2);
      interface_->to_lcd((int)config::feeding.dinner.minute, 40 + 18, -2);
      interface_->to_lcd(dayStr(config::feeding.fasting_day), 60 + 11, -9);
    }

    FeedingEdit::FeedingEdit()
      : editing_{
            {&config::feeding.breakfast.hour, 16, 0, 23},
            {&config::feeding.breakfast.minute, 19, 0, 59},
            {&config::feeding.lunch.hour, 16, 1, 23},
            {&config::feeding.lunch.minute, 19, 1, 59},
            {&config::feeding.dinner.hour, 16, 2, 23},
            {&config::feeding.dinner.minute, 19, 2, 59},
            {&config::feeding.fasting_day, 19, 3, 6},
        }
    {
    }

    void FeedingEdit::edit_current()
    {
      Editing* e = &editing_[current_];
      interface_->cursor_at(e->c, e->r);
    }

    FeedingEdit* FeedingEdit::instance()
    {
      static FeedingEdit screen;
      return &screen;
    }

    void FeedingEdit::enter()
    {
      current_ = 0;
      edit_current();
    }

    void FeedingEdit::leave()
    {
      config::update_eeprom();
      interface_->cursor_off();
    }

    void FeedingEdit::up()
    {
      Editing* e = &editing_[current_];
      *(e->hm) = (*(e->hm) == e->max) ? 0 : *(e->hm) + 1;
    }
    void FeedingEdit::down()
    {
      Editing* e = &editing_[current_];
      *(e->hm) = (*(e->hm) == 0) ? e->max : *(e->hm) - 1;
    }

    void FeedingEdit::right()
    {
      current_ = (current_ == 7) ? 0 : current_ + 1;
      edit_current();
    }

    void FeedingEdit::left()
    {
      current_ = (current_ == 0) ? 7 : current_ - 1;
      edit_current();
    }

    void FeedingEdit::ok() { interface_->move_to_screen(Feeding::instance()); }
    void FeedingEdit::esc()
    {
      config::load_from_eeprom();
      interface_->move_to_screen(Feeding::instance());
    }

    PH* PH::instance()
    {
      static PH screen;
      return &screen;
    }
    void PH::up() { interface_->move_to_screen(Feeding::instance()); }
    void PH::down() { interface_->move_to_screen(TDS::instance()); }
    void PH::ok() { interface_->move_to_screen(PHEdit::instance()); }
    void PH::update_screen()
    {
      //                    01234567890123456789
      interface_->to_lcd(F(" Read PH        --,-"
                    "                    "
                    "      Press Ok      "
                    "    to Calibrate    "));

      sensors::SensorsData data = sensors::get_sensors_data();
      interface_->to_lcd(data.ph.ph, 16, -4, 1);
    }

    PHEdit* PHEdit::instance()
    {
      static PHEdit screen;
      return &screen;
    }

    void PHEdit::enter() { editing_ = Editing::Start; }

    void PHEdit::ok()
    {
      switch (editing_) {
          case (Editing::Start):
          editing_ = Editing::ph4;
          break;
          case (Editing::ph4):
          sensors::PH::calibrate_ph4();
          editing_ = Editing::ph7;
          break;
          case (Editing::ph7):
          sensors::PH::calibrate_ph7();
          break;
          case (Editing::ph10):
          sensors::PH::calibrate_ph10();
          config::update_eeprom();
          interface_->move_to_screen(PH::instance());
          break;
      }
    }

    void PHEdit::esc() { interface_->move_to_screen(PH::instance()); }

    void PHEdit::update_screen()
    {
      //                        01234567890123456789
      if (Editing::Start == editing_) {
            interface_->to_lcd(F("   Calibrating PH   "
                        "                    "
                        "     OK to Start    "
                        "     ESC to Exit    "));
      }
      else {
            interface_->to_lcd(F("   Calibrating PH   "
                        "         99,9       "
                        " Use PH 99 test and "
                        "   Press Ok after   "));

            interface_->to_lcd(sensors::PH::get_ph(), 20 + 10, -4, 1);
            interface_->to_lcd((const int)editing_, 40 + 10, -2);
      }
    }

    // TODO Calibrar TDS
    TDS* TDS::instance()
    {
      static TDS screen;
      return &screen;
    }
    void TDS::up() { interface_->move_to_screen(PH::instance()); }
    void TDS::down() { interface_->move_to_screen(System::instance()); }
    void TDS::update_screen()
    {
      //                    01234567890123456789
      interface_->to_lcd(F("        TDS         "
                    "                    "
                    " (Not implemented)  "
                    "                    "));
      /*
         sensors::SensorsData data = sensors::get_sensors_data();

#define OkKo(v) ((v) ? "Ok" : "KO")
interface_->to_lcd((int)freeRam(), 15, -5);
interface_->to_lcd(OkKo(data.ds18b20.sensor_ok), 20 + 18, 2);
interface_->to_lcd(OkKo(data.dht11.sensor_ok), 40 + 18, 2);
interface_->to_lcd(OkKo(data.bmp280t.sensor_ok), 60 + 18, 2);
#undef OkKo
*/
    }

    System* System::instance()
    {
      static System screen;
      return &screen;
    }
    void System::up() { interface_->move_to_screen(TDS::instance()); }
    void System::down() { interface_->move_to_screen(Home::instance()); }
    void System::update_screen()
    {
      //                    01234567890123456789
      interface_->to_lcd(F("Free Memory:   -----"
                    "Temperature Stat: --"
                    "Humidity Status : --"
                    "Pressure Status : --"));

      sensors::SensorsData data = sensors::get_sensors_data();

#define OkKo(v) ((v) ? "Ok" : "KO")
      interface_->to_lcd((int)freeRam(), 15, -5);
      interface_->to_lcd(OkKo(data.ds18b20.sensor_ok), 20 + 18, 2);
      interface_->to_lcd(OkKo(data.dht11.sensor_ok), 40 + 18, 2);
      interface_->to_lcd(OkKo(data.bmp280t.sensor_ok), 60 + 18, 2);
#undef OkKo
    }

    ///////////////////////////////////////////////////////////////////////
    // Base clases
    void ScreenPhotoperiodBase::to_screen(const actuators::LedStrip& ls,
            const char* name)
    {
      //                    01234567890123456789
      interface_->to_lcd(F("Photoperiod         "
                    "Rise:  00:00 - 00:00"
                    "On  :  00:00 - 00:00"
                    "Fall:  00:00 - 00:00"));

      interface_->to_lcd(name, 12, -8);

      interface_->to_lcd(ls.periods[0].init.hour, 20 + 7, -2);
      interface_->to_lcd(ls.periods[0].init.minute, 20 + 10, -2);
      interface_->to_lcd(ls.periods[0].end.hour, 20 + 15, -2);
      interface_->to_lcd(ls.periods[0].end.minute, 20 + 18, -2);

      interface_->to_lcd(ls.periods[1].init.hour, 40 + 7, -2);
      interface_->to_lcd(ls.periods[1].init.minute, 40 + 10, -2);
      interface_->to_lcd(ls.periods[1].end.hour, 40 + 15, -2);
      interface_->to_lcd(ls.periods[1].end.minute, 40 + 18, -2);

      interface_->to_lcd(ls.periods[2].init.hour, 60 + 7, -2);
      interface_->to_lcd(ls.periods[2].init.minute, 60 + 10, -2);
      interface_->to_lcd(ls.periods[2].end.hour, 60 + 15, -2);
      interface_->to_lcd(ls.periods[2].end.minute, 60 + 18, -2);
    }

    ScreenPhotoperiodEdit::ScreenPhotoperiodEdit(actuators::PhotoPeriod p[])
      : editing_{
            // Rise
            {&p[0].init.hour, 8, 1, 23},
            {&p[0].init.minute, 11, 1, 59},
            {&p[0].end.hour, 16, 1, 23},
            {&p[0].end.minute, 19, 1, 59},
            // On
            {&p[1].init.hour, 8, 2, 23},
            {&p[1].init.minute, 11, 2, 59},
            {&p[1].end.hour, 16, 2, 23},
            {&p[1].end.minute, 19, 2, 59},
            // Fall
            {&p[2].init.hour, 8, 3, 23},
            {&p[2].init.minute, 11, 3, 59},
            {&p[2].end.hour, 16, 3, 23},
            {&p[2].end.minute, 19, 3, 59},
        }
    {
    }

    void ScreenPhotoperiodEdit::enter()
    {
      current_ = 0;
      edit_current();
    }

    void ScreenPhotoperiodEdit::leave()
    {
      config::update_eeprom();
      interface_->cursor_off();
    }

    void ScreenPhotoperiodEdit::edit_current()
    {
      Editing* e = &editing_[current_];
      interface_->cursor_at(e->c, e->r);
    }

    void ScreenPhotoperiodEdit::up()
    {
      Editing* e = &editing_[current_];
      *(e->hm) = (*(e->hm) == e->max) ? 0 : *(e->hm) + 1;
    }
    void ScreenPhotoperiodEdit::down()
    {
      Editing* e = &editing_[current_];
      *(e->hm) = (*(e->hm) == 0) ? e->max : *(e->hm) - 1;
    }

    void ScreenPhotoperiodEdit::right()
    {
      current_ = (current_ == 11) ? 0 : current_ + 1;
      edit_current();
    }

    void ScreenPhotoperiodEdit::left()
    {
      current_ = (current_ == 0) ? 11 : current_ - 1;
      edit_current();
    }
} // namespace interface

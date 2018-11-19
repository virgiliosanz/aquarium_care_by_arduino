#pragma once

#include <Arduino.h>

namespace actuators {

// Public Interfaz
void setup();
void loop();

void switch_pump();
void switch_filter_and_heater();

bool co2_automatic();
void co2_behaviour(bool automatic);
void switch_co2(bool on);

bool lights_automatic();
void lights_behaviour(bool automatic);
void switch_lights(bool on[]);
bool* phases_on();

// Clases y apoyo
class Rele {
  public:
    Rele(byte _pin);
    Rele();

    void set_pin(byte _pin);
    void toggle();
    void on();
    void off();
    bool is_on() const;
    bool is_off() const;

  private:
    byte pin;
    byte value;
};

extern Rele fan;
extern Rele filter;
extern Rele heater;
extern Rele pump;
extern Rele co2;
// extern Rele air_pump;
extern Rele phases[];

} // namespace actuators

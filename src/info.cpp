#include <info.h>

unsigned freeRam()
{
    extern unsigned __heap_start, *__brkval;
    unsigned v;
    return (unsigned)&v -
           (__brkval == 0 ? (unsigned)&__heap_start : (unsigned)__brkval);
}

// http://playground.arduino.cc/Main/Printf
void p(const __FlashStringHelper* fmt, ...)
{
    char buf[129] PROGMEM;
    va_list args;
    va_start(args, fmt);
#ifdef __AVR__
    vsnprintf_P(buf, 128, (const char*)fmt, args);
#else
    vsnprintf(buf, 128, (const char*)fmt, args);
#endif
    va_end(args);
    Serial.println(buf);
}

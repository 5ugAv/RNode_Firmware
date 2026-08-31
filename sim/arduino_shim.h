// Minimal Arduino surface so the firmware's display layer compiles on a host.
// Nothing here models hardware: the point is that draw_stat_area() and
// draw_disp_area() are pure software that paint into two GFXcanvas1 buffers,
// so they can be run and LOOKED AT without a board.
#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>

#define PROGMEM
#define pgm_read_byte(a)  (*(const unsigned char *)(a))
#define pgm_read_word(a)  (*(const unsigned short *)(a))
#define pgm_read_dword(a) (*(const unsigned long *)(a))
#define pgm_read_pointer(a) ((void *)pgm_read_dword(a))
#define F(x) (x)
#define PSTR(x) (x)

typedef std::string String;
typedef uint8_t byte;
typedef bool boolean;

// The harness drives time, so a test can place the display at any moment.
extern unsigned long SIM_MILLIS;
inline unsigned long millis() { return SIM_MILLIS; }
inline void delay(unsigned long) {}
inline void delayMicroseconds(unsigned long) {}
inline void yield() {}

inline int  digitalRead(int) { return 0; }   // BUSY low = panel idle
inline void digitalWrite(int, int) {}
inline void pinMode(int, int) {}
inline void analogWrite(int, int) {}
inline long random(long n) { return n / 2; }

#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2

// --- extras the GFX library expects from the Arduino core -----------------
#include <cmath>
class __FlashStringHelper;
inline float radians(float d) { return d * 0.017453292519943295f; }
inline float degrees(float r) { return r * 57.29577951308232f; }
using std::sin; using std::cos; using std::abs;
#ifndef _min
#define _min(a,b) ((a)<(b)?(a):(b))
#define _max(a,b) ((a)>(b)?(a):(b))
#endif

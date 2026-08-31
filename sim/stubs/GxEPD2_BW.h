#pragma once
#include <cstdint>
#include "SPI.h"
// A no-op panel. The firmware paints into two GFXcanvas1 buffers and only then
// blits them here, so everything the simulator wants to see has already
// happened by the time these are called.
#define GxEPD_WHITE 0xFFFF
#define GxEPD_BLACK 0x0000
struct GxEPD2_213_B74 { static const uint16_t WIDTH = 122, HEIGHT = 250;
  GxEPD2_213_B74() {} template <typename... A> GxEPD2_213_B74(A...) {} };
struct GxEPD2_213_BN  { static const uint16_t WIDTH = 122, HEIGHT = 250; };
struct GxEPD2_154_D67 { static const uint16_t WIDTH = 200, HEIGHT = 200; };
template <typename T, uint16_t H> class GxEPD2_BW {
 public:
  struct Epd2 { void setBusyCallback(void (*)(const void*)) {} } epd2;
  GxEPD2_BW() {} template <typename... A> GxEPD2_BW(A...) {}
  void init() {} template <typename... A> void init(A...) {}
  void begin() {} void clear() {} void clearDisplay() {}
  void display() {} void display(bool) {}
  void setRotation(uint8_t) {} void setFullWindow() {}
  void setPartialWindow(int,int,int,int) {}
  void fillScreen(uint16_t) {} void fillRect(int,int,int,int,uint16_t) {}
  void drawLine(int,int,int,int,uint16_t) {}
  void drawBitmap(int,int,const uint8_t*,int,int,uint16_t) {}
  void drawBitmap(int,int,const uint8_t*,int,int,uint16_t,uint16_t) {}
  void hibernate() {} void powerOff() {}
  void setColor(uint16_t) {} void setRGB(uint8_t,uint8_t,uint8_t) {}
  void setSPISpeed(uint32_t) {}
  void cp437(bool=true) {}
  void setTextSize(uint8_t) {} void setTextColor(uint16_t) {}
  void setCursor(int,int) {} void setFont(const void*) {}
  void print(const char*) {} void print(int) {}
  template <typename... A> void printf(const char*, A...) {}
  uint16_t width() { return T::HEIGHT; } uint16_t height() { return T::WIDTH; }
};

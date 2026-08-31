#pragma once
#include <cstdint>
class Adafruit_SSD1306 {
 public:
  template <typename... A> Adafruit_SSD1306(A...) {}
  bool begin(int,int) { return true; }
  void clearDisplay() {} void display() {}
  void drawBitmap(int,int,const uint8_t*,int,int,uint16_t) {}
  void setRotation(uint8_t) {}
};
#define SSD1306_SWITCHCAPVCC 0
#define SSD1306_BLACK 0
#define SSD1306_WHITE 1

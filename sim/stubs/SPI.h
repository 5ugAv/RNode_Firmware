#pragma once
#include <cstdint>
class SPISettings { public: SPISettings(){} SPISettings(uint32_t,uint8_t,uint8_t){} };
class SPIClass {
 public:
  SPIClass() {}
  template <typename... A> SPIClass(A...) {}
  void begin() {} void begin(int,int,int,int) {} void end() {}
  void beginTransaction(SPISettings) {} void endTransaction() {}
  uint8_t transfer(uint8_t v) { return v; }
};
extern SPIClass SPI;
#define MSBFIRST 1
#define SPI_MODE0 0

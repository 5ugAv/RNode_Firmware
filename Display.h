// Copyright (C) 2024, Mark Qvist

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "Graphics.h"
#include <Adafruit_GFX.h>

#if BOARD_MODEL != BOARD_TECHO
  #if BOARD_MODEL == BOARD_TDECK
    #include <Adafruit_ST7789.h>
  #elif BOARD_MODEL == BOARD_HELTEC_T114
    #include "ST7789.h"
    #define COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
  #elif BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
    #include <Adafruit_ST7735.h>
  #elif BOARD_MODEL == BOARD_TBEAM_S_V1
    #include <Adafruit_SH110X.h>
  #else
    #include <Wire.h>
    #include <Adafruit_SSD1306.h>
  #endif

#else
  void (*display_callback)();
  void display_add_callback(void (*callback)()) { display_callback = callback; }
  void busyCallback(const void* p) { display_callback(); }
  #define SSD1306_BLACK GxEPD_BLACK
  #define SSD1306_WHITE GxEPD_WHITE
  #include <GxEPD2_BW.h>
  #include <SPI.h>
#endif

#include "Fonts/Org_01.h"
#define DISP_W 128
#define DISP_H 64

#if BOARD_MODEL == BOARD_RNODE_NG_20 || BOARD_MODEL == BOARD_LORA32_V2_0
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
#elif BOARD_MODEL == BOARD_TBEAM
  #define DISP_RST 13
  #define DISP_ADDR 0x3C
  #define DISP_CUSTOM_ADDR true
#elif BOARD_MODEL == BOARD_HELTEC32_V2 || BOARD_MODEL == BOARD_LORA32_V1_0
  #define DISP_RST 16
  #define DISP_ADDR 0x3C
  #define SCL_OLED 15
  #define SDA_OLED 4
#elif BOARD_MODEL == BOARD_HELTEC32_V3
  #define DISP_RST 21
  #define DISP_ADDR 0x3C
  #define SCL_OLED 18
  #define SDA_OLED 17
#elif BOARD_MODEL == BOARD_HELTEC32_V4
  #define DISP_RST 21
  #define DISP_ADDR 0x3C
  #define SCL_OLED 18
  #define SDA_OLED 17
#elif BOARD_MODEL == BOARD_RAK4631
  // RAK1921/SSD1306
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 14
  #define SDA_OLED 13
#elif BOARD_MODEL == BOARD_RNODE_NG_21
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
#elif BOARD_MODEL == BOARD_T3S3
  #define DISP_RST 21
  #define DISP_ADDR 0x3C
  #define SCL_OLED 17
  #define SDA_OLED 18
#elif BOARD_MODEL == BOARD_TECHO
  SPIClass displaySPI = SPIClass(NRF_SPIM0, pin_disp_miso, pin_disp_sck, pin_disp_mosi);
  #define DISP_W 128
  #define DISP_H 64
  #define DISP_ADDR -1
#elif BOARD_MODEL == BOARD_TBEAM_S_V1
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 18
  #define SDA_OLED 17
  #define DISP_CUSTOM_ADDR false
#elif BOARD_MODEL == BOARD_XIAO_S3
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define SCL_OLED 6
  #define SDA_OLED 5
  #define DISP_CUSTOM_ADDR true
#elif BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
  #define DISP_RST DISPLAY_RST
  #define DISP_ADDR -1
#else
  #define DISP_RST -1
  #define DISP_ADDR 0x3C
  #define DISP_CUSTOM_ADDR true
#endif

#define SMALL_FONT &Org_01

#if BOARD_MODEL == BOARD_TDECK
  Adafruit_ST7789 display = Adafruit_ST7789(DISPLAY_CS, DISPLAY_DC, -1);
  #define SSD1306_WHITE ST77XX_WHITE
  #define SSD1306_BLACK ST77XX_BLACK
#elif BOARD_MODEL == BOARD_HELTEC_T114
  ST7789Spi display(&SPI1, DISPLAY_RST, DISPLAY_DC, DISPLAY_CS);
  #define SSD1306_WHITE ST77XX_WHITE
  #define SSD1306_BLACK ST77XX_BLACK
#elif BOARD_MODEL == BOARD_TBEAM_S_V1
  Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);
  #define SSD1306_WHITE SH110X_WHITE
  #define SSD1306_BLACK SH110X_BLACK
#elif BOARD_MODEL == BOARD_TECHO
  GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(pin_disp_cs, pin_disp_dc, pin_disp_reset, pin_disp_busy));
  uint32_t last_epd_refresh = 0;
  uint32_t last_epd_full_refresh = 0;
  #define REFRESH_PERIOD 300000
#elif BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
  SPIClass hspi = SPIClass(HSPI);
  Adafruit_ST7735 display = Adafruit_ST7735(&hspi, DISPLAY_CS, DISPLAY_DC, DISPLAY_RST);
  #define SSD1306_WHITE 0xFFFF
  #define SSD1306_BLACK 0x0000
#else
  Adafruit_SSD1306 display(DISP_W, DISP_H, &Wire, DISP_RST);
#endif

float disp_target_fps = 7;
float epd_update_fps  = 0.5;

#define DISP_MODE_UNKNOWN   0x00
#define DISP_MODE_LANDSCAPE 0x01
#define DISP_MODE_PORTRAIT  0x02
#define DISP_PIN_SIZE   6
#define DISPLAY_BLANKING_TIMEOUT 15*1000
uint8_t disp_mode = DISP_MODE_UNKNOWN;
uint8_t disp_ext_fb = false;
unsigned char fb[512];
uint32_t last_disp_update = 0;
uint32_t last_unblank_event = 0;
uint32_t display_blanking_timeout = DISPLAY_BLANKING_TIMEOUT;
uint8_t display_unblank_intensity = display_intensity;
bool display_blanked = false;
bool display_tx = false;
bool recondition_display = false;
int disp_update_interval = 1000/disp_target_fps;
int epd_update_interval = 1000/disp_target_fps;
uint32_t last_page_flip = 0;
int page_interval = 4000;
bool device_signatures_ok();
bool device_firmware_ok();

#define WATERFALL_SIZE 46
int waterfall[WATERFALL_SIZE];
int waterfall_meta[WATERFALL_SIZE];
int waterfall_head = 0;

int p_ad_x = 0;
int p_ad_y = 0;
int p_as_x = 0;
int p_as_y = 0;

GFXcanvas1 stat_area(64, 64);
GFXcanvas1 disp_area(64, 64);

static const uint8_t one_counts[256] = {
  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  1,  1,  1,  1,
  1,  1,  1,  1,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,
  0,  0,  0,  0,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  2,  3,
  2,  2,  2,  2,  2,  2,  2,  2,  1,  2,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,
  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0
};

void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour);

void update_area_positions() {
  #if BOARD_MODEL == BOARD_HELTEC_T114
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 16;
      p_ad_y = 64;
      p_as_x = 16;
      p_as_y = p_ad_y+126;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0;
      p_ad_y = 96;
      p_as_x = 126;
      p_as_y = p_ad_y;
    }
  #elif BOARD_MODEL == BOARD_TECHO
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 61;
      p_ad_y = 36;
      p_as_x = 64;
      p_as_y = 64+36;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0;
      p_ad_y = 0;
      p_as_x = 64;
      p_as_y = 0;
    }
  #else
    if (disp_mode == DISP_MODE_PORTRAIT) {
      p_ad_x = 0 * DISPLAY_SCALE;
      p_ad_y = 0 * DISPLAY_SCALE;
      p_as_x = 0 * DISPLAY_SCALE;
      p_as_y = 64 * DISPLAY_SCALE;
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      p_ad_x = 0 * DISPLAY_SCALE;
      p_ad_y = 0 * DISPLAY_SCALE;
      p_as_x = 64 * DISPLAY_SCALE;
      p_as_y = 0 * DISPLAY_SCALE;
    }
  #endif
}

uint8_t display_contrast = 0x00;
#if BOARD_MODEL == BOARD_TBEAM_S_V1
  void set_contrast(Adafruit_SH1106G *display, uint8_t value) {
  }
#elif BOARD_MODEL == BOARD_HELTEC_T114
  void set_contrast(ST7789Spi *display, uint8_t value) { }
#elif BOARD_MODEL == BOARD_TECHO
  void set_contrast(void *display, uint8_t value) {
    if (value == 0) { analogWrite(pin_backlight, 0); }
    else            { analogWrite(pin_backlight, value); }
  }
#elif BOARD_MODEL == BOARD_TDECK
  void set_contrast(Adafruit_ST7789 *display, uint8_t value) {
    static uint8_t level = 0;
    static uint8_t steps = 16;
    if (value > 15) value = 15;
    if (value == 0) {
        digitalWrite(DISPLAY_BL_PIN, 0);
        delay(3);
        level = 0;
        return;
    }
    if (level == 0) {
        digitalWrite(DISPLAY_BL_PIN, 1);
        level = steps;
        delayMicroseconds(30);
    }
    int from = steps - level;
    int to = steps - value;
    int num = (steps + to - from) % steps;
    for (int i = 0; i < num; i++) {
        digitalWrite(DISPLAY_BL_PIN, 0);
        digitalWrite(DISPLAY_BL_PIN, 1);
    }
    level = value;
  }
#elif BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
  void set_contrast(Adafruit_ST7735 *display, uint8_t contrast) { }
#else
  void set_contrast(Adafruit_SSD1306 *display, uint8_t contrast) {
    display->ssd1306_command(SSD1306_SETCONTRAST);
    display->ssd1306_command(contrast);
  }
#endif

bool display_init() {
  #if HAS_DISPLAY
    #if BOARD_MODEL == BOARD_RNODE_NG_20 || BOARD_MODEL == BOARD_LORA32_V2_0
      int pin_display_en = 16;
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
    #elif BOARD_MODEL == BOARD_T3S3
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC32_V2
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC32_V3
      // enable vext / pin 36
      pinMode(Vext, OUTPUT);
      digitalWrite(Vext, LOW);
      delay(50);
      int pin_display_en = 21;
      pinMode(pin_display_en, OUTPUT);
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
      delay(50);
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC32_V4
      // enable vext / pin 36
      pinMode(Vext, OUTPUT);
      digitalWrite(Vext, LOW);
      delay(50);
      int pin_display_en = 21;
      pinMode(pin_display_en, OUTPUT);
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
      delay(50);
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_LORA32_V1_0
      int pin_display_en = 16;
      digitalWrite(pin_display_en, LOW);
      delay(50);
      digitalWrite(pin_display_en, HIGH);
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_HELTEC_T114
      pinMode(PIN_T114_TFT_EN, OUTPUT);
      digitalWrite(PIN_T114_TFT_EN, LOW);
    #elif BOARD_MODEL == BOARD_TECHO
      display.init(0, true, 10, false, displaySPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
      display.setPartialWindow(0, 0, DISP_W, DISP_H);
      display.epd2.setBusyCallback(busyCallback);
      #if HAS_BACKLIGHT
        pinMode(pin_backlight, OUTPUT);
        analogWrite(pin_backlight, 0);
      #endif
    #elif BOARD_MODEL == BOARD_TBEAM_S_V1
      Wire.begin(SDA_OLED, SCL_OLED);
    #elif BOARD_MODEL == BOARD_XIAO_S3
      Wire.begin(SDA_OLED, SCL_OLED);
    #endif

    #if HAS_EEPROM
      uint8_t display_rotation = EEPROM.read(eeprom_addr(ADDR_CONF_DROT));
    #elif MCU_VARIANT == MCU_NRF52
      uint8_t display_rotation = eeprom_read(eeprom_addr(ADDR_CONF_DROT));
    #endif
    if (display_rotation < 0 or display_rotation > 3) display_rotation = 0xFF;

    #if DISP_CUSTOM_ADDR == true
      #if HAS_EEPROM
        uint8_t display_address = EEPROM.read(eeprom_addr(ADDR_CONF_DADR));
      #elif MCU_VARIANT == MCU_NRF52
        uint8_t display_address = eeprom_read(eeprom_addr(ADDR_CONF_DADR));
      #endif
      if (display_address == 0xFF) display_address = DISP_ADDR;
    #else
      uint8_t display_address = DISP_ADDR;
    #endif

    #if HAS_EEPROM
      if (EEPROM.read(eeprom_addr(ADDR_CONF_BSET)) == CONF_OK_BYTE) {
        uint8_t db_timeout = EEPROM.read(eeprom_addr(ADDR_CONF_DBLK));
        if (db_timeout == 0x00) {
          display_blanking_enabled = false;
        } else {
          display_blanking_enabled = true;
          display_blanking_timeout = db_timeout*1000;
        }
      }
    #elif MCU_VARIANT == MCU_NRF52
      if (eeprom_read(eeprom_addr(ADDR_CONF_BSET)) == CONF_OK_BYTE) {
        uint8_t db_timeout = eeprom_read(eeprom_addr(ADDR_CONF_DBLK));
        if (db_timeout == 0x00) {
          display_blanking_enabled = false;
        } else {
          display_blanking_enabled = true;
          display_blanking_timeout = db_timeout*1000;
        }
      }
    #endif
    
    #if BOARD_MODEL == BOARD_TECHO
    // Don't check if display is actually connected
    if(false) {
    #elif BOARD_MODEL == BOARD_TDECK
    display.init(240, 320);
    display.setSPISpeed(80e6);
    #elif BOARD_MODEL == BOARD_HELTEC_T114
    display.init();
    // set white as default pixel colour for Heltec T114
    display.setRGB(COLOR565(0xFF, 0xFF, 0xFF));
    if (false) {
    #elif BOARD_MODEL == BOARD_TBEAM_S_V1
    if (!display.begin(display_address, true)) {
    #elif BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
    pinMode(DISPLAY_POWER_PIN, OUTPUT);
    digitalWrite(DISPLAY_POWER_PIN, HIGH);
    pinMode(DISPLAY_BL_PIN, OUTPUT);
    digitalWrite(DISPLAY_BL_PIN, HIGH);
    hspi.begin(DISPLAY_CLK, -1, DISPLAY_MOSI, DISPLAY_CS);
    display.initR(INITR_MINI160x80_PLUGIN);
    display.setSPISpeed(20000000);
    // rotation 3 = 180 deg from the old landscape: the tracker rides in the
    // medic USB-side-up, so this way the face reads right on the machine it
    // lives in (operator, 2026-08-27: "flip the screen and make that
    // standard for its birth"). The stray setRotation(2) dead-write is gone.
    display.setRotation(3);
    display.fillScreen(SSD1306_BLACK);
    if (false) {
    #else
    if (!display.begin(SSD1306_SWITCHCAPVCC, display_address)) {
    #endif
      return false;
    } else {
      set_contrast(&display, display_contrast);
      if (display_rotation != 0xFF) {
        if (display_rotation == 0 || display_rotation == 2) {
          disp_mode = DISP_MODE_LANDSCAPE;
        } else {
          disp_mode = DISP_MODE_PORTRAIT;
        }
        display.setRotation(display_rotation);
      } else {
        #if BOARD_MODEL == BOARD_RNODE_NG_20
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_RNODE_NG_21
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_LORA32_V1_0
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_LORA32_V2_0
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_LORA32_V2_1
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_TBEAM
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_TBEAM_S_V1
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC32_V2
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC32_V3
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC32_V4
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_HELTEC_T114
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(1);
        #elif BOARD_MODEL == BOARD_RAK4631
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(0);
        #elif BOARD_MODEL == BOARD_TDECK
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_TECHO
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #elif BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
          disp_mode = DISP_MODE_LANDSCAPE;
          display.setRotation(3);   // 180 deg: standard tracker-birth pose
        #else
          disp_mode = DISP_MODE_PORTRAIT;
          display.setRotation(3);
        #endif
      }

      update_area_positions();

      for (int i = 0; i < WATERFALL_SIZE; i++) { waterfall[i] = 0; }

      last_page_flip = millis();

      stat_area.cp437(true);
      disp_area.cp437(true);

      #if BOARD_MODEL != BOARD_HELTEC_T114
      display.cp437(true);
      #endif

      #if HAS_EEPROM
        display_intensity = EEPROM.read(eeprom_addr(ADDR_CONF_DINT));
      #elif MCU_VARIANT == MCU_NRF52
        display_intensity = eeprom_read(eeprom_addr(ADDR_CONF_DINT));
      #endif
      display_unblank_intensity = display_intensity;

      #if BOARD_MODEL == BOARD_TECHO
        #if HAS_BACKLIGHT
          if (display_intensity == 0) { analogWrite(pin_backlight, 0); }
          else                        { analogWrite(pin_backlight, display_intensity); }
        #endif
      #endif

      #if BOARD_MODEL == BOARD_TDECK
        display.fillScreen(SSD1306_BLACK);
      #endif

      #if BOARD_MODEL == BOARD_HELTEC_T114
        // Enable backlight led (display is always black without this)
        fillRect(p_ad_x, p_ad_y, 128, 128, SSD1306_BLACK);
        fillRect(p_as_x, p_as_y, 128, 128, SSD1306_BLACK);
        pinMode(PIN_T114_TFT_BLGT, OUTPUT);
        digitalWrite(PIN_T114_TFT_BLGT, LOW);
      #endif

      return true;
    }
  #else
    return false;
  #endif
}

// Draws a line on the screen
void drawLine(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour) {
  #if BOARD_MODEL == BOARD_HELTEC_T114
  if(colour == SSD1306_WHITE){
    display.setColor(WHITE);
  } else if(colour == SSD1306_BLACK) {
    display.setColor(BLACK);
  }
  display.drawLine(x, y, width, height);
  #else
  display.drawLine(x, y, width, height, colour);
  #endif
}

// Draws a filled rectangle on the screen
void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t colour) {
  #if BOARD_MODEL == BOARD_HELTEC_T114
  if(colour == SSD1306_WHITE){
    display.setColor(WHITE);
  } else if(colour == SSD1306_BLACK) {
    display.setColor(BLACK);
  }
  display.fillRect(x, y, width, height);
  #else
  display.fillRect(x, y, width, height, colour);
  #endif
}

// Draws a bitmap to the display and auto scales it based on the boards configured DISPLAY_SCALE
void drawBitmap(int16_t startX, int16_t startY, const uint8_t* bitmap, int16_t bitmapWidth, int16_t bitmapHeight, uint16_t foregroundColour, uint16_t backgroundColour) {
  #if DISPLAY_SCALE == 1
    display.drawBitmap(startX, startY, bitmap, bitmapWidth, bitmapHeight, foregroundColour, backgroundColour);
  #else
    for(int16_t row = 0; row < bitmapHeight; row++){
        for(int16_t col = 0; col < bitmapWidth; col++){

            // determine index and bitmask
            int16_t index = row * ((bitmapWidth + 7) / 8) + (col / 8);
            uint8_t bitmask = 1 << (7 - (col % 8));

            // check if the current pixel is set in the bitmap
            if(bitmap[index] & bitmask){
                // draw a scaled rectangle for the foreground pixel
                fillRect(startX + col * DISPLAY_SCALE, startY + row * DISPLAY_SCALE, DISPLAY_SCALE, DISPLAY_SCALE, foregroundColour);
            } else {
                // draw a scaled rectangle for the background pixel
                fillRect(startX + col * DISPLAY_SCALE, startY + row * DISPLAY_SCALE, DISPLAY_SCALE, DISPLAY_SCALE, backgroundColour);
            }

        }
    }
  #endif
}

extern uint8_t wifi_mode;
extern bool wifi_is_connected();
extern bool wifi_host_is_connected();
void draw_cable_icon(int px, int py) {
  #if HAS_WIFI
    if (wifi_mode == WR_WIFI_OFF) {
      if      (cable_state == CABLE_STATE_DISCONNECTED) { stat_area.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      else if (cable_state == CABLE_STATE_CONNECTED)    { stat_area.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
    } else {
      if (wifi_mode == WR_WIFI_STA) {
        if (wifi_is_connected()) {
          stat_area.drawBitmap(px, py, bm_wifi+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
          if (!wifi_host_is_connected()) { stat_area.fillRect(px+5, py+12, 6, 3, SSD1306_BLACK); }
        } else { stat_area.drawBitmap(px, py, bm_wifi+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      
      } else if (wifi_mode == WR_WIFI_AP) {
        if (wifi_host_is_connected()) { stat_area.drawBitmap(px, py, bm_wifi+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
        else                          { stat_area.drawBitmap(px, py, bm_wifi+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      
      } else {
        if      (cable_state == CABLE_STATE_DISCONNECTED) { stat_area.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
        else if (cable_state == CABLE_STATE_CONNECTED)    { stat_area.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
      }
    }

  #else
  if      (cable_state == CABLE_STATE_DISCONNECTED) { stat_area.drawBitmap(px, py, bm_cable+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
  else if (cable_state == CABLE_STATE_CONNECTED)    { stat_area.drawBitmap(px, py, bm_cable+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK); }
  #endif
}

void draw_bt_icon(int px, int py) {
  if (bt_state == BT_STATE_OFF) {
    stat_area.drawBitmap(px, py, bm_bt+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else if (bt_state == BT_STATE_ON) {
    stat_area.drawBitmap(px, py, bm_bt+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else if (bt_state == BT_STATE_PAIRING) {
    stat_area.drawBitmap(px, py, bm_bt+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else if (bt_state == BT_STATE_CONNECTED) {
    stat_area.drawBitmap(px, py, bm_bt+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    stat_area.drawBitmap(px, py, bm_bt+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

void draw_lora_icon(int px, int py) {
  if (radio_online) {
    stat_area.drawBitmap(px, py, bm_rf+1*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    stat_area.drawBitmap(px, py, bm_rf+0*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

void draw_mw_icon(int px, int py) {
  if (mw_radio_online) {
    stat_area.drawBitmap(px, py, bm_rf+3*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    stat_area.drawBitmap(px, py, bm_rf+2*32, 16, 16, SSD1306_WHITE, SSD1306_BLACK);
  }
}

uint8_t charge_tick = 0;
void draw_battery_bars(int px, int py) {
  if (pmu_ready) {
    if (battery_ready) {
      if (battery_installed) {
        float battery_value = battery_percent;

        // Disable charging state display for now, since
        // boards without dedicated PMU are completely
        // unreliable for determining actual charging state.
        bool disable_charge_status = false;
        if (battery_indeterminate && battery_state == BATTERY_STATE_CHARGING) {
          disable_charge_status = true;
        }
        
        if (battery_state == BATTERY_STATE_CHARGING && !disable_charge_status) {
          float battery_prog = battery_percent;
          if (battery_prog > 85) { battery_prog = 84; }
          if (charge_tick < battery_prog ) { charge_tick = battery_prog; }
          battery_value = charge_tick;
          charge_tick += 3;
          if (charge_tick > 100) charge_tick = 0;
        }

        if (battery_indeterminate && battery_state == BATTERY_STATE_CHARGING && !disable_charge_status) {
          stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
          stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
        } else {
          if (battery_state == BATTERY_STATE_CHARGED) {
            stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
            stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
          } else {
            // stat_area.fillRect(px, py, 14, 3, SSD1306_BLACK);
            stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
            stat_area.drawRect(px-2, py-2, 17, 7, SSD1306_WHITE);
            stat_area.drawLine(px+15, py, px+15, py+3, SSD1306_WHITE);
            if (battery_value > 7) stat_area.drawLine(px, py, px, py+2, SSD1306_WHITE);
            if (battery_value > 20) stat_area.drawLine(px+1*2, py, px+1*2, py+2, SSD1306_WHITE);
            if (battery_value > 33) stat_area.drawLine(px+2*2, py, px+2*2, py+2, SSD1306_WHITE);
            if (battery_value > 46) stat_area.drawLine(px+3*2, py, px+3*2, py+2, SSD1306_WHITE);
            if (battery_value > 59) stat_area.drawLine(px+4*2, py, px+4*2, py+2, SSD1306_WHITE);
            if (battery_value > 72) stat_area.drawLine(px+5*2, py, px+5*2, py+2, SSD1306_WHITE);
            if (battery_value > 85) stat_area.drawLine(px+6*2, py, px+6*2, py+2, SSD1306_WHITE);
          }
        }
      } else {
        stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
        stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
      }
    }
  } else {
    stat_area.fillRect(px-2, py-2, 18, 7, SSD1306_BLACK);
    stat_area.drawBitmap(px-2, py-2, bm_plug, 17, 7, SSD1306_WHITE, SSD1306_BLACK);
  }
}

#define Q_SNR_STEP 2.0
#define Q_SNR_MIN_BASE -9.0
#define Q_SNR_MAX 6.0
void draw_quality_bars(int px, int py) {
  stat_area.fillRect(px, py, 13, 7, SSD1306_BLACK);
  if (radio_online) {
    signed char t_snr = (signed int)last_snr_raw;
    int snr_int = (int)t_snr;
    float snr_min = Q_SNR_MIN_BASE-(int)lora_sf*Q_SNR_STEP;
    float snr_span = (Q_SNR_MAX-snr_min);
    float snr = ((int)snr_int) * 0.25;
    float quality = ((snr-snr_min)/(snr_span))*100;
    if (quality > 100.0) quality = 100.0;
    if (quality < 0.0) quality = 0.0;

    // Serial.printf("Last SNR: %.2f\n, quality: %.2f\n", snr, quality);
    if (quality > 0)  stat_area.drawLine(px+0*2, py+7, px+0*2, py+6, SSD1306_WHITE);
    if (quality > 15) stat_area.drawLine(px+1*2, py+7, px+1*2, py+5, SSD1306_WHITE);
    if (quality > 30) stat_area.drawLine(px+2*2, py+7, px+2*2, py+4, SSD1306_WHITE);
    if (quality > 45) stat_area.drawLine(px+3*2, py+7, px+3*2, py+3, SSD1306_WHITE);
    if (quality > 60) stat_area.drawLine(px+4*2, py+7, px+4*2, py+2, SSD1306_WHITE);
    if (quality > 75) stat_area.drawLine(px+5*2, py+7, px+5*2, py+1, SSD1306_WHITE);
    if (quality > 90) stat_area.drawLine(px+6*2, py+7, px+6*2, py+0, SSD1306_WHITE);
  }
}

#if MODEM == SX1280
  #define S_RSSI_MIN -105.0
  #define S_RSSI_MAX -65.0
#else
  #define S_RSSI_MIN -135.0
  #define S_RSSI_MAX -75.0
#endif
#define S_RSSI_SPAN (S_RSSI_MAX-S_RSSI_MIN)
void draw_signal_bars(int px, int py) {
  stat_area.fillRect(px, py, 13, 7, SSD1306_BLACK);

  if (radio_online) {
    int rssi_val = last_rssi;
    if (rssi_val < S_RSSI_MIN) rssi_val = S_RSSI_MIN;
    if (rssi_val > S_RSSI_MAX) rssi_val = S_RSSI_MAX;
    int signal = ((rssi_val - S_RSSI_MIN)*(1.0/S_RSSI_SPAN))*100.0;

    if (signal > 100.0) signal = 100.0;
    if (signal < 0.0) signal = 0.0;

    // Serial.printf("Last SNR: %.2f\n, quality: %.2f\n", snr, quality);
    if (signal > 85) stat_area.drawLine(px+0*2, py+7, px+0*2, py+0, SSD1306_WHITE);
    if (signal > 72) stat_area.drawLine(px+1*2, py+7, px+1*2, py+1, SSD1306_WHITE);
    if (signal > 59) stat_area.drawLine(px+2*2, py+7, px+2*2, py+2, SSD1306_WHITE);
    if (signal > 46) stat_area.drawLine(px+3*2, py+7, px+3*2, py+3, SSD1306_WHITE);
    if (signal > 33) stat_area.drawLine(px+4*2, py+7, px+4*2, py+4, SSD1306_WHITE);
    if (signal > 20) stat_area.drawLine(px+5*2, py+7, px+5*2, py+5, SSD1306_WHITE);
    if (signal > 7)  stat_area.drawLine(px+6*2, py+7, px+6*2, py+6, SSD1306_WHITE);
  }
}

#if MODEM == SX1280
  #define WF_TX_SIZE 5
#else
  #define WF_TX_SIZE 5
#endif
#define WF_RSSI_MAX -60
#define WF_RSSI_MIN -135
#define WF_RSSI_SPAN (WF_RSSI_MAX-WF_RSSI_MIN)
#define WF_PIXEL_WIDTH 10
#define WF_M_RX   0x00
#define WF_M_TX   0x01
#define WF_M_NTFR 0x02
void draw_waterfall(int px, int py) {
  int rssi_val = current_rssi;
  if (rssi_val < WF_RSSI_MIN) rssi_val = WF_RSSI_MIN;
  if (rssi_val > WF_RSSI_MAX) rssi_val = WF_RSSI_MAX;
  int rssi_normalised = ((rssi_val - WF_RSSI_MIN)*(1.0/WF_RSSI_SPAN))*WF_PIXEL_WIDTH;
  if (display_tx) {
    for (uint8_t i = 0; i < WF_TX_SIZE; i++) {
      waterfall_meta[waterfall_head] = WF_M_TX;
      waterfall[waterfall_head++] = -1;
      if (waterfall_head >= WATERFALL_SIZE) waterfall_head = 0;
    }
    display_tx = false;
  } else {
    if (interference_detected) { waterfall_meta[waterfall_head] = WF_M_NTFR; }
    else                       { waterfall_meta[waterfall_head] = WF_M_RX; }
    waterfall[waterfall_head++] = rssi_normalised;
    if (waterfall_head >= WATERFALL_SIZE) waterfall_head = 0;
  }

  stat_area.fillRect(px,py,WF_PIXEL_WIDTH, WATERFALL_SIZE, SSD1306_BLACK);
  for (int i = 0; i < WATERFALL_SIZE; i++){
    int wi = (waterfall_head+i)%WATERFALL_SIZE;
    int ws = waterfall[wi];
    int wm = waterfall_meta[wi];
    if (ws > 0) {
      if      (wm == WF_M_RX)   { stat_area.drawLine(px, py+i, px+ws-1, py+i, SSD1306_WHITE); }
      else if (wm == WF_M_NTFR) {
        uint8_t o = 0;
        for (uint8_t ti = 0; ti < WF_PIXEL_WIDTH/2; ti++) { stat_area.drawPixel(px+ti*2+o, py+i, SSD1306_WHITE); }
      }
    } else if (ws == -1) {
      uint8_t o = i%2;
      for (uint8_t ti = 0; ti < WF_PIXEL_WIDTH/2; ti++) {
        stat_area.drawPixel(px+ti*2+o, py+i, SSD1306_WHITE);
      }
    }
  }
}

bool stat_area_intialised = false;
void draw_stat_area() {
  if (device_init_done) {
    if (!stat_area_intialised) {
      stat_area.drawBitmap(0, 0, bm_frame, 64, 64, SSD1306_WHITE, SSD1306_BLACK);
      stat_area_intialised = true;
    }

    draw_cable_icon(3, 8);
    draw_bt_icon(3, 30);
    draw_lora_icon(45, 8);
    draw_mw_icon(45, 30);
    draw_battery_bars(4, 58);
    draw_quality_bars(28, 56);
    draw_signal_bars(44, 56);
    if (radio_online) {
      draw_waterfall(27, 4);
    }
  }
}

void update_stat_area() {
  if (eeprom_ok && !firmware_update_mode && !console_active) {

    draw_stat_area();
    if (disp_mode == DISP_MODE_PORTRAIT) {
      drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      drawBitmap(p_as_x+2, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
      if (device_init_done && !disp_ext_fb) drawLine(p_as_x, 0, p_as_x, 64, SSD1306_WHITE);
    }

  } else {
    if (firmware_update_mode) {
      drawBitmap(p_as_x, p_as_y, bm_updating, stat_area.width(), stat_area.height(), SSD1306_BLACK, SSD1306_WHITE);
    } else if (console_active && device_init_done) {
      drawBitmap(p_as_x, p_as_y, bm_console, stat_area.width(), stat_area.height(), SSD1306_BLACK, SSD1306_WHITE);
      if (disp_mode == DISP_MODE_LANDSCAPE) {
        drawLine(p_as_x, 0, p_as_x, 64, SSD1306_WHITE);
      }
    }
  }
}

#define START_PAGE 0
const uint8_t pages = 3;
uint8_t disp_page = START_PAGE;
extern char bt_devname[11];
extern char bt_dh[16];
#if HAS_WIFI
  extern IPAddress wr_device_ip;
#endif
void draw_disp_area() {
  if (!device_init_done || firmware_update_mode) {
    uint8_t p_by = 37;
    if (disp_mode == DISP_MODE_LANDSCAPE || firmware_update_mode) {
      p_by = 18;
      disp_area.fillRect(0, 0, disp_area.width(), disp_area.height(), SSD1306_BLACK);
    }
    if (!device_init_done) disp_area.drawBitmap(0, p_by, bm_boot, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
    if (firmware_update_mode) disp_area.drawBitmap(0, p_by, bm_fw_update, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
  } else {
    if (!disp_ext_fb or bt_ssp_pin != 0) {
      if (radio_online && display_diagnostics) {
        disp_area.fillRect(0,8,disp_area.width(),37, SSD1306_BLACK); disp_area.fillRect(0,37,disp_area.width(),27, SSD1306_WHITE);
        disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);

        disp_area.setCursor(2, 13);
        disp_area.print("On");
        disp_area.setCursor(14, 13);
        disp_area.print("@");
        disp_area.setCursor(21, 13);
        disp_area.printf("%.1fKbps", (float)lora_bitrate/1000.0);

        //disp_area.setCursor(31, 23-1);
        disp_area.setCursor(2, 23-1);
        disp_area.print("Airtime:");
        
        disp_area.setCursor(11, 33-1);
        if (total_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", total_channel_util*100.0);
          disp_area.printf("%.1f%%", airtime*100.0);
        } else {
          //disp_area.printf("%.0f%%", total_channel_util*100.0);
          disp_area.printf("%.0f%%", airtime*100.0);
        }
        disp_area.drawBitmap(2, 26-1, bm_hg_low, 5, 9, SSD1306_WHITE, SSD1306_BLACK);

        disp_area.setCursor(32+11, 33-1);
        if (longterm_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", longterm_channel_util*100.0);
          disp_area.printf("%.1f%%", longterm_airtime*100.0);
        } else {
          //disp_area.printf("%.0f%%", longterm_channel_util*100.0);
          disp_area.printf("%.0f%%", longterm_airtime*100.0);
        }
        disp_area.drawBitmap(32+2, 26-1, bm_hg_high, 5, 9, SSD1306_WHITE, SSD1306_BLACK);


        disp_area.setTextColor(SSD1306_BLACK);
        disp_area.setCursor(2, 46);
        disp_area.print("Channel");
        disp_area.setCursor(38, 46);
        disp_area.print("Load:");
        
        disp_area.setCursor(11, 57);
        if (total_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", airtime*100.0);
          disp_area.printf("%.1f%%", total_channel_util*100.0);
        } else {
          //disp_area.printf("%.0f%%", airtime*100.0);
          disp_area.printf("%.0f%%", total_channel_util*100.0);
        }
        disp_area.drawBitmap(2, 50, bm_hg_low, 5, 9, SSD1306_BLACK, SSD1306_WHITE);

        disp_area.setCursor(32+11, 57);
        if (longterm_channel_util < 0.099) {
          //disp_area.printf("%.1f%%", longterm_airtime*100.0);
          disp_area.printf("%.1f%%", longterm_channel_util*100.0);
        } else {
          //disp_area.printf("%.0f%%", longterm_airtime*100.0);
          disp_area.printf("%.0f%%", longterm_channel_util*100.0);
        }
        disp_area.drawBitmap(32+2, 50, bm_hg_high, 5, 9, SSD1306_BLACK, SSD1306_WHITE);

      } else {
        if (device_signatures_ok()) { disp_area.drawBitmap(0, 0, bm_def_lc, disp_area.width(), 23, SSD1306_WHITE, SSD1306_BLACK); }
        else {                        disp_area.drawBitmap(0, 0, bm_def,    disp_area.width(), 23, SSD1306_WHITE, SSD1306_BLACK); }

        bool display_ip = false;
        #if HAS_WIFI
          if (wifi_is_connected() && disp_page%2 == 1) { display_ip = true; }
        #endif
        if (display_ip) {
          #if HAS_WIFI
            uint8_t ones = 3+one_counts[wr_device_ip[0]]+one_counts[wr_device_ip[1]]+one_counts[wr_device_ip[2]]+one_counts[wr_device_ip[3]];
            uint8_t chars = 7;
            for (uint8_t i = 0; i<4; i++) { if (wr_device_ip[i] > 9) { chars++; } if (wr_device_ip[i] > 99) { chars++; } }
            uint8_t width = chars*6-(ones*4);
            int alignment_offset = disp_area.width()-width;
            int ipxpos = alignment_offset;
            disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(1);
            disp_area.fillRect(0, 20, disp_area.width(), 17, SSD1306_BLACK);
            disp_area.setCursor(3, 34-8); disp_area.print("WiFi IP:");
            disp_area.setCursor(ipxpos, 34); disp_area.print(wr_device_ip);
          #endif
        } else {
          disp_area.setFont(SMALL_FONT); disp_area.setTextWrap(false); disp_area.setTextColor(SSD1306_WHITE); disp_area.setTextSize(2);
          disp_area.fillRect(0, 20, disp_area.width(), 17, SSD1306_BLACK); uint8_t ofsc = 0;
          if ((bt_dh[14] & 0b00001111) == 0x01) { ofsc += 8; }
          if ((bt_dh[14] >> 4)         == 0x01) { ofsc += 8; }
          if ((bt_dh[15] & 0b00001111) == 0x01) { ofsc += 8; }
          if ((bt_dh[15] >> 4)         == 0x01) { ofsc += 8; }
          disp_area.setCursor(17+ofsc, 32); disp_area.printf("%02X%02X", bt_dh[14], bt_dh[15]);
        }
      }

      if (!hw_ready || radio_error || !device_firmware_ok()) {
        if (!device_firmware_ok()) {
          disp_area.drawBitmap(0, 37, bm_fw_corrupt, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
        } else {
          if (!modem_installed) {
            disp_area.drawBitmap(0, 37, bm_no_radio, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
          } else {
            disp_area.drawBitmap(0, 37, bm_conf_missing, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
          }
        }
      } else if (bt_state == BT_STATE_PAIRING and bt_ssp_pin != 0) {
        char *pin_str = (char*)malloc(DISP_PIN_SIZE+1);
        sprintf(pin_str, "%06d", bt_ssp_pin);

        disp_area.drawBitmap(0, 37, bm_pairing, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
        for (int i = 0; i < DISP_PIN_SIZE; i++) {
          uint8_t numeric = pin_str[i]-48;
          uint8_t offset = numeric*5;
          disp_area.drawBitmap(7+9*i, 37+16, bm_n_uh+offset, 8, 5, SSD1306_WHITE, SSD1306_BLACK);
        }
        free(pin_str);
      } else {
        if (millis()-last_page_flip >= page_interval) {
          disp_page = (++disp_page%pages);
          last_page_flip = millis();
          if (not community_fw and disp_page == 0) disp_page = 1;
        }

        if (radio_online) {
          if (!display_diagnostics) {
            disp_area.drawBitmap(0, 37, bm_online, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
          }
        } else {
          if (disp_page == 0) {
            if (true || device_signatures_ok()) {
              disp_area.drawBitmap(0, 37, bm_checks, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            } else {
              disp_area.drawBitmap(0, 37, bm_nfr, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            }
          } else if (disp_page == 1) {
            if (!console_active) {
              disp_area.drawBitmap(0, 37, bm_hwok, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            } else {
              disp_area.drawBitmap(0, 37, bm_console_active, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            }
          } else if (disp_page == 2) {
            disp_area.drawBitmap(0, 37, bm_version, disp_area.width(), 27, SSD1306_WHITE, SSD1306_BLACK);
            char *v_str = (char*)malloc(3+1);
            sprintf(v_str, "%01d%02d", MAJ_VERS, MIN_VERS);
            for (int i = 0; i < 3; i++) {
              uint8_t numeric = v_str[i]-48; uint8_t bm_offset = numeric*5;
              uint8_t dxp = 20;
              if (i == 1) dxp += 9*1+4;
              if (i == 2) dxp += 9*2+4;
              disp_area.drawBitmap(dxp, 37+16, bm_n_uh+bm_offset, 8, 5, SSD1306_WHITE, SSD1306_BLACK);
            }
            free(v_str);
            disp_area.drawLine(27, 37+19, 28, 37+19, SSD1306_BLACK);
            disp_area.drawLine(27, 37+20, 28, 37+20, SSD1306_BLACK);
          }
        }
      }
    } else {
      disp_area.drawBitmap(0, 0, fb, disp_area.width(), disp_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    }
  }
}

void update_disp_area() {
  draw_disp_area();

  drawBitmap(p_ad_x, p_ad_y, disp_area.getBuffer(), disp_area.width(), disp_area.height(), SSD1306_WHITE, SSD1306_BLACK);
  if (disp_mode == DISP_MODE_LANDSCAPE) {
    if (device_init_done && !firmware_update_mode && !disp_ext_fb) {
      drawLine(0, 0, 0, 63, SSD1306_WHITE);
    }
  }
}

void display_recondition() {
  #if PLATFORM == PLATFORM_ESP32
    for (uint8_t iy = 0; iy < disp_area.height(); iy++) {
      unsigned char rand_seg [] = {random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF),random(0xFF)};
      stat_area.drawBitmap(0, iy, rand_seg, 64, 1, SSD1306_WHITE, SSD1306_BLACK);
      disp_area.drawBitmap(0, iy, rand_seg, 64, 1, SSD1306_WHITE, SSD1306_BLACK);
    }

    drawBitmap(p_ad_x, p_ad_y, disp_area.getBuffer(), disp_area.width(), disp_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    if (disp_mode == DISP_MODE_PORTRAIT) {
      drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    } else if (disp_mode == DISP_MODE_LANDSCAPE) {
      drawBitmap(p_as_x, p_as_y, stat_area.getBuffer(), stat_area.width(), stat_area.height(), SSD1306_WHITE, SSD1306_BLACK);
    }
  #endif
}

bool epd_blanked = false;
#if BOARD_MODEL == BOARD_TECHO
  void epd_blank(bool full_update = true) {
    display.setFullWindow();
    display.fillScreen(SSD1306_WHITE);
    display.display(full_update);
  }

  void epd_black(bool full_update = true) {
    display.setFullWindow();
    display.fillScreen(SSD1306_BLACK);
    display.display(full_update);
  }
#endif

void update_display(bool blank = false) {
  #if BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
    return; // Tracker: tracker_status_burst() owns the screen
  #endif
  display_updating = true;
  if (blank == true) {
    last_disp_update = millis()-disp_update_interval-1;
  } else {
    if (display_blanking_enabled && millis()-last_unblank_event >= display_blanking_timeout) {
      blank = true;
      display_blanked = true;
      if (display_intensity != 0) {
        display_unblank_intensity = display_intensity;
      }
      display_intensity = 0;
    } else {
      display_blanked = false;
      if (display_unblank_intensity != 0x00) {
        display_intensity = display_unblank_intensity;
        display_unblank_intensity = 0x00;
      }
    }
  }

  if (blank) {
    if (millis()-last_disp_update >= disp_update_interval) {
      if (display_contrast != display_intensity) {
        display_contrast = display_intensity;
        set_contrast(&display, display_contrast);
      }

      #if BOARD_MODEL == BOARD_TECHO
        if (!epd_blanked) {
          epd_blank();
          epd_blanked = true;
        }
      #endif

      #if BOARD_MODEL == BOARD_HELTEC_T114
        display.clear();
        display.display();
        digitalWrite(PIN_T114_TFT_BLGT, HIGH);
      #elif BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
        display.fillScreen(SSD1306_BLACK);
      #elif BOARD_MODEL != BOARD_TDECK && BOARD_MODEL != BOARD_TECHO
        display.clearDisplay();
        display.display();
      #else
        // TODO: Clear screen
      #endif

      last_disp_update = millis();
    }

  } else {
    if (millis()-last_disp_update >= disp_update_interval) {
      uint32_t current = millis();
      if (display_contrast != display_intensity) {
        display_contrast = display_intensity;
        set_contrast(&display, display_contrast);
      }

      #if BOARD_MODEL == BOARD_HELTEC_T114
        display.clear();
        digitalWrite(PIN_T114_TFT_BLGT, LOW);
      #elif BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
        display.fillScreen(SSD1306_BLACK);
      #elif BOARD_MODEL != BOARD_TDECK && BOARD_MODEL != BOARD_TECHO
        display.clearDisplay();
      #endif

      if (recondition_display) {
        disp_target_fps = 30;
        disp_update_interval = 1000/disp_target_fps;
        display_recondition();
      } else {
        #if BOARD_MODEL == BOARD_TECHO
          display.setFullWindow();
          display.fillScreen(SSD1306_WHITE);
        #endif

        update_stat_area();
        update_disp_area();
      }
      
      #if BOARD_MODEL == BOARD_TECHO
        if (current-last_epd_refresh >= epd_update_interval) {
          if (current-last_epd_full_refresh >= REFRESH_PERIOD) { display.display(false); last_epd_full_refresh = millis(); }
          else { display.display(true); }
          last_epd_refresh = millis();
          epd_blanked = false;
        }
      #elif BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
        // ST7789 updates immediately, no display() needed
      #elif BOARD_MODEL != BOARD_TDECK
        display.display();
      #endif

      last_disp_update = millis();
    }
  }
  display_updating = false;
}

void display_unblank() {
  last_unblank_event = millis();
}

void ext_fb_enable() {
  disp_ext_fb = true;
}

void ext_fb_disable() {
  disp_ext_fb = false;
}

// ---- Heltec Wireless Tracker: directional status field (v4) ------------------
// TX=out(overshoot), RX=in(dense), interference=dense disorder, probe=ping train,
// error=explosion-in then a persistent flickering FAULT block, idle=subtle breath
// only while radio_online. Colour code untouched (reads npr/npg/npb).
#if BOARD_MODEL == BOARD_HELTEC_WIRELESS_TRACKER
#include <math.h>
extern uint8_t npr, npg, npb;
extern int last_rssi;
extern int current_rssi, noise_floor;
extern bool noise_floor_sampled;   // false until 128 samples collected / after an RF recal
#if HAS_GPS
// the GNSS object lives in the .ino (declared AFTER this header is pulled in
// via Utilities.h line 18, so a forward extern is required); TinyGPSPlus.h's
// include guard makes this double-include safe.
#include <TinyGPSPlus.h>
extern TinyGPSPlus gps;
#include "GpsPin.h"
#endif
extern bool radio_online;   // true once the host sends CMD_RADIO_STATE=on (Reticulum opened us)

// ---- tunables ---------------------------------------------------------------
#define TB_W             160
#define TB_H             80
#define TB_BAR_H         5                    // calibration bar height (bottom edge)
#define TB_FIELD_H       (TB_H - TB_BAR_H)    // 75: animation field, above the bar
#define TB_CX            80
#define TB_CY            (TB_FIELD_H / 2)      // 37: recentred to make room for the bar
#define TB_PARTS         180
#define TB_BLK           2
#define TB_CORE_R        4
// TB_MAX_R is derived from the field geometry at runtime (tb_maxr, set in tb_setup)
#define TB_OUT_MAX       135.0f  // TX overshoots well past the 89px screen corner
#define TB_KICK          26.0f
#define TB_SPRING        0.055f
#define TB_DAMP          0.87f
#define TB_SPD_POW       1.4f
#define TB_FRAME_MS      40
#define TB_RECOLOUR_FRAC 0.5f
#define TB_IN_FRAC       0.82f   // RX recruits more -> denser blue
#define TB_NOISE_FRAC    0.95f   // interference recruits nearly all -> dense
#define TB_FAULT_FRAC    0.88f
#define TB_COHORT_MS     850     // longer so the TX second pulse fully blooms
#define TB_IN_MS         1700    // RX lingers ~2x (match the V4 LED blue flash length)
#define TB_FAULT_MS      1500    // fault particle-cohort life (the explosion-in)
#define TB_FAULT_HOLD    1500    // base ms the fault block holds after forming
#define TB_FAULT_TAIL    300     // per-frame extension while the LED stays red
#define TB_MAX_COHORTS   3
#define TB_EDGE_FADE     60
#define TB_CORE_GAIN     1.4f
#define TB_AMP_TX        1.05f   // stronger TX drive so amber clears the edge
#define TB_TRAIL         5
#define TB_TRAIL_STEP    3.2f
#define TB_PROBE_K       0.80f
#define TB_PULSES        2       // TX/RX give two pulses per event
#define TB_PULSE_GAP     240     // ms between the two pulses
// interference -> disorder
#define TB_INTERF_MIN    11
#define TB_INTERF_MAX    45
#define TB_JITTER_GAIN   1.30f
#define TB_NOISE_PXL     12.0f
// error -> fault block
#define TB_FAULT_W       132
#define TB_FAULT_H       60
// idle "online" breathing (only when radio_online = Reticulum has opened this RNode)
#define TB_BREATH_MS     4200    // breath period (slow, calm)
#define TB_BREATH_MIN    6       // centre brightness at exhale (tight/dim)
#define TB_BREATH_MAX    48      // centre brightness at inhale (wide/bright)
#define TB_BREATH_S0     3.0f    // gaussian glow sigma at exhale (px)
#define TB_BREATH_S1     8.0f    // gaussian glow sigma at inhale (px)
#define TB_BREATH_R      40      // colour: soft aqua = "on the mesh"
#define TB_BREATH_G      150
#define TB_BREATH_B      120
// noise floor -> ambient haze (background) + calibration bar. Anchored to the
// antenna-#4 suburban benchmark (-102 dBm ~= "almost clear"); quieter reads clear.
#define TB_NOISE_MIN     -108    // dBm: fully clear at/below (quiet / rural)
#define TB_NOISE_MAX     -85     // dBm: full heavy static (bad site)
#define TB_HAZE_GAMMA    2.2f    // >1 keeps the benchmark almost-clear, ramps toward dirty
#define TB_HAZE_DENSITY  0.10f   // max fraction of field pixels hazed at full scale
#define TB_HAZE_BRIGHT   70      // max haze pixel brightness (0-255)
#define TB_HAZE_MIN_BRIGHT 28    // floor so a drawn speck is faintly visible, not rounded to black
#define TB_HAZE_MS       150     // re-seed the haze pattern this often (held between = no strobe)
#define TB_HAZE_CLEAR_R  14      // px kept clear around the heartbeat dot
#define TB_STRIP_MS      300     // calibration bar update interval
#define TB_DEMO          0
#define TB_DEMO_BREATH_START 13600  // demo: show the breathing after the fault clears
// -- motion modes --
#define TB_OUT   0
#define TB_IN    1
#define TB_PROBE 2
#define TB_NOISE 3               // interference (disorder-scaled, dense)
#define TB_FAULT 4               // error (explosion in -> persistent red block)
// -----------------------------------------------------------------------------

static GFXcanvas16 tb_canvas(TB_W, TB_H);
struct TbCohort { uint16_t col; uint32_t born; bool active; uint8_t mode; float disorder;
                  uint32_t repulse_at; uint8_t repulses_left; float pamp; };
struct TbPart   { float ang, spd01, r, vr; int8_t coh; };
static TbCohort tb_coh[TB_MAX_COHORTS];
static TbPart   tb_part[TB_PARTS];
static bool     tb_init = false;
static uint32_t tb_last_frame = 0;
static uint8_t  tb_lr = 0, tb_lg = 0, tb_lb = 0;
static float    tb_maxr = 88.0f;    // = hypot(TB_CX, TB_CY) for the field; set in tb_setup
#define TB_MAX_R tb_maxr
static float    tb_bar_norm = 0.0f; // held calibration-bar level
static uint32_t tb_bar_last = 0;
static uint32_t tb_fault_born = 0, tb_fault_until = 0;

static inline uint16_t tb565(uint8_t r, uint8_t g, uint8_t b) {
  return (uint16_t)((r & 0xF8) << 8) | (uint16_t)((g & 0xFC) << 3) | (uint16_t)(b >> 3);
}
static inline uint16_t tb_scale(uint16_t c, int bright) {
  if (bright < 0) bright = 0; if (bright > 255) bright = 255;
  int r = ((c >> 11) & 0x1F) * bright / 255;
  int g = ((c >>  5) & 0x3F) * bright / 255;
  int b = ( c        & 0x1F) * bright / 255;
  return (uint16_t)((r << 11) | (g << 5) | b);
}
static inline uint32_t tb_life(uint8_t mode) {
  if (mode == TB_FAULT) return TB_FAULT_MS;
  if (mode == TB_IN)    return TB_IN_MS;
  return TB_COHORT_MS;
}
static inline bool tb_is_red(uint8_t r, uint8_t g, uint8_t b) {
  return (r > 60 && g < 40 && b < 40);
}
static uint8_t tb_mode_for(uint8_t r, uint8_t g, uint8_t b) {
  if (tb_is_red(r, g, b))                      return TB_FAULT;   // red    -> error
  if (g > 180 && r < 120 && b < 160)           return TB_PROBE;   // v.green-> health probe
  if (b > r && b > g)                          return TB_IN;      // blue   -> RX
  if (r > 60 && b > 60 && g < r/2 && g < b)    return TB_NOISE;   // purple -> interference
  return TB_OUT;                                                  // amber  -> TX
}
static void tb_setup() {
  tb_maxr = sqrtf((float)(TB_CX*TB_CX + TB_CY*TB_CY));   // re-derived from the field
  for (int i = 0; i < TB_PARTS; i++) {
    tb_part[i].ang   = random(0, 62832) / 10000.0f;
    float u = random(0, 1000) / 1000.0f;
    tb_part[i].spd01 = powf(u, TB_SPD_POW);
    tb_part[i].r = 0; tb_part[i].vr = 0; tb_part[i].coh = -1;
  }
  for (int c = 0; c < TB_MAX_COHORTS; c++) tb_coh[c].active = false;
}
static int tb_active_particles() {
  int n = 0;
  for (int i = 0; i < TB_PARTS; i++)
    if (tb_part[i].coh >= 0 && tb_coh[tb_part[i].coh].active) n++;
  return n;
}
static void tb_fire(uint32_t now, uint16_t col, float amp, uint8_t mode, float disorder) {
  int reclaimed = -1, slot = -1;
  for (int c = 0; c < TB_MAX_COHORTS; c++) if (!tb_coh[c].active) { slot = c; break; }
  if (slot < 0) { slot = 0;
    for (int c = 1; c < TB_MAX_COHORTS; c++) if (tb_coh[c].born < tb_coh[slot].born) slot = c;
    reclaimed = slot; }
  tb_coh[slot].col=col; tb_coh[slot].born=now; tb_coh[slot].active=true;
  tb_coh[slot].mode=mode; tb_coh[slot].disorder=disorder;
  if (mode == TB_FAULT) { tb_fault_born = now; tb_fault_until = now + TB_FAULT_HOLD; }
  tb_coh[slot].pamp = amp;                                        // TX double-pulses; RX is driven
  if (mode == TB_OUT) {                                           // by the radio signal, not forced
    tb_coh[slot].repulses_left = TB_PULSES - 1;
    tb_coh[slot].repulse_at = now + TB_PULSE_GAP;
  } else {
    tb_coh[slot].repulses_left = 0;
  }
  float frac = TB_RECOLOUR_FRAC;
  if (mode == TB_IN) frac = TB_IN_FRAC;
  else if (mode == TB_NOISE) frac = TB_NOISE_FRAC;
  else if (mode == TB_FAULT) frac = TB_FAULT_FRAC;
  bool first = (tb_active_particles() == 0);
  for (int i = 0; i < TB_PARTS; i++) {
    bool take = first
             || (reclaimed >= 0 && tb_part[i].coh == reclaimed)
             || (random(0, 1000) / 1000.0f < frac);
    if (!take) continue;
    TbPart &p = tb_part[i]; p.coh = slot;
    if (mode == TB_OUT) {
      p.r = 0; p.vr = amp * p.spd01 * TB_KICK;                     // wave from centre, outward only
    } else if (mode == TB_IN) {
      p.r = (0.55f + 0.45f * p.spd01) * TB_MAX_R; p.vr = -amp * p.spd01 * TB_KICK;
    } else if (mode == TB_PROBE) {                                 // uniform symmetrical ring
      p.r = 0; p.vr += amp * (0.62f + 0.30f * p.spd01) * TB_KICK * TB_PROBE_K;
    } else if (mode == TB_FAULT) {                                 // explosion in from outside
      p.r = (0.65f + 0.35f * p.spd01) * TB_MAX_R; p.vr = -amp * p.spd01 * TB_KICK;
    } else {                                                       // TB_NOISE: wide spread, out
      p.r = (0.15f + 0.75f * p.spd01) * TB_MAX_R;
      p.vr = amp * p.spd01 * TB_KICK * 0.5f;
    }
  }
}
static void tb_event(uint32_t now, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t mode = tb_mode_for(r, g, b);
  float amp = TB_AMP_TX, disorder = 0.0f;
  if (mode == TB_IN) {
    int s = last_rssi; if (s < -115) s = -115; if (s > -45) s = -45;
    amp = 0.15f + (s + 115) * (1.0f - 0.15f) / 70.0f;
  } else if (mode == TB_FAULT) {
    disorder = 1.0f; amp = 0.75f;
  } else if (mode == TB_NOISE) {
    int lvl = current_rssi - noise_floor;                        // fresh: 3ms sampling
    if (lvl < TB_INTERF_MIN) lvl = TB_INTERF_MIN;
    if (lvl > TB_INTERF_MAX) lvl = TB_INTERF_MAX;
    disorder = (float)(lvl - TB_INTERF_MIN) / (TB_INTERF_MAX - TB_INTERF_MIN);
    amp = 0.35f + 0.30f * disorder;
  }
  tb_fire(now, tb565(r, g, b), amp, mode, disorder);
}

#if TB_DEMO
struct TbDemoStep { uint32_t t; uint8_t r, g, b; float amp, dis; };
static const TbDemoStep tb_demo_script[] = {
  {0,     0,   0,   255, 0.90f, 0.0f},   // blue RX (converge in, dense)
  {1600,  255, 80,  0  , 1.00f, 0.0f},   // orange TX (bloom out past edge)
  {3200,  255, 80,  0  , 1.00f, 0.0f},   // orange TX ...
  {3330,  0,   0,   255, 0.90f, 0.0f},   //   + blue RX -> round trip
  {5000,  144, 0,   112, 0.45f, 0.25f},  // purple interference: MILD (near-clean)
  {6600,  144, 0,   112, 0.65f, 0.95f},  // purple interference: SEVERE (dense chaos)
  {8200,  40,  255, 120, 0.90f, 0.0f},   // green probe pulse 1
  {8430,  40,  255, 120, 0.90f, 0.0f},   // green probe pulse 2
  {8660,  40,  255, 120, 0.90f, 0.0f},   // green probe pulse 3
  {8890,  40,  255, 120, 0.90f, 0.0f},   // green probe pulse 4
  {9120,  40,  255, 120, 0.90f, 0.0f},   // green probe pulse 5
  {9900,  255, 0,   0  , 0.75f, 1.0f},   // red error: explosion in -> PERSISTENT block
};
#define TB_DEMO_LEN 18800
#define TB_DEMO_FAULT_HOLD 3400          // demo: hold the fault block to show persistence
static int tb_demo_idx = 0;
static uint32_t tb_demo_start = 0, tb_demo_lastt = 0, tb_demo_t = 0;
static void tb_demo_step(uint32_t now) {
  if (tb_demo_start == 0) tb_demo_start = now;
  uint32_t t = (now - tb_demo_start) % TB_DEMO_LEN;
  tb_demo_t = t;
  if (t < tb_demo_lastt) tb_demo_idx = 0;
  tb_demo_lastt = t;
  int n = sizeof(tb_demo_script) / sizeof(tb_demo_script[0]);
  while (tb_demo_idx < n && tb_demo_script[tb_demo_idx].t <= t) {
    TbDemoStep s = tb_demo_script[tb_demo_idx++];
    uint8_t mode = tb_mode_for(s.r, s.g, s.b);
    tb_fire(now, tb565(s.r, s.g, s.b), s.amp, mode, s.dis);
    if (mode == TB_FAULT) tb_fault_until = now + TB_DEMO_FAULT_HOLD;   // persist for the demo
  }
}
#endif

void tracker_status_burst() {
  uint32_t now = millis();
  if (now - tb_last_frame < TB_FRAME_MS) return;
  tb_last_frame = now;
  if (!tb_init) { if (tb_canvas.getBuffer() == NULL) return; tb_setup(); tb_init = true; }

  uint8_t r = npr, g = npg, b = npb;
#if TB_DEMO
  tb_demo_step(now);
#else
  // TX FIX (2026-07-31): led_tx_on/off both happen inside ONE loop pass, so
  // the colour sampler below can never see the amber - real transmissions were
  // invisible on the TFT. Consume the firmware's own display_tx latch instead.
  if (display_tx) { display_tx = false; tb_event(now, 255, 80, 0); }
  int dr=(int)r-tb_lr, dg=(int)g-tb_lg, db=(int)b-tb_lb;
  if (((dr*dr+dg*dg+db*db) > (28*28)) && (r>24||g>24||b>24)) tb_event(now, r, g, b);
  tb_lr=r; tb_lg=g; tb_lb=b;
  if (tb_is_red(r, g, b)) {                        // LED steady red -> fault ongoing, keep block
    if (tb_fault_born == 0 || now > tb_fault_until) tb_fault_born = now;
    tb_fault_until = now + TB_FAULT_TAIL;
  }
#endif

  int active_coh = 0;
  for (int c = 0; c < TB_MAX_COHORTS; c++) {
    if (tb_coh[c].active && (now - tb_coh[c].born) < tb_life(tb_coh[c].mode)) active_coh++;
    else tb_coh[c].active = false;
  }
  for (int i = 0; i < TB_PARTS; i++)
    if (tb_part[i].coh >= 0 && !tb_coh[tb_part[i].coh].active) tb_part[i].coh = -1;
  bool fault_active = (tb_fault_born != 0) && (now < tb_fault_until);

  // TX second pulse: re-kick the cohort's particles outward after a short gap
  for (int c = 0; c < TB_MAX_COHORTS; c++) {
    if (!tb_coh[c].active || tb_coh[c].repulses_left == 0 || now < tb_coh[c].repulse_at) continue;
    float a = tb_coh[c].pamp;
    for (int i = 0; i < TB_PARTS; i++) {
      if (tb_part[i].coh != c) continue;
      tb_part[i].r = 0; tb_part[i].vr = a * tb_part[i].spd01 * TB_KICK;  // second wave from centre
    }
    tb_coh[c].repulses_left--;
    tb_coh[c].repulse_at = now + TB_PULSE_GAP;
  }

  tb_canvas.fillScreen(0x0000);

  // ---- the Reticulum mark, dim, BEHIND the pulse (operator, 2026-08-27:
  // "give the heltec tracker the same logo in the centre" — the T114's
  // watermark, scaled to this 160x75 field). Drawn FIRST — the furthest
  // background layer: the noise haze speckles OVER it, the breath glows
  // over it, bursts fly across it. Geometry traced from the authentic
  // mark at ring r=60 (see ~/RTNode-2400 TbField.h), scaled to r=34.
  {
    const float mk = 34.0f / 60.0f;
#define TB_MK(v) ((int)lroundf((v) * mk))
    int cx = TB_CX, cy = TB_CY;
    uint16_t ring = tb565(46, 62, 58);
    uint16_t node = tb565(60, 92, 82);
    // double outer ring + thin inner ring, as the mark has
    tb_canvas.drawCircle(cx, cy, TB_MK(60), ring);
    tb_canvas.drawCircle(cx, cy, TB_MK(60) - 1, ring);
    tb_canvas.drawCircle(cx, cy, TB_MK(54), ring);
    // nodes (traced): hub pair centre-left, edge-breaker right, corner
    // node bottom-left, three-dot chain top, three-dot arc bottom
    // ONE hub dot EXACTLY on the breath-pulse centre (operator, 2026-08-27:
    // offset hub = crescent moon at pulse-minimum; any companion beside it
    // = clutter. Pair detail dropped; red's line runs from the hub).
    int hub_x = cx,              hub_y = cy;
    int red_x = cx + TB_MK(61),  red_y = cy + TB_MK(-1);
    int blc_x = cx + TB_MK(-57), blc_y = cy + TB_MK(53);
    int ta_x = cx + TB_MK(-30), ta_y = cy + TB_MK(-48);
    int tb_x = cx + TB_MK(-14), tb_y = cy + TB_MK(-34);
    int tc_x = cx + TB_MK(-44), tc_y = cy + TB_MK(-22);
    int ba_x = cx + TB_MK(-7),  ba_y = cy + TB_MK(34);
    int bb_x = cx + TB_MK(12),  bb_y = cy + TB_MK(41);
    int bc_x = cx + TB_MK(24),  bc_y = cy + TB_MK(29);
    tb_canvas.drawLine(tc_x, tc_y, ta_x, ta_y, ring);
    tb_canvas.drawLine(ta_x, ta_y, tb_x, tb_y, ring);
    tb_canvas.drawLine(tb_x, tb_y, hub_x, hub_y, ring);
    tb_canvas.drawLine(hub_x, hub_y, red_x, red_y, ring);
    tb_canvas.drawLine(hub_x, hub_y, blc_x, blc_y, ring);
    tb_canvas.drawLine(blc_x, blc_y, ba_x, ba_y, ring);
    tb_canvas.drawLine(ba_x, ba_y, bb_x, bb_y, ring);
    tb_canvas.drawLine(bb_x, bb_y, bc_x, bc_y, ring);
    tb_canvas.drawLine(bc_x, bc_y, red_x, red_y, ring);
    tb_canvas.fillCircle(hub_x, hub_y, TB_MK(8), node);
    tb_canvas.fillCircle(red_x, red_y, TB_MK(7), node);
    tb_canvas.fillCircle(blc_x, blc_y, TB_MK(7), node);
    tb_canvas.fillCircle(ta_x, ta_y, TB_MK(3), node);
    tb_canvas.fillCircle(tb_x, tb_y, TB_MK(4), node);
    tb_canvas.fillCircle(tc_x, tc_y, TB_MK(3), node);
    tb_canvas.fillCircle(ba_x, ba_y, TB_MK(6), node);
    tb_canvas.fillCircle(bb_x, bb_y, TB_MK(4), node);
    tb_canvas.fillCircle(bc_x, bc_y, TB_MK(3), node);
    // the board's flashed ROLE where the mark carries its RNS letters
    // (operator, 2026-08-27). Read LIVE from op_mode — MODE_TNC is the
    // transport role — so a reflash between roles tells the truth.
    // PIL-approved: fixed at cx+5, cy-13 (candidate d — the 36px label
    // can't scale below font size 1; only its tail grazes the r=34 ring).
    // role label, brighter again (operator, 2026-08-27 round 2: "a little
    // brighter... not as bright as the lora wifi lan ble" rows at 238/230/215)
    // — clearly legible, still short of the interface rows. (operator,
    // 2026-08-27). Transport = two stacked lines; RNode single line.
    // Label only once the radio is ONLINE (operator, 2026-08-27: the boot
    // window showed "RNode" before the stored transport config applied —
    // "potentially confusing". Until the board is truly serving, the mark
    // stays unlabelled; the role appears the moment the LORA dot goes
    // green, and then it is the truth.)
    if (radio_online) {
      tb_canvas.setTextWrap(false);
      tb_canvas.setTextSize(1);
      tb_canvas.setTextColor(tb565(150, 195, 172));
      if (op_mode == MODE_TNC) {
      int lx = cx + 5, ly = cy - 17;
      tb_canvas.setCursor(lx, ly);
      tb_canvas.print("Transport");
      tb_canvas.setCursor(lx + (9 - 4) * 6 / 2, ly + 9);
      tb_canvas.print("Node");
    } else {
      tb_canvas.setCursor(cx + 5, cy - 13);
      tb_canvas.print("RNode");
      }
    }
#undef TB_MK
  }

  // --- noise floor -> ambient haze, drawn FIRST so everything composites over it ---
  float nf_norm = 0.0f;
  if (noise_floor_sampled) {
    float t = (float)(noise_floor - TB_NOISE_MIN) / (float)(TB_NOISE_MAX - TB_NOISE_MIN);
    if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
    nf_norm = t;
  }
  float haze01 = powf(nf_norm, TB_HAZE_GAMMA);
  if (haze01 > 0.001f) {
    int nhz = (int)(TB_HAZE_DENSITY * haze01 * TB_W * TB_FIELD_H);
    uint16_t hazecol = tb565(200, 210, 225);
    uint32_t hz = (millis() / TB_HAZE_MS) * 2654435761u;    // held ~150ms, then re-seeds
    int clr2 = TB_HAZE_CLEAR_R * TB_HAZE_CLEAR_R;
    for (int k = 0; k < nhz; k++) {
      hz ^= hz << 13; hz ^= hz >> 17; hz ^= hz << 5; int x = (int)(hz % TB_W);
      hz ^= hz << 13; hz ^= hz >> 17; hz ^= hz << 5; int y = (int)(hz % TB_FIELD_H);
      hz ^= hz << 13; hz ^= hz >> 17; hz ^= hz << 5;
      float jit = 0.45f + 0.55f * (((hz >> 8) & 0xFF) / 255.0f);
      int br = TB_HAZE_MIN_BRIGHT + (int)((TB_HAZE_BRIGHT - TB_HAZE_MIN_BRIGHT) * haze01 * jit);
      int dx = x - TB_CX, dy = y - TB_CY;
      if (dx*dx + dy*dy < clr2) continue;                   // keep clear around the heartbeat
      tb_canvas.drawPixel(x, y, tb_scale(hazecol, br));
    }
  }

  int cc[TB_MAX_COHORTS] = {0};
  for (int i = 0; i < TB_PARTS; i++) {
    TbPart &p = tb_part[i];
    if (p.coh < 0 || !tb_coh[p.coh].active) {                      // unowned: settle silently
      p.vr += -TB_SPRING * p.r; p.vr *= TB_DAMP; p.r += p.vr;
      if (p.r < 0) { p.r = 0; p.vr = 0; }
      continue;
    }
    cc[p.coh]++;
    uint8_t mode = tb_coh[p.coh].mode;
    uint16_t col = tb_coh[p.coh].col;
    if (mode == TB_OUT) {                                         // TX: outward only, no bounce-back
      p.r += p.vr;                                                // ballistic; flies off the edge
      if (p.r > TB_OUT_MAX) p.r = TB_OUT_MAX;                     // park off-screen; cohort end resets
    } else {
      p.vr += -TB_SPRING * p.r; p.vr *= TB_DAMP; p.r += p.vr;     // spring modes (RX/probe)
      if (p.r < 0) { p.r = 0; p.vr = 0; }
      if (p.r > TB_MAX_R) { p.r = TB_MAX_R; p.vr = 0; }
    }

    if (mode == TB_NOISE || mode == TB_FAULT) {                    // disorder-scaled scatter
      float dis = tb_coh[p.coh].disorder;
      float ja = (random(-1000, 1001) / 1000.0f) * dis * TB_JITTER_GAIN;
      int nx = (int)((random(-1000, 1001) / 1000.0f) * dis * TB_NOISE_PXL);
      int ny = (int)((random(-1000, 1001) / 1000.0f) * dis * TB_NOISE_PXL);
      int x = TB_CX + (int)(cosf(p.ang + ja) * p.r) + nx;
      int y = TB_CY + (int)(sinf(p.ang + ja) * p.r) + ny;
      if (x < 0 || y < 0 || x > TB_W - TB_BLK || y > TB_H - TB_BLK) continue;
      int bright = 255 - (int)(p.r / TB_MAX_R * TB_EDGE_FADE);
      uint16_t pc = tb_scale(col, bright);
      tb_canvas.fillRect(x, y, TB_BLK, TB_BLK, pc);
      if (mode == TB_NOISE) {                                      // purple: second block = 2x pixels
        int x2 = x + ((random(0, 2)) ? TB_BLK : -TB_BLK);
        int y2 = y + ((random(0, 2)) ? TB_BLK : -TB_BLK);
        if (x2 >= 0 && y2 >= 0 && x2 <= TB_W - TB_BLK && y2 <= TB_H - TB_BLK)
          tb_canvas.fillRect(x2, y2, TB_BLK, TB_BLK, pc);
      }
      continue;
    }
    // OUT / IN / PROBE: radial + comet tail
    int lead = 255 - (int)(p.r / TB_MAX_R * TB_EDGE_FADE);
    if (mode == TB_IN) {                                           // RX lingers + fades over 2x life
      float agef = 1.0f - (float)(now - tb_coh[p.coh].born) / TB_IN_MS;
      if (agef < 0.0f) agef = 0.0f;
      lead = (int)(lead * agef);
    }
    float dir = (p.vr >= 0.0f) ? 1.0f : -1.0f;
    for (int t = TB_TRAIL - 1; t >= 0; t--) {
      float rr = p.r - dir * t * TB_TRAIL_STEP; if (rr < 0) rr = 0;
      int x = TB_CX + (int)(cosf(p.ang) * rr);
      int y = TB_CY + (int)(sinf(p.ang) * rr);
      if (x < 0 || y < 0 || x > TB_W - TB_BLK || y > TB_H - TB_BLK) continue;
      tb_canvas.fillRect(x, y, TB_BLK, TB_BLK, tb_scale(col, lead * (TB_TRAIL - t) / TB_TRAIL));
    }
  }

  // additive core (node) -- only while a non-fault burst is running; never idle
  if (active_coh > 0 && !fault_active) {
    float cr=0, cg=0, cb=0;
    for (int c = 0; c < TB_MAX_COHORTS; c++) {
      if (!tb_coh[c].active) continue;
      float w = 1.0f - (float)(now - tb_coh[c].born) / tb_life(tb_coh[c].mode); if (w < 0) w = 0;
      float sh = (float)cc[c] / TB_PARTS; uint16_t col = tb_coh[c].col;
      cr += ((col>>11)&0x1F)*(255.0f/31)*sh*w;
      cg += ((col>> 5)&0x3F)*(255.0f/63)*sh*w;
      cb += ( col     &0x1F)*(255.0f/31)*sh*w;
    }
    cr*=TB_CORE_GAIN; cg*=TB_CORE_GAIN; cb*=TB_CORE_GAIN;
    uint16_t core_col = tb565(cr>255?255:cr, cg>255?255:cg, cb>255?255:cb);
    tb_canvas.fillCircle(TB_CX, TB_CY, TB_CORE_R, core_col);
  }

  // idle breathing -- subtle "alive" pulse ONLY while Reticulum has this RNode online
  if (active_coh == 0 && !fault_active) {
#if TB_DEMO
    bool tb_online = (tb_demo_t >= TB_DEMO_BREATH_START);
#else
    bool tb_online = radio_online;
#endif
    if (tb_online) {
      float ph  = (now % TB_BREATH_MS) / (float)TB_BREATH_MS;
      float b01 = 0.5f - 0.5f * cosf(2.0f * 3.14159265f * ph);    // smooth 0..1
      float sigma = TB_BREATH_S0 + b01 * (TB_BREATH_S1 - TB_BREATH_S0);  // width breathes
      int   peak  = TB_BREATH_MIN + (int)(b01 * (TB_BREATH_MAX - TB_BREATH_MIN));
      uint16_t bcol = tb565(TB_BREATH_R, TB_BREATH_G, TB_BREATH_B);
      float inv2s2 = 1.0f / (2.0f * sigma * sigma);
      int rad = (int)(2.6f * sigma) + 1;                          // gaussian glow: no hard edge,
      for (int dy = -rad; dy <= rad; dy++) {                      // grows/shrinks into its own fade
        int y = TB_CY + dy; if (y < 0 || y >= TB_H) continue;
        for (int dx = -rad; dx <= rad; dx++) {
          int x = TB_CX + dx; if (x < 0 || x >= TB_W) continue;
          int br = (int)(peak * expf(-(float)(dx*dx + dy*dy) * inv2s2));
          if (br > 0) tb_canvas.drawPixel(x, y, tb_scale(bcol, br));
        }
      }
    }
  }

  // FAULT block -- forms after the explosion, holds + flickers while the fault persists
  if (fault_active) {
    uint32_t age = now - tb_fault_born;
    float ramp = ((float)age - 0.30f * TB_FAULT_MS) / (0.30f * TB_FAULT_MS); // grows 30%..60%
    if (ramp < 0) ramp = 0; if (ramp > 1) ramp = 1;
    if (ramp > 0.0f) {
      int w = (int)(ramp * TB_FAULT_W), h = (int)(ramp * TB_FAULT_H);
      int bright = (ramp >= 1.0f) ? 255 - random(0, 90) : 255;    // flicker once full
      tb_canvas.fillRect(TB_CX - w/2, TB_CY - h/2, w, h, tb_scale(tb565(255,0,0), bright));
      tb_canvas.drawRect(TB_CX - w/2, TB_CY - h/2, w, h, tb565(60,0,0));
    }
  } else {
    tb_fault_born = 0;
  }

  // --- calibration bar (bottom edge): honest LINEAR noise level, held steady ---
  if (now - tb_bar_last >= TB_STRIP_MS) { tb_bar_norm = nf_norm; tb_bar_last = now; }
  {
    int by = TB_H - TB_BAR_H;
    tb_canvas.fillRect(0, by, TB_W, TB_BAR_H, tb565(18, 18, 22));      // track
    if (noise_floor_sampled) {
      int fw = (int)(tb_bar_norm * TB_W); if (fw < 1 && tb_bar_norm > 0.0f) fw = 1;
      uint8_t r_, g_, b_;                                              // green -> amber -> red
      if (tb_bar_norm < 0.5f) { float u = tb_bar_norm / 0.5f;
        r_ = (uint8_t)(30 + u*(235-30)); g_ = (uint8_t)(200 + u*(170-200)); b_ = (uint8_t)(40*(1.0f-u)); }
      else { float u = (tb_bar_norm - 0.5f) / 0.5f;
        r_ = 235; g_ = (uint8_t)(170 + u*(30-170)); b_ = 0; }
      // fill anchored to the PHYSICAL antenna (fleet rule, operator
      // 2026-08-21): the bar grows AWAY from the antenna end as the floor
      // worsens — bad news retreats from the goal. The 180-deg rotation-3
      // flip (2026-08-27) moved the antenna to the LOGICAL LEFT, so the
      // fill anchor swapped sides to keep the physical truth (operator:
      // "reverse the flow ... so it moves away from the antenna").
      if (fw > 0) tb_canvas.fillRect(0, by, fw, TB_BAR_H, tb565(r_, g_, b_));
    }
  }

  // live frequency, amber, bottom-right above the calibration bar
  // (operator, 2026-08-21: same treatment as the T114). Read off the
  // radio state, never hardcoded.
  {
    char fbuf[12];
    snprintf(fbuf, sizeof(fbuf), "%.3f", (double)lora_freq / 1000000.0);
    tb_canvas.setTextWrap(false);
    tb_canvas.setTextSize(1);
    int tw = (int)strlen(fbuf) * 6;
    tb_canvas.setCursor(TB_W - tw - 3, TB_FIELD_H - 10);
    tb_canvas.setTextColor(tb565(240, 180, 60));
    tb_canvas.print(fbuf);
  }

#if HAS_GPS
  // GPS map-pin icon, top-right (operator, 2026-08-27: "this icon appears in
  // the top right corner in RED when GPS is active, GREEN when satellites
  // connected, disappear if no GPS at all"). Presence = the GNSS is alive and
  // producing NMEA (charsProcessed only grows when real sentences arrive — a
  // dead/unpowered module never draws anything). Green needs a FRESH fix
  // (valid + < 10 s old — the telemetry-fresh-vs-actual-fix trap).
  if (gps.charsProcessed() > 10) {
    bool fix = gps.location.isValid() && gps.location.age() < 10000;
    // blit the painted pin (GpsPin.h), colour = state, 0x0000 = transparent
    const uint16_t *pin = fix ? gps_pin_green : gps_pin_red;
    int gx = TB_W - GPS_PIN_W - 2, gy = 1;
    for (int yy = 0; yy < GPS_PIN_H; yy++)
      for (int xx = 0; xx < GPS_PIN_W; xx++) {
        uint16_t c = pin[yy * GPS_PIN_W + xx];
        if (c) tb_canvas.drawPixel(gx + xx, gy + yy, c);
      }
  }
#endif

  display.startWrite();
  display.setAddrWindow(0, 0, TB_W, TB_H);
  display.writePixels(tb_canvas.getBuffer(), (uint32_t)TB_W * TB_H, true, false);
  display.endWrite();
}
#endif

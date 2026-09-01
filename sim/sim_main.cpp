// Host-side display simulator for the MeshPocket e-ink panel.
//
// draw_stat_area() and draw_disp_area() are pure software: they paint into two
// 64x64 GFXcanvas1 buffers, which are only afterwards blitted to the panel. So
// they can be compiled and run on a laptop, driven to any state, and LOOKED AT
// — no board, no flash, no 5am squinting.
//
// This exists because every display defect in this branch escaped code review
// and was caught by someone holding the hardware: a scrolling waterfall, a
// status page rotating on a timer, a node ID drawn after the refresh. All three
// are visible in one frame here.
//
// Usage:  ./sim <state> <out.pgm>
#include "arduino_shim.h"

unsigned long SIM_MILLIS = 10000;

#include <Adafruit_GFX.h>
#include "stubs/SPI.h"
#include "stubs/Wire.h"
#include "stubs/nrf_regs.h"
SimSPIM sim_spim0, sim_spim1, sim_spim2, sim_spim3;
SPIClass SPI;
TwoWire Wire;

// ---- the firmware's world, as the display layer sees it -------------------
#include "../Boards.h"

// Exactly the questions Display.h asks the radio - names taken from the
// source, not guessed.
struct RadioInterface {
    bool  online = true;
    int   rssi = -94;
    uint8_t sf = 9;
    uint32_t bitrate = 1758;
    float airtime = 0, lt_airtime = 0, chutil = 0, lt_chutil = 0;
    bool  getRadioOnline()        { return online; }
    int   currentRssi()           { return rssi; }
    uint8_t getSpreadingFactor()  { return sf; }
    uint32_t getBitrate()         { return bitrate; }
    float getAirtime()            { return airtime; }
    float getLongtermAirtime()    { return lt_airtime; }
    float getTotalChannelUtil()   { return chutil; }
    float getLongtermChannelUtil(){ return lt_chutil; }
    uint8_t getIndex()            { return 0; }
};
struct FIFOBuffer { uint8_t dummy; };

#include "../Config.h"

RadioInterface  sim_radio;

// Only what Config.h does not already declare.
char bt_dh[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x09,0x2B};
bool charging = false;
bool fw_signature_validated = true;
bool device_firmware_ok() { return fw_signature_validated; }
uint8_t eeprom_read(uint32_t) { return 0xFF; }
char bt_devname[11] = "RNode 092B";
bool device_signatures_ok() { return fw_signature_validated; }

#include "../Display.h"

// ---- render ---------------------------------------------------------------
static void dump(const char* path) {
    const int W = 250, H = 122;
    const float S = 1.90625f;
    static uint8_t px[H][W];
    memset(px, 255, sizeof(px));
    struct { GFXcanvas1* c; int x0; } panes[2] = { {&disp_area, 0}, {&stat_area, 122} };
    for (auto& p : panes) {
        const uint8_t* buf = p.c->getBuffer();
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < 122; x++) {
                int sx = (int)(x / S), sy = (int)(y / S);
                if (sx > 63 || sy > 63) continue;
                int bit = (buf[sy * 8 + (sx >> 3)] >> (7 - (sx & 7))) & 1;
                int ox = p.x0 + x;
                if (ox < W) px[y][ox] = bit ? 0 : 255;
            }
        }
    }
    FILE* f = fopen(path, "wb");
    fprintf(f, "P5\n%d %d\n255\n", W, H);
    fwrite(px, 1, sizeof(px), f);
    fclose(f);
}

int main(int argc, char** argv) {
    std::string state = argc > 1 ? argv[1] : "linked";
    const char* out  = argc > 2 ? argv[2] : "frame.pgm";

    // A booted, provisioned, healthy board - the baseline each state varies
    // from. Without these the firmware quite correctly draws DEVICE STARTING.
    device_init_done = true;
    eeprom_ok        = true;
    hw_ready         = true;
    modems_installed = true;
    radio_online     = true;
    disp_ready       = true;
    firmware_update_mode = false;
    console_active   = false;
    interface_obj[0] = &sim_radio;

    // A COMMA-SEPARATED state argument draws each state in turn into the SAME
    // canvases, then dumps the final frame. That distinction matters: the
    // firmware never rebuilds these buffers from nothing, it redraws over
    // whatever the last frame left behind. Rendering one state into a fresh
    // canvas - all this could do before - cannot show an element that fails to
    // erase its predecessor, which is exactly how a cross and a "connecting"
    // mark ended up occupying one circle on real hardware while every
    // single-state render here looked perfect.
    //
    //   ./sim nophone,pairing seq.pgm     one mark, or two on top of each other
    size_t pos = 0; std::string seq = state; int drawn = 0;
    while (pos <= seq.size()) {
        size_t comma = seq.find(',', pos);
        std::string s = seq.substr(pos, comma == std::string::npos
                                        ? std::string::npos : comma - pos);
        // Reset only the world, never the canvases - the canvases are the
        // thing under test.
        sim_radio.online = true; radio_online = true;
        modems_installed = true; hw_ready = true;
        fw_signature_validated = true;
        sim_radio.chutil = 0; sim_radio.airtime = 0;
        battery_percent = 76; bt_ssp_pin = 0;

        if (s == "linked")        { bt_state = BT_STATE_CONNECTED; }
        else if (s == "pairing")  { bt_state = BT_STATE_PAIRING; bt_ssp_pin = 418209; }
        else if (s == "nophone")  { bt_state = BT_STATE_ON; }
        else if (s == "btoff")    { bt_state = BT_STATE_OFF; }
        else if (s == "noradio")  { sim_radio.online = false; radio_online = false;
                                    modems_installed = false; hw_ready = false; }
        else if (s == "corrupt")  { fw_signature_validated = false; hw_ready = false; }
        else if (s == "busy")     { bt_state = BT_STATE_CONNECTED;
                                    sim_radio.chutil = 42; sim_radio.airtime = 12; }
        else if (s == "lowbatt")  { bt_state = BT_STATE_CONNECTED; battery_percent = 9; }
        else { fprintf(stderr, "unknown state '%s'\n", s.c_str()); return 2; }

        draw_stat_area();
        draw_disp_area();
        drawn++;
        printf("  drew '%s'\n", s.c_str());

        if (comma == std::string::npos) break;
        pos = comma + 1;
    }

    dump(out);
    printf("rendered %d state(s) -> %s\n", drawn, out);
    return 0;
}

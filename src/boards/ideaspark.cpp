#if defined(BOARD_IDEASPARK)
#include "../IBoard.h"
#include "../MonoCanvas.h"
#include <Arduino.h>

// ideaspark ESP8266 0.96" OLED (VR:2.1): SSD1306 128x64 @ 0x3C, software I2C
// SDA=GPIO12 SCL=GPIO14 (U8g2 SW-I2C order is rotation, SCL, SDA, reset).
// Single FLASH button on GPIO0 (active-LOW): tap=primary, hold>=600ms=secondary.
static U8G2_SSD1306_128X64_NONAME_F_SW_I2C g_u8g2(U8G2_R0, 14, 12, U8X8_PIN_NONE);

class Ideaspark : public IBoard {
public:
    Ideaspark() : _canvas(g_u8g2, 128, 64) {}

    void begin() override {
        g_u8g2.begin();
        g_u8g2.setBusClock(400000);
        g_u8g2.setContrast(255);
        pinMode(0, INPUT_PULLUP);
        pinMode(2, OUTPUT);
        digitalWrite(2, HIGH);   // onboard LED is active-LOW; HIGH = off
    }
    Canvas& canvas() override { return _canvas; }

    void poll() override {
        bool down = (digitalRead(0) == LOW);
        uint32_t now = millis();
        // Gestures fire on RELEASE by duration, so a continuous 5 s hold can reach
        // the factory-reset check (held()) without the long-press refresh firing
        // mid-hold: <600ms = tap, 600ms..5s = long-press, >=5s handled by held().
        if (down && !_prev) {
            _downAt = now;
        } else if (!down && _prev) {
            uint32_t dur = now - _downAt;
            if (dur < 600)       _tap = true;
            else if (dur < 5000) _long = true;
        }
        _prev = down;
    }
    bool tapped() override      { bool r = _tap;  _tap = false;  return r; }
    bool longPressed() override { bool r = _long; _long = false; return r; }
    bool held(uint32_t ms) override { return _prev && (millis() - _downAt >= ms); }

    void brightness(uint8_t level) override { g_u8g2.setPowerSave(level == 0 ? 1 : 0); }
    void led(bool on) override { digitalWrite(2, on ? LOW : HIGH); }
    const char* apPrefix() override { return "AIUsage"; }

private:
    MonoCanvas _canvas;
    bool _prev = false, _tap = false, _long = false;
    uint32_t _downAt = 0;
};

IBoard& Board() { static Ideaspark b; return b; }
#endif

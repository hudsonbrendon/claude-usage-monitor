#pragma once
#include "Canvas.h"
#include <stdint.h>

// One board = one display + one input scheme. Exactly one board file is
// compiled (selected by its BOARD_* flag) and provides Board().
class IBoard {
public:
    virtual ~IBoard() {}
    virtual void    begin() = 0;
    virtual Canvas& canvas() = 0;
    virtual void    poll() = 0;            // sample inputs once per loop
    virtual bool    tapped() = 0;          // primary action (consumed on read)
    virtual bool    longPressed() = 0;     // secondary action (consumed)
    virtual bool    held(uint32_t ms) = 0; // currently held >= ms (factory reset)
    virtual int     battery()           { return -1; }   // -1 = none
    virtual void    brightness(uint8_t) {}               // 0=off..n
    virtual void    led(bool on) {}                      // onboard alert LED (no-op default)
    virtual const char* apPrefix()      { return "AIUsage"; }
};

IBoard& Board();   // defined by the selected board file

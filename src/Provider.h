#pragma once
#include "Usage.h"
#include "Settings.h"
#include <Arduino.h>

// A usage data source (Claude, Codex, ...). Each reads the credentials it needs
// from Settings. main holds a registry and one active Provider.
class Provider {
public:
    virtual ~Provider() {}
    virtual const char* id() = 0;                 // short label, e.g. "Claude"
    virtual bool available(const Settings& s) = 0; // are this provider's creds set?
    virtual UsageStatus fetch(const Settings& s, char* errOut, size_t errLen) = 0;
};

#pragma once
#include <cstdint>
#include <cstddef>

// Pure, Arduino-independent. Utilization headers are "0.0".."1.0" strings;
// reset headers are unix-epoch second strings. Empty/null inputs => invalid.
struct UsageStatus {
    float    h5Percent = 0.0f;   // 0..100
    float    d7Percent = 0.0f;   // 0..100
    uint32_t h5Reset   = 0;      // unix epoch, 0 if unknown
    uint32_t d7Reset   = 0;
    bool     valid     = false;  // true if at least one utilization parsed
};

UsageStatus parseUsage(const char* h5util, const char* h5reset,
                       const char* d7util, const char* d7reset);

// Parse the Codex /backend-api/codex/usage JSON body into a UsageStatus.
// Reads rate_limit.primary_window (5h) and .secondary_window (weekly).
UsageStatus parseCodexUsage(const char* json);

// "3d4h" / "2h05m" / "12m" / "now" / "--" into out.
void formatCountdown(uint32_t epoch, uint32_t now, char* out, size_t outLen);

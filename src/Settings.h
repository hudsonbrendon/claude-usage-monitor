#pragma once
#include <string>
#include <cstdint>
#include <cstddef>

struct Settings {
    std::string ssid;
    std::string password;
    std::string token;            // Claude OAuth token (claude setup-token)
    std::string codexToken;       // Codex access_token (~/.codex/auth.json)
    std::string codexAccountId;   // Codex account_id
    std::string codexRefreshToken;   // Codex OAuth refresh_token (auto-renews the access token)
    uint16_t    pollSeconds = 120;
    uint8_t     alertPercent = 80;   // warn at/over this % (0 = alerts off)
    bool        configured   = false;
};

bool settingsToJson(const Settings& s, char* out, size_t outLen);
// Missing fields keep defaults; `configured` = ssid AND at least one provider's creds.
bool settingsFromJson(const char* json, Settings& s);

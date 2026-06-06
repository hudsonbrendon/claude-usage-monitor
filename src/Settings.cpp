#include "Settings.h"
#include <ArduinoJson.h>

bool settingsToJson(const Settings& s, char* out, size_t outLen) {
    JsonDocument doc;
    doc["ssid"] = s.ssid;
    doc["password"] = s.password;
    doc["token"] = s.token;
    doc["codexToken"] = s.codexToken;
    doc["codexAccountId"] = s.codexAccountId;
    doc["codexRefreshToken"] = s.codexRefreshToken;
    doc["pollSeconds"] = s.pollSeconds;
    doc["alertPercent"] = s.alertPercent;
    size_t n = serializeJson(doc, out, outLen);
    return n > 0 && n < outLen;
}

bool settingsFromJson(const char* json, Settings& s) {
    JsonDocument doc;
    if (deserializeJson(doc, json)) return false;
    s.ssid          = doc["ssid"]          | "";
    s.password      = doc["password"]      | "";
    s.token         = doc["token"]         | "";
    s.codexToken        = doc["codexToken"]        | "";
    s.codexAccountId    = doc["codexAccountId"]    | "";
    s.codexRefreshToken = doc["codexRefreshToken"] | "";
    s.pollSeconds   = doc["pollSeconds"]   | (uint16_t)120;
    s.alertPercent  = doc["alertPercent"]  | (uint8_t)80;
    bool hasClaude = !s.token.empty();
    bool hasCodex  = !s.codexAccountId.empty() && (!s.codexToken.empty() || !s.codexRefreshToken.empty());
    s.configured = !s.ssid.empty() && (hasClaude || hasCodex);
    return true;
}

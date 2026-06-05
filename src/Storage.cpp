#include "Storage.h"
#include <LittleFS.h>

static const char* PATH = "/settings.json";

bool Storage::begin() {
    if (LittleFS.begin()) return true;
    return LittleFS.format() && LittleFS.begin();
}
bool Storage::load(Settings& out) {
    File f = LittleFS.open(PATH, "r");
    if (!f) return false;
    String json = f.readString();
    f.close();
    return settingsFromJson(json.c_str(), out);
}
bool Storage::save(const Settings& s) {
    // Big enough for a Codex access_token (~1900 chars) plus the other fields.
    // static (not on the stack) to avoid an ESP8266 stack overflow.
    static char buf[4096];
    if (!settingsToJson(s, buf, sizeof(buf))) return false;
    File f = LittleFS.open(PATH, "w");
    if (!f) return false;
    f.print(buf);
    f.close();
    return true;
}
void Storage::wipe() { LittleFS.remove(PATH); }

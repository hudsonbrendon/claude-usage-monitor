#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif
#include <time.h>
#include "IBoard.h"
#include "Screens.h"
#include "Settings.h"
#include "Storage.h"
#include "Provider.h"
#include "providers/ClaudeProvider.h"

#include <ESP8266WebServer.h>     // portal lives inline; ESP8266-only for now
#include <DNSServer.h>

static Storage     storage;
static ClaudeProvider claudeProvider;
static Provider*      activeProvider = &claudeProvider;
static Settings    settings;
static UsageStatus status;
static char        lastError[32] = "";
static unsigned long lastFetchMs = 0;
static bool        screenOn = true;

static uint32_t nowEpoch() { time_t t; time(&t); return (uint32_t)t; }

static void apName(char* out, size_t n) {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(out, n, "%s-%02X%02X", Board().apPrefix(), mac[4], mac[5]);
}

static bool connectWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(settings.ssid.c_str(), settings.password.c_str());
    Screens::connecting(Board().canvas(), settings.ssid.c_str());
    for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) delay(500);
    return WiFi.status() == WL_CONNECTED;
}

static void syncClock() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    for (int i = 0; i < 50 && nowEpoch() < 100000; i++) delay(100);
}

static void refresh() {
    if (WiFi.status() != WL_CONNECTED) connectWifi();
    status = activeProvider->fetch(String(settings.token.c_str()), lastError, sizeof(lastError));
    lastFetchMs = millis();
}

static void draw() {
    if (status.valid)
        Screens::dashboard(Board().canvas(), status, nowEpoch(),
                            WiFi.RSSI(), (int)((millis() - lastFetchMs) / 1000));
    else
        Screens::error(Board().canvas(), "API ERROR", lastError);
}

static const char PORTAL_HTML[] PROGMEM = R"HTML(<!doctype html><html><head>
<meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1">
<title>AI Usage Setup</title><style>body{font-family:system-ui;background:#15161a;
color:#e6e6e6;margin:0;padding:16px}.c{max-width:420px;margin:auto;background:#22242b;
padding:18px;border-radius:12px}h1{font-size:1.2em;margin:0 0 12px}label{display:block;
font-size:.85em;color:#9aa;margin:10px 0 4px}input,select{width:100%;box-sizing:border-box;
padding:9px;border-radius:8px;border:1px solid #3a3d47;background:#15161a;color:#e6e6e6}
button{width:100%;margin-top:16px;padding:11px;border:0;border-radius:8px;background:#d97a4a;
color:#15161a;font-weight:700}</style></head><body><div class=c><h1>AI Usage Monitor</h1>
<form method=POST action=/save><label>Wi-Fi network (2.4 GHz)</label>
<input name=ssid required maxlength=32><label>Wi-Fi password</label>
<input name=password type=password maxlength=64>
<label>Claude token (run: claude setup-token)</label>
<input name=token required maxlength=200 placeholder="sk-ant-oat01-...">
<label>Refresh</label><select name=poll><option value=60>60 s</option>
<option value=120 selected>2 min</option><option value=300>5 min</option></select>
<button type=submit>Save & restart</button></form></div></body></html>)HTML";

static void runPortalAndReboot() {
    char ap[28];
    apName(ap, sizeof(ap));
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap);
    delay(100);
    IPAddress ip = WiFi.softAPIP();
    DNSServer dns; dns.start(53, "*", ip);
    ESP8266WebServer server(80);

    char ipStr[16]; snprintf(ipStr, sizeof(ipStr), "%s", ip.toString().c_str());
    Screens::portal(Board().canvas(), ap, ipStr);

    bool done = false;
    server.on("/", HTTP_GET, [&]() { server.send_P(200, "text/html", PORTAL_HTML); });
    server.on("/save", HTTP_POST, [&]() {
        Settings s;
        s.ssid = server.arg("ssid").c_str();
        s.password = server.arg("password").c_str();
        s.token = server.arg("token").c_str();
        long p = server.arg("poll").toInt();
        s.pollSeconds = (p >= 30 && p <= 600) ? (uint16_t)p : 120;
        s.configured = !s.ssid.empty() && !s.token.empty();
        if (!s.configured) { server.send(400, "text/plain", "ssid and token required"); return; }
        if (!storage.save(s)) { server.send(500, "text/plain", "save failed"); return; }
        server.send(200, "text/html", "<meta charset=utf-8>Saved. Restarting...");
        done = true;
    });
    server.onNotFound([&]() {
        server.sendHeader("Location", String("http://") + ip.toString());
        server.send(302, "text/plain", "");
    });
    server.begin();

    while (!done) { dns.processNextRequest(); server.handleClient(); delay(2); }
    delay(800);
    Screens::splash(Board().canvas(), "Saved. Rebooting");
    delay(800);
    ESP.restart();
}

void setup() {
    Serial.begin(115200);
    delay(50);
    Board().begin();
    Screens::splash(Board().canvas(), "starting...");
    storage.begin();

    if (!storage.load(settings) || !settings.configured) {
        runPortalAndReboot();   // does not return
    }

    if (!connectWifi()) {
        Screens::error(Board().canvas(), "WIFI FAILED", settings.ssid.c_str());
        delay(4000);
        ESP.restart();
    }
    Screens::splash(Board().canvas(), "syncing time");
    syncClock();
    Screens::splash(Board().canvas(), "fetching usage");
    refresh();
    draw();
}

void loop() {
    Board().poll();

    if (Board().held(5000)) {                 // factory reset
        Screens::error(Board().canvas(), "FACTORY RESET", "wiping...");
        storage.wipe();
        delay(1500);
        ESP.restart();
    }
    if (Board().tapped()) {                    // toggle screen
        screenOn = !screenOn;
        Board().brightness(screenOn ? 2 : 0);
    }
    if (Board().longPressed()) {               // force refresh
        Screens::splash(Board().canvas(), "refreshing...");
        refresh();
    }

    uint32_t pollMs = (uint32_t)settings.pollSeconds * 1000u;
    if (millis() - lastFetchMs >= pollMs) refresh();

    static unsigned long lastDraw = 0;
    if (screenOn && millis() - lastDraw > 5000) { draw(); lastDraw = millis(); }
    delay(20);
}

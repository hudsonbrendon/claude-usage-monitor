#include "ClaudeProvider.h"
#include "globalsign_root.h"

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecureBearSSL.h>
  #include <ESP8266HTTPClient.h>
#else
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include <HTTPClient.h>
#endif


static const char* ENDPOINT = "https://api.anthropic.com/v1/messages";

UsageStatus ClaudeProvider::fetch(const Settings& s, char* errOut, size_t errLen) {
    String token(s.token.c_str());
    UsageStatus bad;

#if defined(ESP8266)
    static BearSSL::X509List ca(GLOBALSIGN_ROOT_CA);
    BearSSL::WiFiClientSecure client;
    client.setTrustAnchors(&ca);
#else
    WiFiClientSecure client;
    client.setCACert(GLOBALSIGN_ROOT_CA);
#endif

    HTTPClient https;
    if (!https.begin(client, ENDPOINT)) { snprintf(errOut, errLen, "tls_init"); return bad; }
    https.addHeader("Authorization", String("Bearer ") + token);
    https.addHeader("anthropic-version", "2023-06-01");
    https.addHeader("anthropic-beta", "oauth-2025-04-20");
    https.addHeader("content-type", "application/json");
    https.addHeader("User-Agent", "ai-usage-monitor/1.0");
    https.setTimeout(15000);

    const char* keys[] = {
        "anthropic-ratelimit-unified-5h-utilization",
        "anthropic-ratelimit-unified-5h-reset",
        "anthropic-ratelimit-unified-7d-utilization",
        "anthropic-ratelimit-unified-7d-reset",
    };
    https.collectHeaders(keys, 4);

    int code = https.POST(
        "{\"model\":\"claude-haiku-4-5-20251001\",\"max_tokens\":1,"
        "\"messages\":[{\"role\":\"user\",\"content\":\".\"}]}");
    if (code <= 0) { snprintf(errOut, errLen, "http_%d", code); https.end(); return bad; }

    String h5u = https.header(keys[0]), h5r = https.header(keys[1]);
    String d7u = https.header(keys[2]), d7r = https.header(keys[3]);
    https.end();

    if (h5u.isEmpty() && d7u.isEmpty()) {
        snprintf(errOut, errLen, code == 401 ? "auth_failed" : "no_usage_%d", code);
        return bad;
    }
    return parseUsage(h5u.c_str(), h5r.c_str(), d7u.c_str(), d7r.c_str());
}

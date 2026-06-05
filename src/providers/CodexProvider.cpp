#include "CodexProvider.h"
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

static const char* CODEX_ENDPOINT = "https://chatgpt.com/backend-api/codex/usage";

UsageStatus CodexProvider::fetch(const Settings& s, char* errOut, size_t errLen) {
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
    if (!https.begin(client, CODEX_ENDPOINT)) { snprintf(errOut, errLen, "tls_init"); return bad; }
    https.addHeader("Authorization", String("Bearer ") + s.codexToken.c_str());
    https.addHeader("chatgpt-account-id", s.codexAccountId.c_str());
    https.addHeader("originator", "codex_cli_rs");
    https.addHeader("OpenAI-Beta", "responses=experimental");
    https.addHeader("User-Agent", "codex_cli_rs/0.137.0 (ai-usage-monitor; esp8266)");
    https.addHeader("Accept", "application/json");
    https.setTimeout(15000);

    int code = https.GET();
    if (code <= 0) { snprintf(errOut, errLen, "http_%d", code); https.end(); return bad; }
    if (code != 200) { snprintf(errOut, errLen, code == 401 ? "auth_failed" : "http_%d", code); https.end(); return bad; }

    String body = https.getString();
    https.end();

    UsageStatus u = parseCodexUsage(body.c_str());
    if (!u.valid) { snprintf(errOut, errLen, "no_usage"); return bad; }
    return u;
}

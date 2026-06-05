#include "ClaudeProvider.h"

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecureBearSSL.h>
  #include <ESP8266HTTPClient.h>
#else
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
  #include <HTTPClient.h>
#endif

// GlobalSign Root CA R1 — trust anchor for api.anthropic.com (public certificate).
static const char ROOT_CA[] PROGMEM = R"CA(
-----BEGIN CERTIFICATE-----
MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG
A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv
b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw
MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i
YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT
aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ
jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp
xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp
1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG
snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ
U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8
9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E
BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B
AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz
yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE
38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP
AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad
DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME
HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==
-----END CERTIFICATE-----
)CA";

static const char* ENDPOINT = "https://api.anthropic.com/v1/messages";

UsageStatus ClaudeProvider::fetch(const String& token, char* errOut, size_t errLen) {
    UsageStatus bad;

#if defined(ESP8266)
    static BearSSL::X509List ca(ROOT_CA);
    BearSSL::WiFiClientSecure client;
    client.setTrustAnchors(&ca);
#else
    WiFiClientSecure client;
    client.setCACert(ROOT_CA);
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

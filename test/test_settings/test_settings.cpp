#include <unity.h>
#include "../../src/Settings.h"

void test_roundtrip(void) {
    Settings a;
    a.ssid="MyNet"; a.password="secret"; a.token="sk-ant-oat01-abc"; a.pollSeconds=60;
    char buf[512];
    TEST_ASSERT_TRUE(settingsToJson(a, buf, sizeof(buf)));
    Settings b;
    TEST_ASSERT_TRUE(settingsFromJson(buf, b));
    TEST_ASSERT_EQUAL_STRING("MyNet", b.ssid.c_str());
    TEST_ASSERT_EQUAL_STRING("secret", b.password.c_str());
    TEST_ASSERT_EQUAL_STRING("sk-ant-oat01-abc", b.token.c_str());
    TEST_ASSERT_EQUAL_UINT16(60, b.pollSeconds);
    TEST_ASSERT_TRUE(b.configured);
}
void test_missing_token_not_configured(void) {
    Settings b;
    TEST_ASSERT_TRUE(settingsFromJson("{\"ssid\":\"X\"}", b));
    TEST_ASSERT_FALSE(b.configured);
}
void test_malformed_json_fails(void) {
    Settings b;
    TEST_ASSERT_FALSE(settingsFromJson("{not json", b));
}
void test_default_poll_when_absent(void) {
    Settings b;
    TEST_ASSERT_TRUE(settingsFromJson("{\"ssid\":\"X\",\"token\":\"t\"}", b));
    TEST_ASSERT_EQUAL_UINT16(120, b.pollSeconds);
    TEST_ASSERT_TRUE(b.configured);
}
void test_codex_roundtrip(void) {
    Settings a;
    a.ssid="N"; a.codexToken="cx-abc"; a.codexAccountId="acc-1";
    char buf[512];
    TEST_ASSERT_TRUE(settingsToJson(a, buf, sizeof(buf)));
    Settings b;
    TEST_ASSERT_TRUE(settingsFromJson(buf, b));
    TEST_ASSERT_EQUAL_STRING("cx-abc", b.codexToken.c_str());
    TEST_ASSERT_EQUAL_STRING("acc-1", b.codexAccountId.c_str());
    TEST_ASSERT_TRUE(b.configured);
}
void test_configured_requires_a_provider(void) {
    Settings b;
    TEST_ASSERT_TRUE(settingsFromJson("{\"ssid\":\"N\"}", b));
    TEST_ASSERT_FALSE(b.configured);
}
void test_alert_percent_roundtrip(void) {
    Settings a; a.ssid="N"; a.token="t"; a.alertPercent=90;
    char buf[512]; TEST_ASSERT_TRUE(settingsToJson(a, buf, sizeof(buf)));
    Settings b; TEST_ASSERT_TRUE(settingsFromJson(buf, b));
    TEST_ASSERT_EQUAL_UINT8(90, b.alertPercent);
}
void test_alert_percent_default(void) {
    Settings b; TEST_ASSERT_TRUE(settingsFromJson("{\"ssid\":\"N\",\"token\":\"t\"}", b));
    TEST_ASSERT_EQUAL_UINT8(80, b.alertPercent);
}
void test_codex_refresh_roundtrip(void) {
    Settings a; a.ssid="N"; a.codexAccountId="acc"; a.codexRefreshToken="rt-123";
    char buf[1024]; TEST_ASSERT_TRUE(settingsToJson(a, buf, sizeof(buf)));
    Settings b; TEST_ASSERT_TRUE(settingsFromJson(buf, b));
    TEST_ASSERT_EQUAL_STRING("rt-123", b.codexRefreshToken.c_str());
    TEST_ASSERT_TRUE(b.configured);
}
void setUp(void) {}
void tearDown(void) {}
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_roundtrip);
    RUN_TEST(test_missing_token_not_configured);
    RUN_TEST(test_malformed_json_fails);
    RUN_TEST(test_default_poll_when_absent);
    RUN_TEST(test_codex_roundtrip);
    RUN_TEST(test_configured_requires_a_provider);
    RUN_TEST(test_alert_percent_roundtrip);
    RUN_TEST(test_alert_percent_default);
    RUN_TEST(test_codex_refresh_roundtrip);
    return UNITY_END();
}

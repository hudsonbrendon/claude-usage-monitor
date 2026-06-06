#include <unity.h>
#include "../../src/Usage.h"

void test_parse_typical(void) {
    UsageStatus s = parseUsage("0.42", "1750000000", "0.7", "1750500000");
    TEST_ASSERT_TRUE(s.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 42.0f, s.h5Percent);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 70.0f, s.d7Percent);
    TEST_ASSERT_EQUAL_UINT32(1750000000u, s.h5Reset);
    TEST_ASSERT_EQUAL_UINT32(1750500000u, s.d7Reset);
}
void test_parse_empty_is_invalid(void) {
    UsageStatus s = parseUsage("", "", "", "");
    TEST_ASSERT_FALSE(s.valid);
}
void test_parse_null_is_invalid(void) {
    UsageStatus s = parseUsage(nullptr, nullptr, nullptr, nullptr);
    TEST_ASSERT_FALSE(s.valid);
}
void test_parse_partial_5h_only(void) {
    UsageStatus s = parseUsage("0.05", "1750000000", "", "");
    TEST_ASSERT_TRUE(s.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, s.h5Percent);
    TEST_ASSERT_EQUAL_UINT32(0u, s.d7Reset);
}
void test_countdown_days(void) {
    char b[16]; formatCountdown(1000000u+3u*86400u+4u*3600u, 1000000u, b, sizeof(b));
    TEST_ASSERT_EQUAL_STRING("3d4h", b);
}
void test_countdown_hours_minutes(void) {
    char b[16]; formatCountdown(1000000u+2u*3600u+5u*60u, 1000000u, b, sizeof(b));
    TEST_ASSERT_EQUAL_STRING("2h05m", b);
}
void test_countdown_minutes(void) {
    char b[16]; formatCountdown(1000000u+12u*60u, 1000000u, b, sizeof(b));
    TEST_ASSERT_EQUAL_STRING("12m", b);
}
void test_countdown_now(void) {
    char b[16]; formatCountdown(1000000u, 1000000u, b, sizeof(b));
    TEST_ASSERT_EQUAL_STRING("now", b);
}
void test_countdown_unknown(void) {
    char b[16]; formatCountdown(0u, 1000000u, b, sizeof(b));
    TEST_ASSERT_EQUAL_STRING("--", b);
}
void test_codex_parse_typical(void) {
    const char* j = "{\"plan_type\":\"plus\",\"rate_limit\":{\"allowed\":true,"
        "\"primary_window\":{\"used_percent\":65,\"reset_at\":1781000000},"
        "\"secondary_window\":{\"used_percent\":62,\"reset_at\":1781500000}}}";
    UsageStatus s = parseCodexUsage(j);
    TEST_ASSERT_TRUE(s.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 65.0f, s.h5Percent);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 62.0f, s.d7Percent);
    TEST_ASSERT_EQUAL_UINT32(1781000000u, s.h5Reset);
    TEST_ASSERT_EQUAL_UINT32(1781500000u, s.d7Reset);
}
void test_codex_parse_no_ratelimit_invalid(void) {
    UsageStatus s = parseCodexUsage("{\"plan_type\":\"plus\"}");
    TEST_ASSERT_FALSE(s.valid);
}
void test_codex_parse_malformed_invalid(void) {
    UsageStatus s = parseCodexUsage("{not json");
    TEST_ASSERT_FALSE(s.valid);
}
void test_over_threshold_hit(void) { UsageStatus u; u.valid=true; u.h5Percent=82; u.d7Percent=10; TEST_ASSERT_TRUE(overThreshold(u,80)); }
void test_over_threshold_weekly(void) { UsageStatus u; u.valid=true; u.h5Percent=10; u.d7Percent=95; TEST_ASSERT_TRUE(overThreshold(u,80)); }
void test_over_threshold_under(void) { UsageStatus u; u.valid=true; u.h5Percent=50; u.d7Percent=50; TEST_ASSERT_FALSE(overThreshold(u,80)); }
void test_over_threshold_disabled(void) { UsageStatus u; u.valid=true; u.h5Percent=99; u.d7Percent=99; TEST_ASSERT_FALSE(overThreshold(u,0)); }
void test_over_threshold_invalid(void) { UsageStatus u; u.valid=false; u.h5Percent=99; TEST_ASSERT_FALSE(overThreshold(u,80)); }
void setUp(void) {}
void tearDown(void) {}
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_typical);
    RUN_TEST(test_parse_empty_is_invalid);
    RUN_TEST(test_parse_null_is_invalid);
    RUN_TEST(test_parse_partial_5h_only);
    RUN_TEST(test_countdown_days);
    RUN_TEST(test_countdown_hours_minutes);
    RUN_TEST(test_countdown_minutes);
    RUN_TEST(test_countdown_now);
    RUN_TEST(test_countdown_unknown);
    RUN_TEST(test_codex_parse_typical);
    RUN_TEST(test_codex_parse_no_ratelimit_invalid);
    RUN_TEST(test_codex_parse_malformed_invalid);
    RUN_TEST(test_over_threshold_hit);
    RUN_TEST(test_over_threshold_weekly);
    RUN_TEST(test_over_threshold_under);
    RUN_TEST(test_over_threshold_disabled);
    RUN_TEST(test_over_threshold_invalid);
    return UNITY_END();
}

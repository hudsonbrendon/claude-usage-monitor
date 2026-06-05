#pragma once
#include "../Provider.h"

// Anthropic provider: probes the Messages API and reads the
// anthropic-ratelimit-unified-5h/7d-* response headers.
class ClaudeProvider : public Provider {
public:
    const char* id() override { return "Claude"; }
    bool available(const Settings& s) override { return !s.token.empty(); }
    UsageStatus fetch(const Settings& s, char* errOut, size_t errLen) override;
};

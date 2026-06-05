#pragma once
#include "../Provider.h"

class CodexProvider : public Provider {
public:
    const char* id() override { return "Codex"; }
    bool available(const Settings& s) override {
        return !s.codexToken.empty() && !s.codexAccountId.empty();
    }
    UsageStatus fetch(const Settings& s, char* errOut, size_t errLen) override;
};

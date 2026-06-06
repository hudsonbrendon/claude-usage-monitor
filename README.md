<p align="center">
  <img src="assets/preview.png" alt="AI Usage Monitor" width="480">
</p>

<h1 align="center">AI Usage Monitor</h1>

<p align="center">Live AI agent usage — your rolling 5-hour and weekly rate-limit utilization, with reset countdowns, on a tiny ESP8266 OLED.</p>

<p align="center">
  <a href="https://github.com/hudsonbrendon/ai-usage-monitor/actions/workflows/tests.yml"><img src="https://github.com/hudsonbrendon/ai-usage-monitor/actions/workflows/tests.yml/badge.svg" alt="Tests"></a>
  <a href="https://github.com/hudsonbrendon/ai-usage-monitor/releases"><img src="https://img.shields.io/github/v/release/hudsonbrendon/ai-usage-monitor" alt="Release"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/PlatformIO-ESP8266-orange" alt="PlatformIO ESP8266">
</p>

A from-scratch ESP8266 firmware that shows your AI agent usage on a tiny 0.96" OLED: rolling 5-hour and weekly rate-limit utilization bars, each with a reset countdown, so you always know how much headroom you have left and when it refills. Provider data is fetched through a pluggable `Provider` interface, making it straightforward to add new AI services. Every screen is drawn through a `Canvas`/`IBoard` abstraction, so the display logic is completely board-agnostic and can target different hardware without touching the providers or business logic.

## Providers

| Provider | Status | Data source |
|---|---|---|
| Claude (Anthropic) | ✅ supported | `anthropic-ratelimit-unified-5h/7d-*` headers on the Messages API |
| Codex (OpenAI) | ✅ supported | `chatgpt.com/backend-api/codex/usage` (5h + weekly windows) |

## Quick start

1. **Flash a release** — grab the latest `firmware.bin` from [Releases](https://github.com/hudsonbrendon/ai-usage-monitor/releases) and follow [Installation](docs/INSTALL.md) to flash it with `esptool.py` (no toolchain required).
2. **Set it up over the Wi-Fi portal** — on first boot the device opens an AP; join it, open the captive portal, enter your 2.4 GHz Wi-Fi credentials and at least one provider token. See [Usage](docs/USAGE.md) for the full walkthrough.

You need a 2.4 GHz Wi-Fi network and a valid token for at least one provider (Claude session token or Codex access token + account ID).

## Documentation

- [Installation](docs/INSTALL.md) — flash a prebuilt build or build from source
- [Usage](docs/USAGE.md) — portal setup, the dashboard, button controls
- [Providers](docs/PROVIDERS.md) — Claude & Codex: credentials, how they work, caveats
- [Architecture](docs/ARCHITECTURE.md) — the Canvas / IBoard / Provider design
- [Development](docs/DEVELOPMENT.md) — build, test, CI/release
- [Extending](docs/EXTENDING.md) — add a board or a provider
- [Troubleshooting](docs/TROUBLESHOOTING.md) — common issues and fixes
- [Contributing](CONTRIBUTING.md) — how to help

## Hardware

**ideaspark ESP8266** (ESP-12S) with an integrated **SSD1306 128x64 I2C OLED** (VR:2.1). The OLED is wired to **SDA = GPIO12** and **SCL = GPIO14**; the onboard **FLASH** button is on **GPIO0**.

## License

MIT © Hudson Brendon

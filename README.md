<p align="center">
  <img src="assets/banner.png" alt="claude-usage-monitor" width="480">
</p>

<h1 align="center">Claude Usage Monitor</h1>

<p align="center">Live Claude API usage — your 5-hour and 7-day rate-limit utilization, with reset countdowns, on a tiny ESP8266 OLED.</p>

<p align="center">
  <a href="https://github.com/hudsonbrendon/claude-usage-monitor/actions/workflows/tests.yml"><img src="https://github.com/hudsonbrendon/claude-usage-monitor/actions/workflows/tests.yml/badge.svg" alt="Tests"></a>
  <a href="https://github.com/hudsonbrendon/claude-usage-monitor/releases"><img src="https://img.shields.io/github/v/release/hudsonbrendon/claude-usage-monitor" alt="Release"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/PlatformIO-ESP8266-orange" alt="PlatformIO ESP8266">
</p>

A from-scratch firmware that polls the Anthropic API and shows your Claude Code usage on a 0.96" OLED. Designed to start small (one board) and grow: a `Canvas`/`IBoard` abstraction keeps every screen board-agnostic, so new boards are additive.

## Features

- 5-hour and 7-day rate-limit utilization as on-screen bars, with reset countdowns.
- One-time setup over a captive Wi-Fi portal — no hardcoded credentials.
- Single-button UX: tap = screen on/off, long-press = refresh, hold 5 s = factory reset.
- Board-agnostic `Canvas`/`IBoard` design, ready to extend to more boards.
- Host-side unit tests for the pure logic (parsing, settings) via PlatformIO `native`.

## Hardware

- **ideaspark ESP8266** (ESP-12S) with an integrated **SSD1306 128x64 I2C OLED** (VR:2.1).
- OLED wired to **SDA = GPIO12, SCL = GPIO14**; the onboard **FLASH** button is on **GPIO0**.

## Build & flash (PlatformIO)

```bash
pio test -e native        # host unit tests
pio run  -e ideaspark     # build firmware
pio run  -e ideaspark -t upload
```

## Flash a release build (no toolchain)

Grab the latest `firmware.bin` from [Releases](https://github.com/hudsonbrendon/claude-usage-monitor/releases) and flash it at offset `0x0`:

```bash
esptool.py --chip esp8266 --baud 460800 write_flash \
  --flash_mode dio --flash_freq 40m --flash_size detect \
  0x0 claude-usage-monitor-ideaspark-vX.Y.Z.bin
```

## Setup

1. On first boot the device opens an open Wi-Fi AP `ClaudeUsage-XXXX`.
2. Join it and open `http://192.168.4.1`.
3. Enter your 2.4 GHz Wi-Fi and a Claude token (`claude setup-token`).
4. It reboots and shows the dashboard.

## How it works

The firmware probes `https://api.anthropic.com/v1/messages` with your OAuth token and reads the `anthropic-ratelimit-unified-*` response headers to render the 5h / 7d utilization. Settings live in LittleFS; TLS uses BearSSL on the ESP8266.

## Adding more boards

Boards are additive — see [`docs/EXTENDING.md`](docs/EXTENDING.md). In short: drop a `src/boards/<name>.cpp` implementing `IBoard`, add a PlatformIO env, and (only for a new display technology) a `Canvas` subclass. No screen or logic code changes.

## Credits

- The Anthropic API and its `anthropic-ratelimit-unified-*` usage headers.
- [U8g2](https://github.com/olikraus/u8g2) for OLED rendering and [ArduinoJson](https://arduinojson.org/) for settings.

## License

MIT © Hudson Brendon

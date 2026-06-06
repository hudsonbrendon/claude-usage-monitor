<p align="center">
  <img src="assets/preview.png" alt="AI Usage Monitor" width="480">
</p>

<h1 align="center">AI Usage Monitor</h1>

<p align="center">A tiny desk gadget that shows how much of your AI coding quota is left — at a glance, no browser tab required.</p>

<p align="center">
  <a href="https://github.com/hudsonbrendon/ai-usage-monitor/actions/workflows/tests.yml"><img src="https://github.com/hudsonbrendon/ai-usage-monitor/actions/workflows/tests.yml/badge.svg" alt="Tests"></a>
  <a href="https://github.com/hudsonbrendon/ai-usage-monitor/releases"><img src="https://img.shields.io/github/v/release/hudsonbrendon/ai-usage-monitor" alt="Release"></a>
  <a href="https://hudsonbrendon.github.io/ai-usage-monitor/"><img src="https://img.shields.io/badge/docs-mkdocs--material-blue" alt="Docs"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/PlatformIO-ESP8266-orange" alt="PlatformIO ESP8266">
  <img src="https://img.shields.io/badge/providers-Claude%20%2B%20Codex-8957e5" alt="Providers">
</p>

If you code with **Claude Code** or **Codex**, you live inside rolling usage windows — a
5‑hour limit and a weekly one. Hit them at the wrong moment and your flow stops cold.
**AI Usage Monitor** puts those numbers on a little OLED on your desk: two live bars, the
exact percentage used, and a countdown to when each window refills. Glance over, know your
headroom, keep coding.

It runs on a **~$6 ESP8266 board with a built‑in screen**, sets up in two minutes over a
Wi‑Fi captive portal (no soldering, no app), and shows **either provider at the tap of a
button**.

---

## ✨ Features

- 📊 **Two live windows** — rolling **5‑hour** and **weekly** rate‑limit utilization as on‑screen bars with exact percentages.
- ⏳ **Reset countdowns** — see exactly when each window refills (`2h05m`, `3d4h`).
- 🔀 **Multi‑provider** — **Claude (Anthropic)** and **Codex (OpenAI)** today; tap the button to switch between them on the same device.
- 📶 **Two‑minute setup** — a captive Wi‑Fi portal asks for your network and tokens; nothing is hardcoded, nothing is soldered.
- 🧩 **Pluggable by design** — a `Provider` seam makes new AI services additive, and a `Canvas`/`IBoard` seam makes new boards/displays additive.
- 🔌 **Cheap, self‑contained hardware** — one ESP8266 board with an integrated OLED, powered from any USB port.
- ✅ **Tested & shipped** — pure logic is unit‑tested on the host (CI green), and tagged releases ship a ready‑to‑flash `firmware.bin`.

## 🖥️ What you see

```
┌────────────────────────────┐
│ 5H  41%              1h26m │
│ ▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░ │
│ 7D  15%              5d23h │
│ ▓▓▓▓░░░░░░░░░░░░░░░░░░░░░░ │
│ ────────────────────────── │
│ Claude   -54dBm   12s      │
└────────────────────────────┘
```

The top bar is the **5‑hour** window, the second is the **weekly** window, each with the
percent used and a reset countdown. The bottom line shows the **active provider**, Wi‑Fi
signal, and how long ago the data was fetched. **Tap the button** and it flips to the other
provider.

## 🤖 Providers

| Provider | Status | Data source |
|----------|--------|-------------|
| **Claude** (Anthropic) | ✅ supported | `anthropic-ratelimit-unified-5h/7d-*` headers on the Messages API |
| **Codex** (OpenAI) | ✅ supported | `chatgpt.com/backend-api/codex/usage` (5h + weekly windows) |

You need a token for **at least one** provider — or add both and toggle. See
[Providers](docs/PROVIDERS.md) for exactly how each works and how to get its credentials.

## 🔧 Hardware

- **ideaspark ESP8266** (ESP‑12S) with an integrated **SSD1306 128×64 I²C OLED** (VR:2.1) — sold as one small board for a few dollars.
- A **micro‑USB data cable** and any USB power source.
- A **2.4 GHz Wi‑Fi** network (the ESP8266 is 2.4 GHz only).

No wiring or soldering — the screen and button are already on the board.

## 🚀 Installation

### Option A — Flash a prebuilt release (no toolchain)

1. Download `ai-usage-monitor-ideaspark-<version>.bin` from the [latest release](https://github.com/hudsonbrendon/ai-usage-monitor/releases/latest).
2. Install [esptool](https://github.com/espressif/esptool): `pip install esptool`.
3. Flash it (find your port with `ls /dev/cu.usbserial-*`):

```bash
esptool.py --chip esp8266 --port /dev/cu.usbserial-XXXX --baud 460800 write_flash \
  --flash_mode dio --flash_freq 40m --flash_size detect \
  0x0 ai-usage-monitor-ideaspark-<version>.bin
```

### Option B — Build from source (PlatformIO)

```bash
git clone https://github.com/hudsonbrendon/ai-usage-monitor.git
cd ai-usage-monitor
pio run -e ideaspark -t upload   # build + flash
```

Full walkthrough, including the macOS CH340 driver note, is in [Installation](docs/INSTALL.md).

## 📶 Setup

1. On first boot the device opens an open Wi‑Fi access point named **`AIUsage-XXXX`**.
2. Join it and open **`http://192.168.4.1`**.
3. Enter your 2.4 GHz Wi‑Fi and at least one provider's credentials:
   - **Claude** — the token from `claude setup-token`.
   - **Codex** — the `access_token` and `account_id` from `~/.codex/auth.json` (`jq -r '.tokens.access_token' ~/.codex/auth.json`).
4. Save — it reboots and starts showing your usage.

The full setup guide is in [Usage](docs/USAGE.md).

## 🎛️ Controls

One button does everything:

| Gesture | Action |
|---------|--------|
| **Tap** | Switch to the next configured provider |
| **Long‑press** (hold ~0.6–5 s, release) | Refresh now |
| **Hold ≥ 5 s** | Factory reset (wipe settings, reopen the portal) |

## 📚 Documentation

- [Installation](docs/INSTALL.md) — flash a prebuilt build or build from source
- [Usage](docs/USAGE.md) — portal setup, the dashboard, button controls
- [Providers](docs/PROVIDERS.md) — Claude & Codex: credentials, how they work, caveats
- [Architecture](docs/ARCHITECTURE.md) — the Canvas / IBoard / Provider design
- [Development](docs/DEVELOPMENT.md) — build, test, CI/release
- [Extending](docs/EXTENDING.md) — add a board or a provider
- [Troubleshooting](docs/TROUBLESHOOTING.md) — common issues and fixes
- [Contributing](CONTRIBUTING.md) — how to help

## 🛠️ Development

```bash
pio test -e native        # host unit tests (parsing, settings)
pio run  -e ideaspark     # build the firmware
```

Pure logic is Arduino‑independent and unit‑tested on the host; device code lives behind the
`Canvas` / `IBoard` / `Provider` seams. See [Development](docs/DEVELOPMENT.md) and
[Architecture](docs/ARCHITECTURE.md).

## 🔒 A note on credentials

Your provider tokens are stored **in plaintext** on the device's flash (no PIN/encryption),
and a factory reset wipes them. Treat the stick as sensitively as the tokens it holds — details in [Providers → Security](docs/PROVIDERS.md).

## 🤝 Contributing

Contributions are welcome — new providers and boards are meant to be additive. Start with
[Contributing](CONTRIBUTING.md) and [Extending](docs/EXTENDING.md).

## 📄 License

MIT © Hudson Brendon

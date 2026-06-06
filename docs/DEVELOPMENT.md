# Development

This guide covers everything you need to build, test, and flash the
ai-usage-monitor firmware, understand the project layout, and contribute
changes. All commands assume you are in the repository root unless stated
otherwise.

---

## Prerequisites

| Tool | Purpose | Install |
|------|---------|---------|
| [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) | Build system, dependency manager, test runner | `pip install platformio` |
| Python 3.8+ | Required by PlatformIO and the optional headless-flash scripts | ships with most OSes; use `python3 --version` to verify |
| USB data cable (CH340-compatible) | Flashing the ESP8266 | the cable that came with your board must carry data, not charge-only |

Verify PlatformIO after installation:

```bash
pio --version
```

---

## Project layout

```
ai-usage-monitor/
├── .github/
│   ├── dependabot.yml          # weekly auto-update for GitHub Actions versions
│   └── workflows/
│       ├── tests.yml           # run native tests + firmware build on every push/PR
│       └── release.yml         # build firmware.bin and attach it to a GitHub Release on v* tags
│
├── docs/
│   ├── DEVELOPMENT.md          # this file
│   ├── EXTENDING.md            # how to add boards and providers
│   ├── INSTALL.md              # hardware assembly and first-boot Wi-Fi setup
│   ├── PROVIDERS.md            # per-provider API key and endpoint notes
│   └── USAGE.md                # day-to-day operation
│
├── src/
│   ├── main.cpp                # Arduino setup()/loop(); wires everything together
│   ├── Canvas.h                # abstract drawing interface (text, box, hline, clear, present)
│   ├── MonoCanvas.h / .cpp     # Canvas implementation over U8g2 (mono OLEDs)
│   ├── Screens.h / .cpp        # all on-device screens; depends only on Canvas + domain types
│   ├── IBoard.h                # abstract board interface (display, buttons)
│   ├── Provider.h              # abstract provider interface (fetch usage)
│   ├── Settings.h / .cpp       # configuration parsing/serialization — Arduino-independent
│   ├── Usage.h / .cpp          # usage data model and JSON parsing — Arduino-independent
│   ├── Storage.h / .cpp        # LittleFS persistence layer
│   ├── boards/
│   │   └── ideaspark.cpp       # IBoard implementation for the IdeaSpark ESP8266+OLED module
│   └── providers/
│       ├── ClaudeProvider.h / .cpp   # Anthropic Claude usage API client
│       ├── CodexProvider.h / .cpp    # OpenAI Codex/usage API client
│       └── globalsign_root.h         # embedded GlobalSign root CA for TLS (BearSSL)
│
├── test/
│   ├── test_usage/
│   │   └── test_usage.cpp      # Unity unit tests for Usage parsing logic
│   └── test_settings/
│       └── test_settings.cpp   # Unity unit tests for Settings parsing/serialization
│
├── assets/
│   └── preview.png             # screenshot used in the README
│
├── platformio.ini              # two envs: ideaspark (device) and native (host tests)
├── README.md
└── LICENSE
```

---

## Build, test, flash

### Run the host unit tests

```bash
pio test -e native
```

This compiles `Settings.cpp` and `Usage.cpp` together with the test suites
under `test/` using the host toolchain — no device required. The
`test_build_src = yes` setting in `platformio.ini` is what makes PlatformIO
pull `src/` source files into the native test binary; without it only the test
files themselves would compile.

### Build the firmware

```bash
pio run -e ideaspark
```

The resulting binary is written to `.pio/build/ideaspark/firmware.bin`.

### Flash over USB

Plug in the board. PlatformIO auto-detects the port on most systems:

```bash
pio run -e ideaspark -t upload
```

If you have several serial devices attached, specify the port explicitly:

```bash
pio run -e ideaspark -t upload --upload-port /dev/cu.usbserial-XXXX
```

---

## Flashing in a headless shell

`pio run -t upload` and `pio device monitor` require an interactive TTY. In CI,
Docker containers, or SSH sessions without a pseudo-terminal they will hang or
error out. Use the two approaches below instead.

### Flash with esptool directly

```bash
esptool.py \
  --chip esp8266 \
  --port /dev/cu.usbserial-XXXX \
  --baud 460800 \
  write_flash \
  --flash_mode dio \
  --flash_freq 40m \
  --flash_size detect \
  0x0 .pio/build/ideaspark/firmware.bin
```

Replace `/dev/cu.usbserial-XXXX` with your actual port (`ls /dev/cu.*` on macOS,
`ls /dev/ttyUSB*` on Linux).

### Read serial output with pyserial

The CH340 adapter holds the board in reset when DTR/RTS are left at their
default levels. Use a small script that lowers both lines first, then pulses RTS
to trigger a clean reset:

```python
import serial, time

port = "/dev/cu.usbserial-XXXX"   # adjust to your port
with serial.Serial(port, 115200, timeout=1) as s:
    s.dtr = False
    s.rts = False
    time.sleep(0.1)
    s.rts = True
    time.sleep(0.1)
    s.rts = False
    while True:
        line = s.readline()
        if line:
            print(line.decode(errors="replace"), end="")
```

---

## Continuous integration & releases

### tests.yml

Triggered on every push to `main` and on every pull request. The workflow:

1. Installs PlatformIO via `pip`.
2. Runs `pio test -e native` — the host unit tests must pass.
3. Runs `pio run -e ideaspark` — the firmware must build cleanly.

A red check on either step blocks merge.

### release.yml

Triggered when you push a tag that matches `v*` (e.g. `v1.2.0`). The workflow:

1. Builds the firmware with `pio run -e ideaspark`.
2. Copies the binary to `dist/ai-usage-monitor-ideaspark-<tag>.bin`.
3. Creates a GitHub Release via `softprops/action-gh-release`, attaches the
   `.bin` as a release asset, and auto-generates release notes from the commits
   since the previous tag.

To cut a release:

```bash
git tag v1.2.0
git push origin v1.2.0
```

The CI job handles everything else. Download the `.bin` from the GitHub
Releases page to flash onto any board without a local PlatformIO installation.

### dependabot.yml

Dependabot opens weekly pull requests to keep the `uses:` action versions in
both workflow files up to date. Commits from those PRs carry the `ci` prefix
and the `dependencies` label.

---

## Conventions

**Host-testable pure logic.** `Usage` and `Settings` are plain C++ with no
Arduino headers. Keep them that way: any new parsing, validation, or
serialization logic belongs in these files (or similarly Arduino-independent
files) so it can be covered by `pio test -e native` without a device.

**Seams.** Device-specific code lives behind the three seams defined in
`Canvas.h`, `IBoard.h`, and `Provider.h`. `Screens` knows nothing about
hardware; it calls only `Canvas` methods. Adding a new board means implementing
`IBoard` and wiring in a `Canvas` subclass — nothing else changes. See
[Extending](EXTENDING.md) for step-by-step instructions.

**Commits.** Keep commits small and write the subject line in the imperative
mood: `Add CodexProvider`, `Fix token reset at month boundary`, `Bump U8g2 to
2.36.5`. One logical change per commit.

**Versioning.** Releases follow [Semantic Versioning](https://semver.org/)
(`vMAJOR.MINOR.PATCH`). Bump PATCH for bug fixes, MINOR for backward-compatible
additions, MAJOR for breaking changes to the settings schema or wire format.

---

## Extending

For a walkthrough of adding a new board or a new provider see
[Extending](EXTENDING.md).

---

← Back to [Home](index.md)

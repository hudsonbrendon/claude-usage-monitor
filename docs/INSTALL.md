# Installation

This guide walks you through getting the ai-usage-monitor firmware onto your ideaspark ESP8266 board so it can start displaying your Claude and Codex usage on the built-in 0.96" OLED. You have two options: flash a prebuilt binary (no compiler required) or build from source with PlatformIO.

---

## What you need

- **ideaspark ESP8266 (ESP-12S) board** with the integrated SSD1306 128×64 I2C OLED (VR:2.1)
- **Micro-USB DATA cable** — a charge-only cable will not work; make sure yours transfers data
- **2.4 GHz Wi-Fi network** — the ESP8266 does not support 5 GHz
- **Credentials for at least one provider** — see [Providers](PROVIDERS.md) for what you will need to gather before setup

> **macOS note:** The board uses a CH340 USB-to-serial bridge. On macOS 12 and earlier you may need to install the [CH340 driver](https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver) before the port appears. On macOS 13 (Ventura) and later the driver is usually bundled; if the port still does not show up, install the driver anyway and reboot.

---

## Option A — Flash a prebuilt release (no toolchain)

This is the fastest path. You only need Python and `esptool`.

### 1. Download the firmware

Go to the [Releases page](https://github.com/hudsonbrendon/ai-usage-monitor/releases) and download the latest `ai-usage-monitor-ideaspark-<version>.bin` file, where `<version>` is the release tag (for example `v1.0.0`).

### 2. Install esptool

```bash
pip install esptool
```

Verify it installed correctly:

```bash
esptool.py version
```

### 3. Find your serial port

Plug the board in with your data cable, then run:

```bash
ls /dev/cu.usbserial-*
```

You should see something like `/dev/cu.usbserial-1420`. Note that exact string — you will use it in the next step in place of `XXXX`.

### 4. Flash the firmware

Run the command below, substituting your actual port and the version you downloaded:

```bash
esptool.py --chip esp8266 \
  --port /dev/cu.usbserial-XXXX \
  --baud 460800 \
  write_flash \
  --flash_mode dio \
  --flash_freq 40m \
  --flash_size detect \
  0x0 ai-usage-monitor-ideaspark-<version>.bin
```

For example, if your port is `/dev/cu.usbserial-1420` and you downloaded `v1.2.0`:

```bash
esptool.py --chip esp8266 \
  --port /dev/cu.usbserial-1420 \
  --baud 460800 \
  write_flash \
  --flash_mode dio \
  --flash_freq 40m \
  --flash_size detect \
  0x0 ai-usage-monitor-ideaspark-v1.2.0.bin
```

The flash takes about 30 seconds. When `esptool` prints `Hash of data verified` and exits cleanly, you are done. Skip ahead to [First boot](#first-boot).

---

## Option B — Build from source (PlatformIO)

Use this path if you want to modify the firmware or track the latest commits.

### 1. Install PlatformIO Core

```bash
pip install platformio
```

Or follow the [official instructions](https://docs.platformio.org/en/latest/core/installation/index.html) if you prefer a different method.

Verify:

```bash
pio --version
```

### 2. Clone the repository

```bash
git clone https://github.com/hudsonbrendon/ai-usage-monitor.git
cd ai-usage-monitor
```

### 3. Build the firmware

```bash
pio run -e ideaspark
```

PlatformIO downloads all dependencies automatically on the first run. A successful build ends with a line like `[SUCCESS] Took N.XX seconds`.

### 4. Flash to the board

Plug in your board, then:

```bash
pio run -e ideaspark -t upload
```

PlatformIO detects the port automatically. If it cannot find the board, set the port explicitly in `platformio.ini` or pass `--upload-port /dev/cu.usbserial-XXXX`.

### 5. Run host unit tests (optional)

To run the test suite on your machine without hardware:

```bash
pio test -e native
```

All tests execute locally and do not require the board to be connected.

---

## First boot

After a successful flash the board reboots automatically. On a blank (unconfigured) device you will see:

- The OLED displays a **SETUP** screen showing the IP address `192.168.4.1`
- The board broadcasts an open Wi-Fi access point named **`AIUsage-XXXX`** (where `XXXX` is derived from the board's MAC address)

Connect your phone or laptop to that network, open `http://192.168.4.1` in a browser, and follow the on-screen steps to enter your Wi-Fi credentials and provider API keys.

Continue with [Usage](USAGE.md) for a full walkthrough of the setup portal and the display screens.

---

## Trouble?

If the OLED stays blank, the port does not appear, or flashing fails with an error, see [Troubleshooting](TROUBLESHOOTING.md) for step-by-step fixes.

---

← Back to [Home](index.md)

# Troubleshooting

This guide covers the most common problems encountered with ai-usage-monitor. Each section follows a **Symptom → Cause → Fix** structure so you can jump straight to what is going wrong.

---

## OLED stays completely blank

**Symptom:** The display never shows anything — not even the boot splash — after flashing.

**Cause:** The I2C wiring is wrong, the SDA/SCL pins are swapped, or you are using a board variant that does not match the firmware pin assignments. The firmware targets the **ideaspark VR:2.1** board, which has a factory-integrated SSD1306 128x64 OLED wired to software I2C on **SDA = GPIO12** and **SCL = GPIO14**.

**Fix:**
- Confirm you have the ideaspark VR:2.1 board (look for the version silkscreen on the PCB). Other ESP8266 boards with add-on OLEDs almost certainly use different pin numbers.
- If you are using the integrated display, no external wiring is needed — verify nothing is shorting GPIO12 or GPIO14.
- If you added an external display, re-check your jumpers: SDA to GPIO12, SCL to GPIO14.

---

## `/dev/cu.usbserial-*` not found on macOS

**Symptom:** The port never appears in `/dev/` when you plug in the board, or `pio run -t upload` cannot find the device.

**Cause:** The board uses a **CH340** USB-to-serial bridge, which requires a driver that macOS does not ship. Additionally, many USB cables are charge-only and carry no data lines.

**Fix:**
1. Download and install the CH340 driver for macOS (search "CH340 macOS driver" for the current installer from the chip vendor).
2. After installing, unplug and re-plug the board. The port should appear as `/dev/cu.usbserial-XXXXXXXX`.
3. If it still does not appear, try a different cable — confirm it is a **data cable**, not a charge-only cable — and try a different USB port on your Mac.

---

## `pio run -t upload` does nothing, or `pio device monitor` fails with a termios error

**Symptom:** PlatformIO upload hangs or exits silently; `pio device monitor` prints a `termios` error and quits immediately.

**Cause:** There is no interactive TTY (you are running in CI, a script, or a terminal emulator that does not expose a real PTY). PlatformIO's built-in monitor and some upload paths rely on an interactive TTY.

**Fix — flashing without PlatformIO:**

Build first (`pio run`), then flash directly with esptool:

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

Replace `/dev/cu.usbserial-XXXX` with your actual port.

**Fix — reading serial output non-interactively:**

A plain passive `open()` on a CH340 port can yield no data. Use a small pyserial script instead:

```python
import serial, time

with serial.Serial('/dev/cu.usbserial-XXXX', 115200, timeout=3) as s:
    s.dtr = False
    s.rts = False
    time.sleep(0.1)
    # pulse RTS to reset the chip
    s.rts = True
    time.sleep(0.05)
    s.rts = False
    time.sleep(1)
    while True:
        line = s.readline()
        if not line:
            break
        print(line.decode('utf-8', errors='replace'), end='')
```

---

## Dashboard shows `API ERROR auth_failed`

**Symptom:** The OLED displays `API ERROR auth_failed` for Claude, Codex, or both.

**Cause:** The stored token is wrong or has expired.

**Fix:**

- **Claude:** Regenerate the token with:
  ```bash
  claude setup-token
  ```
  Then paste the new token into the device's setup page.

- **Codex:** The Codex CLI keeps a fresh access token in its auth file. Re-copy it with:
  ```bash
  jq -r '.tokens.access_token' ~/.codex/auth.json
  ```
  The Codex CLI refreshes the token automatically, so run this command at the moment you need to paste it.

---

## Codex shows `API ERROR http_403`

**Symptom:** Claude data loads fine, but Codex always shows `API ERROR http_403`.

**Cause:** Cloudflare (which fronts OpenAI's usage endpoint) blocked the direct request from the device's IP. This was not observed during development but is a known possibility with ESP8266 TLS clients.

**Fix (workaround):** Run a small proxy on an always-on machine on your LAN. The proxy reads `~/.codex/auth.json`, calls the upstream usage API, and re-serves the JSON over plain HTTP on your local network. Point the device at the proxy address instead of the upstream URL. This is a potential future option in the firmware's provider settings.

---

## Codex never appears / tap toggle stays on Claude only

**Symptom:** Tapping the button never switches to the Codex view; Codex seems unavailable even after entering credentials.

**Cause:** The Codex credentials were not saved completely. The device needs **both** the long access token **and** the account ID to consider Codex "available". If either field was empty or the token was pasted into a single-line input box (which truncates long strings), the credentials were silently rejected.

**Fix:**
1. Open the device's web setup page.
2. Paste the access token into the **multi-line TEXTAREA** — not a single-line `<input>` field — so the full token is accepted.
3. Make sure the **account ID** field is also filled in.
4. Save and restart. Re-run the setup flow if the fields appear empty after a save.

---

## Cannot factory reset / button does not reset

**Symptom:** You press the FLASH button but the device just switches provider or refreshes, never resets.

**Cause:** The button has three distinct behaviors based on hold duration, and a short press does not reset:

| Hold duration | Action |
|---|---|
| Quick tap (< 0.6 s) | Switch provider (Claude ↔ Codex) |
| 0.6 s – 5 s | Force-refresh current data |
| Continuous hold ≥ 5 s | Factory reset |

**Fix:** Hold the FLASH button **continuously** for at least 5 seconds without releasing it. Release only after the reset confirmation appears on the OLED.

---

## `pio test -e native` builds nothing ("Nothing to build")

**Symptom:** Running `pio test -e native` prints "Nothing to build" and exits without compiling tests.

**Cause:** The `[env:native]` environment is missing the `test_build_src = yes` option.

**Fix:** The repo already includes this option in `platformio.ini`. If you edited that file, restore the line:

```ini
[env:native]
test_build_src = yes
```

---

## `API ERROR no_usage`, `http_-1`, or display stalls on TLS

**Symptom:** The OLED shows `API ERROR no_usage`, `API ERROR http_-1`, or hangs for a long time before showing an error. The issue is intermittent.

**Cause:** A transient network or TLS failure. The ESP8266's TLS stack can stall or time out when the upstream server is slow, when the Wi-Fi signal is marginal, or when the device connects to a 5 GHz network (which it cannot use).

**Fix:**
- Long-press the button (0.6–5 s) to trigger a manual retry.
- Check that your Wi-Fi network is **2.4 GHz**. The ESP8266 does not support 5 GHz; if your router broadcasts a combined SSID, the device may associate at 5 GHz and then fail silently.
- Move the device closer to the router to improve signal quality.

---

← Back to [Home](index.md)

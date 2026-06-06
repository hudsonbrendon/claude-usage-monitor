# Usage

This guide walks you through setting up your ESP8266-based AI Usage Monitor, reading the dashboard, and using the hardware button. After flashing the firmware onto a blank device, everything is configured through a browser — no serial console or code editing needed.

---

## 1. Open the setup portal

When the device boots for the first time (or after a factory reset), it broadcasts an open Wi-Fi access point named **`AIUsage-XXXX`** (where `XXXX` is unique to your board).

1. On your phone or computer, connect to that network — no password required.
2. Open a browser and navigate to **http://192.168.4.1**.
3. The setup portal loads with the following fields:

| Field | Description |
|---|---|
| **Wi-Fi SSID** | The name of your home or office network |
| **Wi-Fi password** | The password for that network |
| **Claude token** | Your Claude API OAuth token (`sk-ant-oat01-…`) |
| **Codex token** | Your Codex access token (~1900 characters — use the multi-line textarea) |
| **Codex account id** | Your Codex account identifier |
| **Refresh interval** | How often the device polls for new data: 60 s, 2 min, or 5 min |

You must fill in the Wi-Fi fields and **at least one provider** (Claude, Codex, or both). Fields you leave blank are simply ignored.

When you are happy with the values, press **Save**. The device reboots and connects to your network automatically.

---

## 2. Get your credentials

### Claude token

Run the following command in a terminal:

```bash
claude setup-token
```

Copy the value it prints — it starts with `sk-ant-oat01-`. Paste it into the **Claude token** field in the portal.

### Codex credentials

Codex CLI must already be logged in on the machine you are using. Run these two commands:

```bash
# Access token (~1900 characters)
jq -r '.tokens.access_token' ~/.codex/auth.json

# Account ID
jq -r '.tokens.account_id' ~/.codex/auth.json
```

The access token is very long (~1900 characters). The portal's **Codex token** field is a multi-line textarea specifically to accommodate it — the easiest approach is to copy and paste it on the same computer where you ran the command above.

Paste the access token into **Codex token** and the shorter account identifier into **Codex account id**.

> For more detail on what each provider measures, see [Providers](PROVIDERS.md).

> **Reminder:** you need at least one provider configured. The device works with Claude only, Codex only, or both at the same time.

---

## 3. Read the dashboard

Once the device has connected and fetched data, the OLED shows two horizontal bar rows:

| Row | Window | What it shows |
|---|---|---|
| **Top** | Rolling 5-hour window | Utilisation percentage for the last 5 hours |
| **Bottom** | Weekly (7-day) window | Utilisation percentage over the current 7-day period |

To the right of each bar you will see a **reset countdown** — the time remaining until that window resets (for example `2h05m` on the 5-hour row, or `3d4h` on the weekly row).

The **status line** at the very bottom of the screen shows:

```
<provider>  <rssi>dBm  <seconds since last fetch>
```

For example: `claude  -67dBm  42` means the device is showing Claude data, your Wi-Fi signal is −67 dBm, and the last successful fetch was 42 seconds ago.

---

## 4. Button controls

Your board has a single button labelled **FLASH**. Actions are triggered on **release**, based on how long you held the button down:

| Gesture | Hold duration | Action |
|---|---|---|
| **Tap** | Under ~0.6 s | Switch to the next configured provider |
| **Long-press** | ~0.6 s – 5 s | Refresh data immediately |
| **Hold** | ≥ 5 s (continuous) | Factory reset — wipes all settings and reopens the setup portal |

---

## 5. Switching providers

Every tap cycles forward through the providers you configured during setup. When you switch, the screen briefly shows a splash with the provider name, then immediately displays its dashboard.

If you only configured a single provider, a tap has no other provider to switch to — it simply refreshes the current provider instead.

---

## 6. Reconfigure / factory reset

There is no partial-edit screen. If you need to change your Wi-Fi network, update a token, or add a provider, you must perform a full factory reset:

1. **Hold the FLASH button for 5 seconds or more**, then release.
2. The device wipes all saved settings and reboots into the setup portal.
3. The `AIUsage-XXXX` access point appears again — follow the steps in [Section 1](#1-open-the-setup-portal) to reconfigure from scratch.

---

← Back to [README](../README.md)

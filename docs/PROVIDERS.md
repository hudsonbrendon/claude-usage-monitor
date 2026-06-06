# Providers

The firmware supports multiple **providers** — data sources that each implement the same `Provider` interface. Every provider exposes two utilization windows:

| Window | Duration | What it tracks |
|--------|----------|----------------|
| Primary | 5 hours (rolling) | Short-term token burn rate |
| Secondary | 7 days (weekly) | Longer-term quota consumption |

Each window yields a **utilization percentage** (0–100 %) and a **reset time**. The device renders both windows identically regardless of which provider is active; only the credential fields and the HTTP mechanics differ between providers.

---

## Claude (Anthropic)

### Credential

You need the OAuth token issued by running `claude setup-token` in the Claude CLI. The token begins with `sk-ant-oat01-` and is valid for approximately **one year**. Enter it in the device's web UI under **Provider → Claude → Token**.

### How it works

The firmware sends a minimal, low-cost `POST` request to:

```
https://api.anthropic.com/v1/messages
```

It does not care about the response body. The utilization data is returned in the **response headers**:

| Header | Description |
|--------|-------------|
| `anthropic-ratelimit-unified-5h-utilization` | Primary-window utilization, a decimal in the range **0.0–1.0** |
| `anthropic-ratelimit-unified-5h-reset` | ISO 8601 timestamp at which the 5-hour window resets |
| `anthropic-ratelimit-unified-7d-utilization` | Weekly-window utilization, same 0.0–1.0 scale |
| `anthropic-ratelimit-unified-7d-reset` | ISO 8601 timestamp at which the 7-day window resets |

The firmware multiplies each utilization value by 100 before rendering it as a percentage on the OLED.

---

## Codex (OpenAI)

### Credentials

You need two values from the Codex CLI's local auth file (`~/.codex/auth.json`). The Codex CLI must have been logged in at least once to populate this file.

Retrieve them with:

```bash
# Access token (~7-day lifetime)
jq -r '.tokens.access_token' ~/.codex/auth.json

# Account ID (stable)
jq -r '.tokens.account_id' ~/.codex/auth.json
```

Enter both values in the device's web UI under **Provider → Codex**.

### How it works

The firmware sends:

```
GET https://chatgpt.com/backend-api/codex/usage
```

with the following request headers:

| Header | Value |
|--------|-------|
| `Authorization` | `Bearer <access_token>` |
| `chatgpt-account-id` | `<account_id>` |
| `originator` | `codex_cli_rs` — **required**; omitting it causes Cloudflare to block the request |
| `OpenAI-Beta` | `responses=experimental` |
| `User-Agent` | `codex_cli_rs/<version>` |

The JSON response body contains a `rate_limit` object with two sub-objects:

**`rate_limit.primary_window`** — the 5-hour rolling window (`limit_window_seconds` = 18 000):

| Field | Type | Description |
|-------|------|-------------|
| `used_percent` | number | Utilization already scaled to **0–100** |
| `reset_at` | number | Unix epoch at which the window resets |

**`rate_limit.secondary_window`** — the weekly window (`limit_window_seconds` = 604 800):

| Field | Type | Description |
|-------|------|-------------|
| `used_percent` | number | Utilization already scaled to **0–100** |
| `reset_at` | number | Unix epoch at which the window resets |

### Caveats

- **Unofficial endpoint.** `backend-api/codex/usage` is undocumented and not part of any public API contract. OpenAI may change or remove it without notice. If the provider stops working after an OpenAI update, check the response structure first.
- **Short-lived access token.** The `access_token` expires in approximately **7 days**. You will need to re-extract it from `~/.codex/auth.json` and re-enter it in the web UI periodically. An automatic refresh flow is a possible future feature but is not currently implemented.

---

## Security

Provider tokens are stored in **plaintext** in the device's LittleFS filesystem at `/settings.json`. There is **no PIN, no encryption, and no access control** on that file.

Anyone with physical access to the device and a serial connection or flash dump can read the stored credentials. This applies to both the Anthropic token (`sk-ant-oat01-…`) and the OpenAI access token.

Treat the device with the same level of physical security as you would the credentials themselves. If a device is lost or compromised:

1. **Revoke the Anthropic token** via the Anthropic console immediately.
2. **Log out and re-authenticate** the Codex CLI on your machine to invalidate the access token.
3. Perform a **factory reset** on the device (hold the button for 5 seconds) to wipe `/settings.json` before discarding or re-provisioning the hardware.

---

## TLS / Certificates

Both `api.anthropic.com` and `chatgpt.com` chain up to the same GlobalSign root certificate authority. The firmware ships with that root CA embedded at:

```
src/providers/globalsign_root.h
```

TLS connections to both provider endpoints are validated against this embedded root. No external certificate store is required, and the validation applies to both providers. If either host rotates to a different root CA in the future, the firmware will need to be updated with the new root.

---

← Back to [Home](index.md)

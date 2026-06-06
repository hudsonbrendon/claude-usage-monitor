# Architecture

`ai-usage-monitor` is ESP8266 firmware (PlatformIO / Arduino framework) that periodically fetches AI-agent usage data from one or more cloud APIs and renders it on a small OLED display. The design is organised around **three pluggable seams** — `Canvas`, `IBoard`, and `Provider` — so that display hardware, input hardware, and data sources can each be swapped independently without touching the rest of the code. A layer of pure, Arduino-independent logic (`Usage`, `Settings`) is host-testable under the PlatformIO `native` environment.

---

## Layers

```
┌─────────────────────────────────────────────────────────────┐
│                         main.cpp                            │
│  boot state machine · provider registry · button handling   │
└───────────┬──────────────────────┬──────────────────────────┘
            │                      │
     ┌──────▼──────┐        ┌──────▼──────┐
     │  Provider*  │        │   IBoard&   │
     │  (seam 3)   │        │  (seam 2)   │
     └──────┬──────┘        └──────┬──────┘
            │                      │
     ┌──────▼──────┐        ┌──────▼──────┐
     │UsageStatus  │        │  Canvas&    │
     │ (data bag)  │        │  (seam 1)   │
     └──────┬──────┘        └──────┬──────┘
            │                      │
     ┌──────▼──────────────────────▼──────┐
     │              Screens               │
     │  board-agnostic responsive render  │
     └────────────────────────────────────┘

     ┌──────────────────────┐   ┌─────────────────────────┐
     │  Usage  (pure logic) │   │  Settings  (pure logic) │
     │  host-testable        │   │  host-testable          │
     └──────────────────────┘   └─────────────────────────┘

     ┌──────────────────────┐
     │  Storage             │
     │  LittleFS JSON       │
     └──────────────────────┘
```

`Screens` depends only on `Canvas` and `UsageStatus`; it never calls a display library directly. `Provider` implementations depend on `Usage` and `Settings` but not on `IBoard` or `Canvas`. `main.cpp` owns all wiring.

---

## The three seams

### Seam 1 — `Canvas` (`src/Canvas.h`)

**Purpose.** An abstract drawing surface that expresses _what_ to draw in semantic terms (colour intent, text size category) without encoding _how_ the physical panel works. `Screens` calls only `Canvas` methods, which means any panel that provides a `Canvas` subclass can drive the same rendering code.

**Semantic types.**

```cpp
enum class Ink   : uint8_t { Fg, Dim, Accent, Warn, Bg };
enum class Scale : uint8_t { Small, Medium, Large };
```

On a monochrome panel `Fg`/`Dim`/`Accent`/`Warn` all map to "pixel on"; `Bg` maps to "pixel off". A colour implementation would map each value to a real colour.

**Key methods** (all pure virtual; `(x, y)` is always the top-left corner of the drawn element):

| Method | Signature |
|---|---|
| Dimensions | `int width() const` / `int height() const` |
| Frame control | `void clear()` / `void present()` |
| Power | `void power(bool on)` |
| Text | `void text(int x, int y, Scale s, Ink ink, const char* str)` |
| Text measurement | `int textWidth(Scale s, const char* str)` / `int lineHeight(Scale s) const` |
| Shapes | `void box(int x, int y, int w, int h, Ink ink, bool fill)` |
| | `void hline(int x, int y, int w, Ink ink)` |

**Current implementation — `MonoCanvas`** (`src/MonoCanvas.h/cpp`, guarded by `USE_MONO_CANVAS`): wraps the U8g2 library against an SSD1306 controller. It accepts an `offX`/`offY` offset and a logical `w`/`h` to support cropped viewports (e.g. a 72×40 active area inside a 128×64 controller frame). Font selection per `Scale` is encapsulated in a private `font(Scale)` helper.

**Extending.** Adding a colour TFT board requires only a new `Canvas` subclass; `Screens` and all business logic remain untouched.

---

### Seam 2 — `IBoard` (`src/IBoard.h`)

**Purpose.** Encapsulates everything that differs between physical boards: the display initialisation, the `Canvas` instance to draw on, and the input scheme (button debouncing, gesture classification). One board file is compiled per build, selected by its `BOARD_*` preprocessor flag.

**Key methods:**

| Method | Signature | Notes |
|---|---|---|
| Initialise | `void begin()` | Set up display + inputs |
| Drawing surface | `Canvas& canvas()` | Returns the board's `Canvas` |
| Input sampling | `void poll()` | Must be called once per `loop()` |
| Tap | `bool tapped()` | Primary action; consumed on read |
| Long press | `bool longPressed()` | Secondary action; consumed |
| Hold | `bool held(uint32_t ms)` | True while held ≥ `ms` (factory reset) |
| Battery | `int battery()` | Returns −1 if board has no gauge |
| Brightness | `void brightness(uint8_t)` | 0 = off |
| AP name prefix | `const char* apPrefix()` | Default `"AIUsage"` |

**Free function:** `IBoard& Board()` — defined in the selected board file, returns a reference to the singleton instance. `main.cpp` calls `Board()` everywhere and never names a concrete type.

**Current implementation — `ideaspark`** (`src/boards/ideaspark.cpp`, `#if defined(BOARD_IDEASPARK)`): targets the IdeaSpark ESP8266 module with an integrated SSD1306 display and a single push-button. Constructs a `MonoCanvas` over a U8g2 instance with the panel's physical geometry.

---

### Seam 3 — `Provider` (`src/Provider.h`)

**Purpose.** Abstracts a usage data source. Each provider reads whatever credentials it needs from `Settings`, makes an HTTPS request, and returns a normalised `UsageStatus`. `main.cpp` holds a fixed registry and an active index; tapping the button cycles to the next _configured_ provider.

**Interface:**

```cpp
class Provider {
public:
    virtual const char*  id()                                           = 0;
    virtual bool         available(const Settings& s)                  = 0;
    virtual UsageStatus  fetch(const Settings& s,
                               char* errOut, size_t errLen)            = 0;
};
```

- `id()` returns a short label (e.g. `"Claude"`, `"Codex"`) shown on the display.
- `available(s)` returns `true` if the relevant credentials in `s` are non-empty.
- `fetch(s, errOut, errLen)` performs the HTTPS call and returns a populated `UsageStatus`; on failure it writes a short human-readable message into `errOut` and returns a `UsageStatus` with `valid = false`.

**Current implementations:**

| Class | File | Endpoint |
|---|---|---|
| `ClaudeProvider` | `src/providers/ClaudeProvider.cpp` | `api.anthropic.com` — reads `anthropic-ratelimit-*` response headers |
| `CodexProvider` | `src/providers/CodexProvider.cpp` | `chatgpt.com/backend-api/codex/usage` — parses JSON body |

**Extending.** A new provider needs only a class that inherits `Provider`, a two-line entry in `main.cpp`'s `providers[]` array, and (if it calls a different host) its TLS root certificate.

---

## Boot state machine

### `setup()`

```
Board().begin()
  └─ Screens::splash(canvas, "starting...")
       └─ storage.begin()          // mount LittleFS; format on first use
            └─ storage.load(settings)
                 ├─ [not configured] → runPortalAndReboot()
                 │       Starts a Wi-Fi AP + captive-portal web server.
                 │       User submits SSID, password, and provider tokens.
                 │       Validated settings are saved; ESP.restart() is called.
                 │       (Does not return.)
                 └─ [configured]
                      └─ selectFirstAvailable()   // pick first provider with creds
                           └─ connectWifi()
                                ├─ [failed] → Screens::error → delay → ESP.restart()
                                └─ [ok] → syncClock()      // NTP: pool.ntp.org
                                               └─ refresh()    // first fetch
                                                    └─ draw()
```

`runPortalAndReboot` hosts the configuration UI at `http://<ap-ip>/`. A DNS catch-all redirects any hostname to the same IP (captive-portal behaviour). After a successful `POST /save`, `ESP.restart()` re-enters `setup()` with the new settings.

### `loop()`

```
Board().poll()                         // sample inputs

Board().held(5000) ──► factory reset: storage.wipe() → ESP.restart()

Board().tapped()   ──► cycleProvider() → splash(provider id) → refresh() → draw()

Board().longPressed() ──► splash("refreshing...") → refresh()

millis() - lastFetchMs >= pollSeconds*1000 ──► refresh()

millis() - lastDraw > 5000 ──► draw()    // keep countdown timers up to date

delay(20)
```

`refresh()` calls `providers[activeIdx]->fetch(settings, ...)` and stores the result in the file-scope `UsageStatus status`. `draw()` calls `Screens::dashboard` when `status.valid` is true, or `Screens::error` otherwise.

---

## Data flow

```
Provider::fetch(Settings)
    │
    │  HTTPS GET/POST
    │  parse headers (Claude) or JSON body (Codex)
    ▼
UsageStatus { h5Percent, d7Percent, h5Reset, d7Reset, valid }
    │
    ▼
Screens::dashboard(Canvas&, providerId, UsageStatus, now, rssi, secsAgo)
    │
    │  Canvas::text / Canvas::box / Canvas::hline
    ▼
physical panel (via MonoCanvas → U8g2 → SSD1306)
```

**Pure parsers** (in `src/Usage.h/cpp`, Arduino-independent, covered by `test/`):

| Function | Input | Output |
|---|---|---|
| `parseUsage` | Four C-strings: `h5util`, `h5reset`, `d7util`, `d7reset` (Anthropic rate-limit headers, values `"0.0"`–`"1.0"` and Unix-epoch seconds) | `UsageStatus` |
| `parseCodexUsage` | Full JSON body from the Codex usage endpoint | `UsageStatus` (reads `rate_limit.primary_window` for the 5 h window, `.secondary_window` for the 7 d window) |
| `formatCountdown` | `uint32_t epoch`, `uint32_t now`, output buffer | Human-readable string: `"3d4h"`, `"2h05m"`, `"12m"`, `"now"`, or `"--"` |

`UsageStatus` fields:

```cpp
struct UsageStatus {
    float    h5Percent;  // 0–100, 5-hour window utilisation
    float    d7Percent;  // 0–100, 7-day window utilisation
    uint32_t h5Reset;    // Unix epoch of next 5-hour window reset (0 if unknown)
    uint32_t d7Reset;    // Unix epoch of next 7-day window reset
    bool     valid;      // true if at least one utilisation value was parsed
};
```

---

## Storage

`Storage` (`src/Storage.h/cpp`) persists a single `Settings` struct as plaintext JSON at `/settings.json` in the LittleFS partition.

- `begin()` mounts LittleFS; if the mount fails it formats the partition and retries, so the device is always in a usable state after flashing.
- `load(Settings&)` reads the file and calls `settingsFromJson`. Missing fields keep their struct defaults; `configured` is derived — it is set to `true` only when `ssid` is non-empty and at least one provider's credentials are present.
- `save(const Settings&)` serialises via `settingsToJson` into a **4 KB static buffer** (allocated statically to avoid an ESP8266 stack overflow) before writing. The buffer is sized to accommodate the Codex `access_token`, which can be approximately 1 900 characters.
- `wipe()` removes `/settings.json`, which causes `setup()` to re-enter the captive portal on the next boot.

The `Settings` struct fields:

| Field | Type | Purpose |
|---|---|---|
| `ssid` | `std::string` | Wi-Fi network name |
| `password` | `std::string` | Wi-Fi password |
| `token` | `std::string` | Claude OAuth token (`claude setup-token`) |
| `codexToken` | `std::string` | Codex `access_token` from `~/.codex/auth.json` |
| `codexAccountId` | `std::string` | Codex account ID |
| `pollSeconds` | `uint16_t` | Refresh interval (default 120 s) |
| `configured` | `bool` | Derived; guards the portal bypass |

---

## TLS

All outbound HTTPS connections use **BearSSL** (`WiFiClientSecure`) on ESP8266 and the Arduino TLS stack (`WiFiClientSecure::setCACert`) on ESP32.

Both `api.anthropic.com` and `chatgpt.com` share the same root CA (GlobalSign Root CA — R1). The DER-encoded trust anchor is embedded in `src/providers/globalsign_root.h` and loaded once per request via `setTrustAnchors` (ESP8266) or `setCACert` (ESP32). This means only one certificate constant is needed to validate both providers, and the firmware does not disable certificate verification.

If a future provider uses a different root CA, its anchor must be added (or the existing constant must be replaced with a set that covers all required roots).

---

← Back to [Home](index.md)

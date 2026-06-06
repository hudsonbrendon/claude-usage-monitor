# Extending

The firmware is built around three pluggable seams — **Canvas** (drawing primitives), **IBoard** (display construction and button mapping), and **Provider** (data source) — so you can add a new board or a new data provider without touching shared logic.

---

## Adding a board

### 1. Decide whether you need a new Canvas subclass

Mono OLEDs reuse `MonoCanvas`, which is backed by U8g2 and gated behind `USE_MONO_CANVAS`. If your display is a new technology (color TFT, e-paper, etc.) you must first write a new `Canvas` subclass that implements the same primitives (`text`, `box`, `hline`, `clear`, `present`, and the dimension accessors). Guard the new subclass behind a `USE_<X>_CANVAS` preprocessor flag so existing builds are unaffected.

### 2. Create `src/boards/<name>.cpp`

Wrap the entire file in `#if defined(BOARD_<NAME>)` … `#endif`. Inside it:

- Instantiate the hardware display object.
- Construct the appropriate `Canvas` subclass over that display object.
- Implement every method of the `IBoard` interface, mapping the board's physical buttons to `tapped()`, `longPressed()`, and `held()`.
- Define the free function `IBoard& Board()` that returns your singleton instance.

### 3. Add a PlatformIO environment in `platformio.ini`

Create a new `[env:<name>]` section. At minimum it needs:

- `-DBOARD_<NAME>` in `build_flags` to gate your `.cpp` file.
- `-DUSE_<X>_CANVAS` in `build_flags` to select the right Canvas implementation.
- The display library in `lib_deps` (e.g. the U8g2 entry for a mono OLED, or the TFT_eSPI entry for a color panel).

### 4. Build

```
pio run -e <name>
```

`Screens`, `Storage`, the boot loop, and all other boards are untouched by this addition.

---

## Adding a provider

Follow these steps in order so that parsing is always host-testable and the build stays green throughout.

### 1. Implement the `Provider` interface

Create `src/providers/<Name>Provider.h` and `src/providers/<Name>Provider.cpp`. The interface requires three methods:

```cpp
const char* id();                                              // short unique slug
bool        available(const Settings&);                        // returns true when credentials are present
UsageStatus fetch(const Settings&, char* errOut, size_t errLen); // performs the HTTP call
```

`UsageStatus` carries two rolling-window percentages (`h5Percent`, `d7Percent`), two reset epochs (`h5Reset`, `d7Reset`), and a `valid` flag. Populate `errOut` on failure.

### 2. Add credentials to `Settings`

If your provider needs new credentials (API key, base URL, account identifier, etc.) add the corresponding fields to `src/Settings.h` and their JSON serialization/deserialization in `src/Settings.cpp`. Write or extend the host tests in `test/test_settings` to cover the new fields — this keeps credential round-tripping verifiable off-device.

### 3. Expose the credentials in the captive portal

Add the new input fields to `PORTAL_HTML` in `src/main.cpp` and handle them in the `/save` request handler so users can enter the credentials via the browser-based setup flow.

### 4. Write a pure parser and host tests

If the API returns JSON, extract a pure function `parse<Name>Usage(const char*)` in `src/Usage.cpp`. "Pure" means it takes only a string and returns a `UsageStatus` with no I/O, no HTTP, no global state. Add host tests in `test/test_usage` that cover at least: a valid full response, a missing field, and malformed JSON. See `parseUsage` and `parseCodexUsage` as reference examples. This is the most important testing step because it runs on your development machine via `pio test -e native`.

### 5. Handle TLS trust

Reuse `src/providers/globalsign_root.h` if the provider's host certificate chains to the GlobalSign Root CA. Otherwise identify the correct root certificate for the host and embed it as a PEM string in a new header, then reference that header from your provider's `.cpp`.

### 6. Register the provider

Add an instance of your provider to the `providers[]` array in `src/main.cpp`. The selection and cycling logic already iterates that array, so no further changes to the boot loop or screen code are needed.

### 7. Build and test

```
pio test -e native   # runs host tests for Settings, Usage parsers, etc.
pio run -e <target>  # compiles for the device
```

---

## Why it stays additive

`Screens` calls only the `Canvas` API, so it renders correctly on any display size or technology without modification. The boot loop and cycling logic iterate the `providers[]` array and the `IBoard` interface without knowing anything about specific boards or APIs. `Storage` is LittleFS on every chip. Adding a board changes only `src/boards/` and `platformio.ini`; adding a provider changes only `src/providers/`, `src/Settings.*`, `src/Usage.cpp`, and the registration line in `src/main.cpp`. Nothing else moves.

---

← Back to [Home](index.md)

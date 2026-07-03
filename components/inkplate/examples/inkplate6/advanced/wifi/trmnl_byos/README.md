# TRMNL BYOS

TRMNL BYOS ("Bring Your Own Server") client for Soldered Inkplate 6.

## Overview

Connects Inkplate 6 to WiFi, registers with a self-hosted Terminus (TRMNL BYOS) server via `/api/setup`, then fetches and displays whatever screen the server assigns via `/api/display`, deep-sleeping between refreshes on the interval the server specifies.

## Hardware Required

- Soldered Inkplate 6
- USB cable
- Stable WiFi connection
- A Terminus (TRMNL BYOS) server reachable on your network (see Setup)

## Setup

### 1. Run a Terminus server

Terminus is TRMNL's official self-hosted BYOS implementation, run via Docker. Quick start (not for permanent use):

```
curl https://raw.githubusercontent.com/usetrmnl/terminus/refs/heads/main/scripts/docker/quick.sh | bash
```

For permanent use, clone and run `bin/setup` instead - see the full instructions in `main/main.cpp`.

### 2. Register this device

In the Terminus dashboard: **Devices → Add Device**, using this Inkplate's WiFi MAC address (shown on the display after it connects) as the device ID.

### 3. Configure the example

In `main.cpp`, set `BYOS_SERVER` to your Terminus server's address, e.g. `http://192.168.1.50:2300` (no trailing slash).

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate6**
- **WiFi Configuration → Enter your SSID and password**

### 4. Build a screen in Terminus

**Designs → Screens → Playlists → Devices**: create a template, confirm it renders, add it to a playlist, and assign that playlist to this device.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

The board connects to WiFi, shows its device ID, fetches and displays the screen assigned to it in Terminus, then deep-sleeps until the server's configured refresh interval elapses.

## Notes

- Uses BLACK_AND_WHITE display mode so WiFi-connect progress dots can be shown with fast partial updates.
- `update_firmware` is logged only - OTA is not implemented in this example.
- Depends on the `espressif/cjson` managed component (declared in this example's `idf_component.yml`) for JSON parsing.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Terminus: https://github.com/usetrmnl/terminus

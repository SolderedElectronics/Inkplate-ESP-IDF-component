# Mapbox API

Display a Mapbox map tile on Soldered Inkplate 7.

## Overview

Connects to WiFi, fetches a map tile from the Mapbox Static Images API using a configured bounding box and API key, displays the map on the e-paper screen, then enters deep sleep for 5 minutes before waking and refreshing.

## Hardware Required

- Soldered Inkplate 7
- USB cable
- Stable WiFi connection
- Free Mapbox API key

## Setup

### 1. Get a Mapbox API key

Sign up at [https://www.mapbox.com/](https://www.mapbox.com/) and copy your API key.

### 2. Configure the example

In `main.cpp`:
- Set `API_KEY` to your Mapbox API key.
- Adjust `LAT1`, `LON1`, `LAT2`, `LON2` to the bounding box of your area of interest. Use [http://bboxfinder.com/](http://bboxfinder.com/) to find coordinates.

### 3. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate7**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- A Mapbox map tile displayed on the Inkplate 7 e-paper screen.
- Board enters deep sleep for 5 minutes, then wakes and refreshes the map.

## Notes

- Deep sleep resets normal RAM; the whole program runs in `app_main` each wakeup.
- Adjust `TIME_TO_SLEEP_US` in `main.cpp` to change the refresh interval.
- Free Mapbox tier has monthly request limits.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/

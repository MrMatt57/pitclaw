# Meater BLE Thermometer Support

Pit Claw can use a **Meater** (or Meater+) wireless probe as a full replacement for the wired Thermoworks thermistor probes.

## Full-swap model

When **Thermometer → Meater BLE** is selected in Settings (touchscreen or web UI):

| Pit Claw channel | Meater source |
|------------------|---------------|
| Pit              | Probe ambient |
| Meat 1           | Probe tip     |
| Meat 2           | Second tip (Meater Pro multi-sensor) or disconnected |

Wired ADS1115 probes are **not** read in Meater mode. Do not mix wired pit with Meater meat — the backend is mutually exclusive. Switch back to **Wired** to use the jack probes again.

## How to enable

1. Flash firmware that includes Meater support (this branch / release).
2. On the device or at `http://bbq.local`, open **Settings → Thermometer → Meater BLE**.
3. Wake the Meater probe (remove from charger). **Close the official Meater app** and disconnect any Meater Block that is holding the GATT connection — the probe allows only one BLE client at a time.
4. Wait for status to show `connected`. Pit/meat cards should populate.

The choice is stored in `config.json` as `"thermometerBackend": "meater"` (default `"wired"`).

## Supported hardware

- **MEATER / MEATER+** (classic 6-byte GATT payload) — primary target
- **MEATER Pro / MEATER 2 Plus** (12-byte multi-sensor payload) — best-effort support

Local BLE only — no Meater cloud account or internet is required.

## Protocol sources

Temperature decode formulas and UUIDs are derived from public reverse-engineering:

- [ESPHome Meater gist](https://gist.github.com/MortenVinding/a513c0094d0df41a4425612257b3cabc) (R00S / community)
- [Emkraan/homeassistant-meater](https://github.com/Emkraan/homeassistant-meater)
- [nathanfaber/meaterble](https://github.com/nathanfaber/meaterble)
- [WLANThermo Meater2 parser](https://github.com/WLANThermo-nano/WLANThermo_nRF52_Software)

Classic service UUID: `a75cc7fc-c956-488f-ac2a-2dbc08b63a04`  
Temp characteristic: `7edda774-045e-4bbf-909b-45d1991a2876`

## Caveats

- **Range**: Metal smokers attenuate BLE heavily. Keep the ESP32 within a few meters of the probe, or use a Meater Block as a bridge if you later add Block support.
- **Single connection**: Official app / Block will steal the link; Pit Claw will rescan with backoff and mark probes disconnected until it reconnects.
- **Stale data**: If no GATT updates arrive for 30 seconds, probes are marked disconnected and a reconnect is forced.
- **Wi-Fi coexistence**: Meater uses NimBLE on the ESP32-S3 alongside Wi-Fi. Heavy BLE traffic can slightly affect Wi-Fi latency.
- **Calibration offsets** in config still apply on top of Meater readings; Steinhart-Hart coefficients are ignored in Meater mode.

## Disconnect / error handling

- Disconnect → status `disconnected`, pit/meat show as disconnected (`---` / `null`), PID holds last output (same as wired open-circuit).
- Automatic rescan with exponential backoff (3s → ~60s cap).
- Switch back to Wired at any time to restore ADS1115 sampling.

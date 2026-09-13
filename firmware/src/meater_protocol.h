#pragma once

#include <stdint.h>
#include <stddef.h>

// Pure Meater BLE temperature decode helpers (no BLE stack dependency).
// Sources:
//   - ESPHome community: https://gist.github.com/MortenVinding/a513c0094d0df41a4425612257b3cabc
//   - Home Assistant local BLE: https://github.com/Emkraan/homeassistant-meater
//   - nathanfaber/meaterble reverse engineering: https://github.com/nathanfaber/meaterble
//   - WLANThermo Meater2 parser: https://github.com/WLANThermo-nano/WLANThermo_nRF52_Software

namespace meater_protocol {

// Classic MEATER / MEATER+ GATT UUIDs
static constexpr const char* SERVICE_UUID_CLASSIC =
    "a75cc7fc-c956-488f-ac2a-2dbc08b63a04";
static constexpr const char* CHAR_UUID_TEMP =
    "7edda774-045e-4bbf-909b-45d1991a2876";
static constexpr const char* CHAR_UUID_BATTERY =
    "2adb4877-68d8-4884-bd3c-d83853bf27b8";

// MEATER Pro / MEATER 2 Plus service (characteristic UUID matches classic temp)
static constexpr const char* SERVICE_UUID_MEATER2 =
    "dcbb67ca-64fb-41a3-99d1-5d9fd8cf33ca";

// Apption Labs manufacturer ID in advertisements
static constexpr uint16_t MANUFACTURER_ID = 0x037B;

struct ClassicReading {
    float tipC;
    float ambientC;
};

struct Meater2Reading {
    float tipC[5];     // multi-point tip sensors along the probe
    float ambientC;
    uint8_t tipCount;  // usually 5
};

// Decode classic 6-byte temperature characteristic.
// Returns false if len < 6.
bool decodeClassic(const uint8_t* data, size_t len, ClassicReading& out);

// Decode MEATER Pro / 2 Plus 12-byte temperature characteristic.
// Returns false if len < 12.
bool decodeMeater2(const uint8_t* data, size_t len, Meater2Reading& out);

// Decode classic 2-byte battery characteristic → percent 0-100.
// Returns -1 on invalid input.
int decodeBatteryPercent(const uint8_t* data, size_t len);

// True if advertised local name looks like a MEATER probe/block.
bool isMeaterDeviceName(const char* name);

}  // namespace meater_protocol

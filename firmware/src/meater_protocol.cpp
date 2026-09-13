#include "meater_protocol.h"
#include <string.h>

namespace meater_protocol {

bool decodeClassic(const uint8_t* data, size_t len, ClassicReading& out) {
    if (!data || len < 6) return false;

    const uint16_t tipRaw = (uint16_t)(data[0] | (data[1] << 8));
    const uint16_t ra     = (uint16_t)(data[2] | (data[3] << 8));
    const uint16_t oa     = (uint16_t)(data[4] | (data[5] << 8));

    out.tipC = (tipRaw + 8.0f) / 16.0f;

    const uint16_t minVal = 48;
    const uint16_t clampedOa = (oa < minVal) ? oa : minVal;
    int ambientAdj = 0;
    if (ra > clampedOa) {
        ambientAdj = (int)(((uint32_t)(ra - clampedOa) * 16u * 589u) / 1487u);
    }
    out.ambientC = (tipRaw + ambientAdj + 8.0f) / 16.0f;
    return true;
}

bool decodeMeater2(const uint8_t* data, size_t len, Meater2Reading& out) {
    if (!data || len < 12) return false;

    out.tipCount = 5;
    for (uint8_t i = 0; i < 5; i++) {
        uint16_t t = (uint16_t)(data[i * 2] | (data[i * 2 + 1] << 8));
        float temp = t / 32.0f;
        if (temp > 2000.0f) {
            temp -= 2048.0f;
        }
        out.tipC[i] = temp;
    }

    uint16_t ambRaw = (uint16_t)(data[10] | (data[11] << 8));
    float amb = ambRaw / 32.0f;
    if (amb > 2000.0f) {
        amb -= 2048.0f;
    }
    // WLANThermo "Meater App Fix" for ambient
    out.ambientC = amb + ((amb - out.tipC[4]) / 5.0f);
    return true;
}

int decodeBatteryPercent(const uint8_t* data, size_t len) {
    if (!data || len < 2) return -1;
    const uint16_t raw = (uint16_t)(data[0] | (data[1] << 8));
    int pct = (int)raw * 10;
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}

bool isMeaterDeviceName(const char* name) {
    if (!name || name[0] == '\0') return false;
    // Match "MEATER", "MEATER+", "MEATER Block", "MEATER2", etc. (case-insensitive prefix)
    char buf[16];
    size_t n = 0;
    while (name[n] && n < sizeof(buf) - 1) {
        char c = name[n];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
        buf[n++] = c;
    }
    buf[n] = '\0';
    return strncmp(buf, "MEATER", 6) == 0;
}

}  // namespace meater_protocol

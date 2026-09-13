#include "temp_manager.h"
#include <string.h>

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

// ADC channel mapping: probe index -> ADS1115 channel
const uint8_t TempManager::_adcChannels[NUM_PROBES] = {
    ADC_CHANNEL_PIT,
    ADC_CHANNEL_MEAT1,
    ADC_CHANNEL_MEAT2
};

TempManager::TempManager()
    : _backend(ThermometerBackend::Wired)
    , _adsOk(false)
    , _emaAlpha(TEMP_EMA_ALPHA)
    , _useFahrenheit(true)
    , _lastSampleMs(0)
{
    for (uint8_t i = 0; i < NUM_PROBES; i++) {
        _rawADC[i] = 0;
        _filteredTempC[i] = 0.0f;
        _status[i] = ProbeStatus::OPEN_CIRCUIT;
        _firstReading[i] = true;
    }
}

ThermometerBackend TempManager::backendFromString(const char* s) {
    if (s && strcmp(s, "meater") == 0) return ThermometerBackend::Meater;
    return ThermometerBackend::Wired;
}

const char* TempManager::backendToString(ThermometerBackend b) {
    return (b == ThermometerBackend::Meater) ? "meater" : "wired";
}

bool TempManager::begin() {
#ifndef NATIVE_BUILD
    Wire.begin(PIN_SDA, PIN_SCL);

    if (!_ads.begin(ADS1115_ADDR, &Wire)) {
        Serial.println("[TEMP] ADS1115 not found at 0x48!");
        _adsOk = false;
        // Still return true — Meater-only setups may have no ADS fitted
    } else {
        _ads.setGain(GAIN_ONE);
        _adsOk = true;
        Serial.println("[TEMP] ADS1115 initialized OK.");
    }
#else
    _adsOk = true;
#endif
    _lastSampleMs = 0;

    // If config already selected Meater, start BLE now
    if (_backend == ThermometerBackend::Meater) {
        _meater.begin();
    }
    return true;
}

void TempManager::setBackend(ThermometerBackend backend) {
    if (_backend == backend) return;

#ifndef NATIVE_BUILD
    Serial.printf("[TEMP] Switching thermometer backend to %s\n",
                  backendToString(backend));
#endif

    if (_backend == ThermometerBackend::Meater) {
        _meater.end();
    }

    _backend = backend;

    // Reset EMA / status so we don't bleed stale values across backends
    for (uint8_t i = 0; i < NUM_PROBES; i++) {
        _status[i] = ProbeStatus::OPEN_CIRCUIT;
        _firstReading[i] = true;
        _filteredTempC[i] = 0.0f;
        _rawADC[i] = 0;
    }

    if (_backend == ThermometerBackend::Meater) {
        _meater.begin();
    }
}

void TempManager::update() {
    if (_backend == ThermometerBackend::Meater) {
        updateMeater();
    } else {
        updateWired();
    }
}

void TempManager::updateMeater() {
    _meater.update();

#ifndef NATIVE_BUILD
    unsigned long now = millis();
    if (now - _lastSampleMs < TEMP_SAMPLE_INTERVAL_MS) {
        return;
    }
    _lastSampleMs = now;
#endif

    applyReading(PROBE_PIT,   _meater.getPitTempC(),   _meater.hasPit());
    applyReading(PROBE_MEAT1, _meater.getMeat1TempC(), _meater.hasMeat1());
    applyReading(PROBE_MEAT2, _meater.getMeat2TempC(), _meater.hasMeat2());

    // Raw ADC unused in Meater mode
    for (uint8_t i = 0; i < NUM_PROBES; i++) {
        _rawADC[i] = 0;
    }
}

void TempManager::applyReading(uint8_t probe, float tempC, bool connected) {
    if (probe >= NUM_PROBES) return;

    if (!connected) {
        _status[probe] = ProbeStatus::OPEN_CIRCUIT;
        _firstReading[probe] = true;
        return;
    }

    // Apply calibration offset (shared with wired path)
    tempC += _probeConfig[probe].offset;

    if (_firstReading[probe]) {
        _filteredTempC[probe] = tempC;
        _firstReading[probe] = false;
    } else {
        _filteredTempC[probe] = _emaAlpha * tempC + (1.0f - _emaAlpha) * _filteredTempC[probe];
    }
    _status[probe] = ProbeStatus::OK;
}

void TempManager::updateWired() {
#ifndef NATIVE_BUILD
    if (!_adsOk) {
        for (uint8_t i = 0; i < NUM_PROBES; i++) {
            _status[i] = ProbeStatus::OPEN_CIRCUIT;
        }
        return;
    }

    unsigned long now = millis();
    if (now - _lastSampleMs < TEMP_SAMPLE_INTERVAL_MS) {
        return;  // Not time to sample yet
    }
    _lastSampleMs = now;

    for (uint8_t i = 0; i < NUM_PROBES; i++) {
        // Read raw ADC value from ADS1115 single-ended
        int16_t raw = _ads.readADC_SingleEnded(_adcChannels[i]);
        _rawADC[i] = raw;

        // Check for probe errors
        if (raw >= ERROR_PROBE_OPEN_THRESHOLD) {
            _status[i] = ProbeStatus::OPEN_CIRCUIT;
            _firstReading[i] = true;  // Reset EMA on reconnect
            continue;
        }
        if (raw <= ERROR_PROBE_SHORT_THRESHOLD) {
            _status[i] = ProbeStatus::SHORT_CIRCUIT;
            _firstReading[i] = true;
            continue;
        }

        // Convert ADC to resistance
        float resistance = adcToResistance(raw);
        if (resistance <= 0.0f) {
            _status[i] = ProbeStatus::SHORT_CIRCUIT;
            _firstReading[i] = true;
            continue;
        }

        // Convert resistance to temperature in Celsius
        float tempC = resistanceToTempC(resistance, _probeConfig[i]);

        applyReading(i, tempC, true);
    }
#endif
}

float TempManager::getTemp(uint8_t probe) const {
    if (probe >= NUM_PROBES) return 0.0f;
    if (_status[probe] != ProbeStatus::OK) return 0.0f;

    if (_useFahrenheit) {
        return cToF(_filteredTempC[probe]);
    }
    return _filteredTempC[probe];
}

float TempManager::getTempC(uint8_t probe) const {
    if (probe >= NUM_PROBES) return 0.0f;
    if (_status[probe] != ProbeStatus::OK) return 0.0f;
    return _filteredTempC[probe];
}

bool TempManager::isConnected(uint8_t probe) const {
    if (probe >= NUM_PROBES) return false;
    return _status[probe] == ProbeStatus::OK;
}

ProbeStatus TempManager::getStatus(uint8_t probe) const {
    if (probe >= NUM_PROBES) return ProbeStatus::OPEN_CIRCUIT;
    return _status[probe];
}

int16_t TempManager::getRawADC(uint8_t probe) const {
    if (probe >= NUM_PROBES) return 0;
    return _rawADC[probe];
}

void TempManager::setEMAAlpha(float alpha) {
    if (alpha > 0.0f && alpha <= 1.0f) {
        _emaAlpha = alpha;
    }
}

void TempManager::setOffset(uint8_t probe, float offset) {
    if (probe < NUM_PROBES) {
        _probeConfig[probe].offset = offset;
    }
}

void TempManager::setCoefficients(uint8_t probe, float a, float b, float c) {
    if (probe < NUM_PROBES) {
        _probeConfig[probe].a = a;
        _probeConfig[probe].b = b;
        _probeConfig[probe].c = c;
    }
}

void TempManager::setUseFahrenheit(bool useF) {
    _useFahrenheit = useF;
}

float TempManager::adcToResistance(int16_t raw) const {
    // Voltage divider: Vout = Vref * R_therm / (R_ref + R_therm)
    // ADC value proportional to voltage: raw / ADC_MAX = Vout / Vref
    // Solving for R_therm:
    //   R_therm = R_ref * raw / (ADC_MAX - raw)
    // But the standard NTC voltage divider with pullup:
    //   Vout = Vcc * R_ref / (R_ref + R_therm)
    //   raw / ADC_MAX = R_ref / (R_ref + R_therm)
    //   R_therm = R_ref * (ADC_MAX / raw - 1)
    if (raw <= 0) return 0.0f;
    return REFERENCE_RESISTANCE * ((float)ADC_MAX_VALUE / (float)raw - 1.0f);
}

float TempManager::resistanceToTempC(float resistance, const ProbeConfig& cfg) const {
    // Steinhart-Hart equation:
    // 1/T = A + B * ln(R) + C * (ln(R))^3
    // T is in Kelvin
    float lnR = logf(resistance);
    float lnR3 = lnR * lnR * lnR;
    float invT = cfg.a + cfg.b * lnR + cfg.c * lnR3;

    if (invT == 0.0f) return 0.0f;

    float tempK = 1.0f / invT;
    float tempC = tempK - 273.15f;
    return tempC;
}

#include "temp_manager.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>

// Adafruit's register reader does not expose transport errors to callers.
// Check both I2C phases so stale buffer bytes cannot become a valid temperature.
static bool readAdcRegister(uint8_t reg, uint16_t& value) {
    Wire.beginTransmission(ADS1115_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((uint8_t)ADS1115_ADDR, (uint8_t)2) != 2) return false;
    value = ((uint16_t)Wire.read() << 8) | (uint8_t)Wire.read();
    return true;
}

static bool writeAdcRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(ADS1115_ADDR);
    Wire.write(reg);
    Wire.write((uint8_t)(value >> 8));
    Wire.write((uint8_t)value);
    return Wire.endTransmission() == 0;
}
#endif

// ADC channel mapping: probe index -> ADS1115 channel
const uint8_t TempManager::_adcChannels[NUM_PROBES] = {
    ADC_CHANNEL_PIT,
    ADC_CHANNEL_MEAT1,
    ADC_CHANNEL_MEAT2
};

TempManager::TempManager()
    : _emaAlpha(TEMP_EMA_ALPHA)
    , _useFahrenheit(true)
    , _lastSampleMs(0)
{
    for (uint8_t i = 0; i < NUM_PROBES; i++) {
        _rawADC[i] = 0;
        _filteredTempC[i] = 0.0f;
        _status[i] = ProbeStatus::OPEN_CIRCUIT;
        _firstReading[i] = true;
        // ProbeConfig default-initialized with THERM_A/B/C and offset 0
    }
}

bool TempManager::begin() {
#ifndef NATIVE_BUILD
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setTimeOut(25);

    if (!_ads.begin(ADS1115_ADDR, &Wire)) {
        Serial.println("[TEMP] ADS1115 not found at 0x48!");
        _adsAvailable = false;
        return false;
    }
    _adsAvailable = true;

    // Set gain to GAIN_ONE (+/- 4.096V range)
    _ads.setGain(GAIN_ONE);

    Serial.println("[TEMP] ADS1115 initialized OK.");
#endif
    _lastSampleMs = 0;
    return true;
}

void TempManager::update() {
#ifndef NATIVE_BUILD
    if (!_adsAvailable) return;  // ADC not connected — skip to avoid I2C error spam.
    unsigned long now = millis();
    if (now - _lastSampleMs < TEMP_SAMPLE_INTERVAL_MS) {
        return;  // Not time to sample yet
    }
    _lastSampleMs = now;

    // Detect a missing ADC before conversion; fail closed until the next boot.
    Wire.beginTransmission(ADS1115_ADDR);
    if (Wire.endTransmission() != 0) {
        _adsAvailable = false;
        for (uint8_t i = 0; i < NUM_PROBES; ++i)
            processRawReading(i, ERROR_PROBE_OPEN_THRESHOLD);
        Serial.println("[TEMP] ADC lost; outputs inhibited. Check wiring, then reboot.");
        return;
    }

    for (uint8_t i = 0; i < NUM_PROBES; i++) {
        // The convenience readADC_SingleEnded() waits without a deadline.
        // Bound conversion waiting so an I2C fault cannot hold fan output forever.
        const uint16_t requestedConfig = ADS1X15_REG_CONFIG_OS_SINGLE |
            (ADS1X15_REG_CONFIG_MUX_SINGLE_0 + (_adcChannels[i] << 12)) |
            ADS1X15_REG_CONFIG_PGA_4_096V | ADS1X15_REG_CONFIG_MODE_SINGLE |
            RATE_ADS1115_128SPS | ADS1X15_REG_CONFIG_CQUE_NONE;
        bool startedOK = writeAdcRegister(ADS1X15_REG_POINTER_CONFIG, requestedConfig);
        unsigned long started = millis();
        bool ready = false;
        while (startedOK && millis() - started < 50) {
            uint16_t config = 0;
            if (!readAdcRegister(ADS1X15_REG_POINTER_CONFIG, config)) break;
            // OS changes while converting; all programmed configuration bits
            // must match, including channel, gain, mode, and sample rate.
            if ((config & 0x7FFF) != (requestedConfig & 0x7FFF)) break;
            if (config & 0x8000) { ready = true; break; }
            delay(1);
        }
        uint16_t sample = 0;
        if (!ready || !readAdcRegister(ADS1X15_REG_POINTER_CONVERT, sample)) {
            _adsAvailable = false;
            for (uint8_t j = 0; j < NUM_PROBES; ++j)
                processRawReading(j, ERROR_PROBE_OPEN_THRESHOLD);
            Serial.println("[TEMP] ADC conversion failed; check wiring, then reboot.");
            return;
        }
        processRawReading(i, (int16_t)sample);
    }
#endif
}

void TempManager::processRawReading(uint8_t probe, int16_t raw) {
    if (probe >= NUM_PROBES) return;
    _rawADC[probe] = raw;
    if (raw >= ERROR_PROBE_OPEN_THRESHOLD || raw <= ERROR_PROBE_SHORT_THRESHOLD) {
        _status[probe] = raw >= ERROR_PROBE_OPEN_THRESHOLD
            ? ProbeStatus::OPEN_CIRCUIT : ProbeStatus::SHORT_CIRCUIT;
        _firstReading[probe] = true;
        return;
    }
    float resistance = adcToResistance(raw);
    float tempC = resistanceToTempC(resistance, _probeConfig[probe]) +
                  _probeConfig[probe].offset;
    if (!isfinite(resistance) || resistance <= 0.0f || !isfinite(tempC)) {
        _status[probe] = ProbeStatus::OPEN_CIRCUIT;
        _firstReading[probe] = true;
        return;
    }
    _filteredTempC[probe] = _firstReading[probe] ? tempC :
        _emaAlpha * tempC + (1.0f - _emaAlpha) * _filteredTempC[probe];
    _firstReading[probe] = false;
    _status[probe] = ProbeStatus::OK;
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

float TempManager::cToF(float tempC) {
    return tempC * 9.0f / 5.0f + 32.0f;
}

float TempManager::adcToVoltage(int16_t raw) {
    return (float)raw * ADC_MAX_VOLTAGE / ADC_COUNTS_PER_FS;
}

float TempManager::adcToResistance(int16_t raw) {
    // 3.3V -> fixed 10k -> ADC node -> NTC -> GND.
    // ADS1115 gain=1 has a 4.096V full scale, independent of its 3.3V supply.
    if (raw <= 0) return 0.0f;
    float voltage = adcToVoltage(raw);
    const float excitation = PROBE_EXCITATION_VOLTAGE;
    if (voltage >= excitation) return INFINITY;
    return REFERENCE_RESISTANCE * voltage / (excitation - voltage);
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

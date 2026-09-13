#include "meater_client.h"
#include <string.h>
#include <string>

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif

#if !defined(NATIVE_BUILD) && !defined(SIMULATOR_BUILD)
#include <NimBLEDevice.h>
#endif

// Stale reading timeout — treat probes as disconnected if no GATT data arrives
#ifndef MEATER_STALE_MS
#define MEATER_STALE_MS 30000UL
#endif

#ifndef MEATER_SCAN_MS
#define MEATER_SCAN_MS 5000UL
#endif

#ifndef MEATER_RECONNECT_BASE_MS
#define MEATER_RECONNECT_BASE_MS 3000UL
#endif

static MeaterClient* g_meaterInstance = nullptr;

MeaterClient::MeaterClient()
    : _active(false)
    , _status(MeaterStatus::Disabled)
    , _meater2ExtraTipC(0.0f)
    , _meater2ExtraValid(false)
    , _lastDataMs(0)
    , _lastScanMs(0)
    , _nextReconnectMs(0)
    , _reconnectAttempt(0)
    , _wantMeater2(false)
    , _havePending(false)
#if !defined(NATIVE_BUILD) && !defined(SIMULATOR_BUILD)
    , _client(nullptr)
    , _connectedIdx(-1)
#endif
{
    memset(_probes, 0, sizeof(_probes));
    _pendingAddress[0] = '\0';
    for (uint8_t i = 0; i < MEATER_MAX_PROBES; i++) {
        _probes[i].batteryPct = -1;
    }
}

const char* MeaterClient::getStatusString() const {
    switch (_status) {
        case MeaterStatus::Disabled:     return "disabled";
        case MeaterStatus::Scanning:     return "scanning";
        case MeaterStatus::Connecting:   return "connecting";
        case MeaterStatus::Connected:    return "connected";
        case MeaterStatus::Disconnected: return "disconnected";
        case MeaterStatus::Error:        return "error";
        default:                         return "unknown";
    }
}

const MeaterProbeReading& MeaterClient::getProbe(uint8_t index) const {
    static const MeaterProbeReading empty = {};
    if (index >= MEATER_MAX_PROBES) return empty;
    return _probes[index];
}

uint8_t MeaterClient::getProbeCount() const {
    uint8_t n = 0;
    for (uint8_t i = 0; i < MEATER_MAX_PROBES; i++) {
        if (_probes[i].valid) n++;
    }
    return n;
}

bool MeaterClient::hasPit() const {
    return _probes[0].valid;
}

bool MeaterClient::hasMeat1() const {
    return _probes[0].valid;
}

bool MeaterClient::hasMeat2() const {
    if (_probes[1].valid) return true;
    return _meater2ExtraValid;
}

float MeaterClient::getPitTempC() const {
    return _probes[0].valid ? _probes[0].ambientC : 0.0f;
}

float MeaterClient::getMeat1TempC() const {
    return _probes[0].valid ? _probes[0].tipC : 0.0f;
}

float MeaterClient::getMeat2TempC() const {
    if (_probes[1].valid) return _probes[1].tipC;
    if (_meater2ExtraValid) return _meater2ExtraTipC;
    return 0.0f;
}

int MeaterClient::getBatteryPct(uint8_t probeIndex) const {
    if (probeIndex >= MEATER_MAX_PROBES) return -1;
    return _probes[probeIndex].batteryPct;
}

void MeaterClient::markStale() {
    for (uint8_t i = 0; i < MEATER_MAX_PROBES; i++) {
        _probes[i].valid = false;
    }
    _meater2ExtraValid = false;
}

#if !defined(NATIVE_BUILD) && !defined(SIMULATOR_BUILD)

// ---- NimBLE callbacks -------------------------------------------------------

class MeaterClientCallbacks : public NimBLEClientCallbacks {
public:
    void onDisconnect(NimBLEClient* /*pClient*/, int reason) override {
        Serial.printf("[MEATER] Disconnected (reason=%d)\n", reason);
        if (g_meaterInstance) {
            g_meaterInstance->onDisconnect();
        }
    }
};

class MeaterScanCallbacks : public NimBLEScanCallbacks {
public:
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        if (!g_meaterInstance || !advertisedDevice) return;

        const char* name = advertisedDevice->haveName()
            ? advertisedDevice->getName().c_str() : "";
        const char* addr = advertisedDevice->getAddress().toString().c_str();

        bool hasClassic = advertisedDevice->isAdvertisingService(
            NimBLEUUID(meater_protocol::SERVICE_UUID_CLASSIC));
        bool hasMeater2 = advertisedDevice->isAdvertisingService(
            NimBLEUUID(meater_protocol::SERVICE_UUID_MEATER2));

        // Manufacturer ID 0x037B (Apption Labs) or name prefix MEATER*
        bool mfgMatch = false;
        if (advertisedDevice->haveManufacturerData()) {
            auto md = advertisedDevice->getManufacturerData();
            if (md.size() >= 2) {
                const uint8_t* bytes = reinterpret_cast<const uint8_t*>(md.data());
                uint16_t id = bytes[0] | (uint16_t(bytes[1]) << 8);
                mfgMatch = (id == meater_protocol::MANUFACTURER_ID);
            }
        }

        if (!hasClassic && !hasMeater2 && !mfgMatch &&
            !meater_protocol::isMeaterDeviceName(name)) {
            return;
        }

        g_meaterInstance->onAdvertisedDevice(name, addr, hasClassic, hasMeater2);
    }
};

static MeaterClientCallbacks g_clientCbs;
static MeaterScanCallbacks   g_scanCbs;

static void tempNotifyCB(NimBLERemoteCharacteristic* /*pChar*/,
                         uint8_t* pData, size_t length, bool /*isNotify*/) {
    if (!g_meaterInstance || !pData) return;
    // Heuristic: 12+ bytes → Meater2/Pro payload; else classic
    g_meaterInstance->onTempNotify(pData, length, length >= 12);
}

static void batteryNotifyCB(NimBLERemoteCharacteristic* /*pChar*/,
                            uint8_t* pData, size_t length, bool /*isNotify*/) {
    if (!g_meaterInstance || !pData) return;
    g_meaterInstance->onBatteryNotify(pData, length);
}

bool MeaterClient::begin() {
    if (_active) return true;

    g_meaterInstance = this;
    _status = MeaterStatus::Scanning;
    _reconnectAttempt = 0;
    _havePending = false;
    markStale();

    NimBLEDevice::init("");
    // Lower TX power slightly — smoker proximity; saves power
    NimBLEDevice::setPower(ESP_PWR_LVL_P3);
    NimBLEDevice::setSecurityAuth(false, false, false);

    NimBLEClient* client = NimBLEDevice::createClient();
    if (!client) {
        Serial.println("[MEATER] Failed to create BLE client");
        _status = MeaterStatus::Error;
        return false;
    }
    client->setClientCallbacks(&g_clientCbs, false);
    client->setConnectTimeout(10 * 1000);
    _client = client;

    _active = true;
    Serial.println("[MEATER] BLE client started — scanning for probes");
    startScan();
    return true;
}

void MeaterClient::end() {
    if (!_active) return;

    stopScan();

    if (_client) {
        NimBLEClient* client = static_cast<NimBLEClient*>(_client);
        if (client->isConnected()) {
            client->disconnect();
        }
        NimBLEDevice::deleteClient(client);
        _client = nullptr;
    }

    // Do not deinit entire NimBLE stack — may be shared later; just mark inactive
    _active = false;
    _status = MeaterStatus::Disabled;
    _connectedIdx = -1;
    markStale();
    if (g_meaterInstance == this) g_meaterInstance = nullptr;
    Serial.println("[MEATER] BLE client stopped");
}

void MeaterClient::startScan() {
    NimBLEScan* scan = NimBLEDevice::getScan();
    if (!scan) return;
    scan->setScanCallbacks(&g_scanCbs, false);
    scan->setActiveScan(true);
    scan->setInterval(100);
    scan->setWindow(80);
    scan->setMaxResults(0);  // don't store results; callback only
    if (!scan->isScanning()) {
        scan->start(MEATER_SCAN_MS, false, true);
        _status = (_status == MeaterStatus::Connected) ? _status : MeaterStatus::Scanning;
        _lastScanMs = millis();
        Serial.println("[MEATER] Scan started");
    }
}

void MeaterClient::stopScan() {
    NimBLEScan* scan = NimBLEDevice::getScan();
    if (scan && scan->isScanning()) {
        scan->stop();
    }
}

bool MeaterClient::onAdvertisedDevice(const char* name, const char* address,
                                      bool hasClassicSvc, bool hasMeater2Svc) {
    if (!_active || !address) return false;

    NimBLEClient* client = static_cast<NimBLEClient*>(_client);
    if (client && client->isConnected()) {
        return false;  // already connected — ignore additional advertisements for now
    }

    // Prefer first free probe slot
    strncpy(_pendingAddress, address, sizeof(_pendingAddress) - 1);
    _pendingAddress[sizeof(_pendingAddress) - 1] = '\0';
    _wantMeater2 = hasMeater2Svc && !hasClassicSvc;
    _havePending = true;

    if (name && name[0]) {
        strncpy(_probes[0].name, name, sizeof(_probes[0].name) - 1);
        _probes[0].name[sizeof(_probes[0].name) - 1] = '\0';
    }
    strncpy(_probes[0].address, address, sizeof(_probes[0].address) - 1);
    _probes[0].address[sizeof(_probes[0].address) - 1] = '\0';

    Serial.printf("[MEATER] Found %s (%s)%s\n",
                  name && name[0] ? name : "MEATER",
                  address,
                  _wantMeater2 ? " [Pro/2]" : "");

    // Stop scan so we can connect
    stopScan();
    return true;
}

bool MeaterClient::connectToAddress(const char* address, bool meater2) {
    NimBLEClient* client = static_cast<NimBLEClient*>(_client);
    if (!client || !address) return false;

    _status = MeaterStatus::Connecting;
    Serial.printf("[MEATER] Connecting to %s...\n", address);

    NimBLEAddress addr(address);
    if (!client->connect(addr, false)) {
        Serial.println("[MEATER] Connect failed");
        _status = MeaterStatus::Disconnected;
        return false;
    }

    Serial.println("[MEATER] Connected — discovering services");

    const char* svcUuid = meater2
        ? meater_protocol::SERVICE_UUID_MEATER2
        : meater_protocol::SERVICE_UUID_CLASSIC;

    NimBLERemoteService* svc = client->getService(svcUuid);
    if (!svc && meater2) {
        // Fallback: try classic service UUID
        svc = client->getService(meater_protocol::SERVICE_UUID_CLASSIC);
        meater2 = false;
    }
    if (!svc && !meater2) {
        svc = client->getService(meater_protocol::SERVICE_UUID_MEATER2);
        meater2 = (svc != nullptr);
    }
    if (!svc) {
        Serial.println("[MEATER] Temperature service not found");
        client->disconnect();
        _status = MeaterStatus::Error;
        return false;
    }

    NimBLERemoteCharacteristic* tempChar =
        svc->getCharacteristic(meater_protocol::CHAR_UUID_TEMP);
    if (!tempChar) {
        Serial.println("[MEATER] Temperature characteristic not found");
        client->disconnect();
        _status = MeaterStatus::Error;
        return false;
    }

    if (tempChar->canNotify()) {
        if (!tempChar->subscribe(true, tempNotifyCB)) {
            Serial.println("[MEATER] Temp notify subscribe failed — will poll");
        }
    }

    // Initial read
    if (tempChar->canRead()) {
        std::string value = tempChar->readValue();
        if (!value.empty()) {
            onTempNotify(reinterpret_cast<const uint8_t*>(value.data()),
                         value.size(), meater2 || value.size() >= 12);
        }
    }

    NimBLERemoteCharacteristic* batChar =
        svc->getCharacteristic(meater_protocol::CHAR_UUID_BATTERY);
    if (batChar) {
        if (batChar->canNotify()) {
            batChar->subscribe(true, batteryNotifyCB);
        }
        if (batChar->canRead()) {
            std::string value = batChar->readValue();
            if (!value.empty()) {
                onBatteryNotify(reinterpret_cast<const uint8_t*>(value.data()),
                                value.size());
            }
        }
    }

    _connectedIdx = 0;
    _status = MeaterStatus::Connected;
    _reconnectAttempt = 0;
    _lastDataMs = millis();
    _wantMeater2 = meater2;
    Serial.println("[MEATER] Ready");
    return true;
}

void MeaterClient::onTempNotify(const uint8_t* data, size_t len, bool meater2) {
    if (!data) return;

#ifndef NATIVE_BUILD
    _lastDataMs = millis();
#endif

    if (meater2 && len >= 12) {
        meater_protocol::Meater2Reading reading;
        if (!meater_protocol::decodeMeater2(data, len, reading)) return;

        // Tip sensor 0 → meat1; tip sensor 1 → meat2; ambient → pit
        _probes[0].tipC = reading.tipC[0];
        _probes[0].ambientC = reading.ambientC;
        _probes[0].valid = true;
        _meater2ExtraTipC = reading.tipC[1];
        _meater2ExtraValid = true;
        _status = MeaterStatus::Connected;
        return;
    }

    meater_protocol::ClassicReading reading;
    if (!meater_protocol::decodeClassic(data, len, reading)) return;

    _probes[0].tipC = reading.tipC;
    _probes[0].ambientC = reading.ambientC;
    _probes[0].valid = true;
    _meater2ExtraValid = false;
    _status = MeaterStatus::Connected;
}

void MeaterClient::onBatteryNotify(const uint8_t* data, size_t len) {
    int pct = meater_protocol::decodeBatteryPercent(data, len);
    if (pct >= 0) {
        _probes[0].batteryPct = pct;
    }
}

void MeaterClient::onDisconnect() {
    markStale();
    _connectedIdx = -1;
    _status = MeaterStatus::Disconnected;

    // Exponential backoff reconnect
    uint32_t delayMs = MEATER_RECONNECT_BASE_MS;
    for (uint8_t i = 0; i < _reconnectAttempt && i < 4; i++) {
        delayMs *= 2;
    }
    if (delayMs > 60000UL) delayMs = 60000UL;
    _nextReconnectMs = millis() + delayMs;
    if (_reconnectAttempt < 8) _reconnectAttempt++;
    Serial.printf("[MEATER] Will rescan in %lu ms\n", (unsigned long)delayMs);
}

void MeaterClient::update() {
    if (!_active) return;

    unsigned long now = millis();

    // Stale data watchdog
    if (_status == MeaterStatus::Connected && _lastDataMs > 0) {
        if (now - _lastDataMs > MEATER_STALE_MS) {
            Serial.println("[MEATER] Stale data — forcing reconnect");
            NimBLEClient* client = static_cast<NimBLEClient*>(_client);
            if (client && client->isConnected()) {
                client->disconnect();
            } else {
                onDisconnect();
            }
        }
    }

    // Periodic keepalive read while connected
    static unsigned long lastPollMs = 0;
    if (_status == MeaterStatus::Connected && now - lastPollMs > 4000UL) {
        lastPollMs = now;
        NimBLEClient* client = static_cast<NimBLEClient*>(_client);
        if (client && client->isConnected()) {
            const char* svcUuid = _wantMeater2
                ? meater_protocol::SERVICE_UUID_MEATER2
                : meater_protocol::SERVICE_UUID_CLASSIC;
            NimBLERemoteService* svc = client->getService(svcUuid);
            if (svc) {
                NimBLERemoteCharacteristic* tempChar =
                    svc->getCharacteristic(meater_protocol::CHAR_UUID_TEMP);
                if (tempChar && tempChar->canRead()) {
                    std::string value = tempChar->readValue();
                    if (!value.empty()) {
                        onTempNotify(reinterpret_cast<const uint8_t*>(value.data()),
                                     value.size(),
                                     _wantMeater2 || value.size() >= 12);
                    }
                }
            }
        }
    }

    // Connect to pending advertisement
    if (_havePending && _status != MeaterStatus::Connected &&
        _status != MeaterStatus::Connecting) {
        _havePending = false;
        if (!connectToAddress(_pendingAddress, _wantMeater2)) {
            _nextReconnectMs = now + MEATER_RECONNECT_BASE_MS;
        }
        return;
    }

    // Reconnect / rescan
    if (_status == MeaterStatus::Disconnected ||
        _status == MeaterStatus::Scanning ||
        _status == MeaterStatus::Error) {
        NimBLEClient* client = static_cast<NimBLEClient*>(_client);
        if (client && client->isConnected()) return;

        if (_nextReconnectMs != 0 && now < _nextReconnectMs) return;

        NimBLEScan* scan = NimBLEDevice::getScan();
        if (scan && !scan->isScanning()) {
            startScan();
            _nextReconnectMs = 0;
        } else if (scan && scan->isScanning() && now - _lastScanMs > MEATER_SCAN_MS + 1000UL) {
            // Scan finished without finding a device — restart after short pause
            _nextReconnectMs = now + MEATER_RECONNECT_BASE_MS;
            _status = MeaterStatus::Disconnected;
        }
    }
}

#else  // NATIVE_BUILD || SIMULATOR_BUILD

bool MeaterClient::begin() {
    _active = true;
    _status = MeaterStatus::Scanning;
    return true;
}

void MeaterClient::end() {
    _active = false;
    _status = MeaterStatus::Disabled;
    markStale();
}

void MeaterClient::update() {}

void MeaterClient::onTempNotify(const uint8_t* data, size_t len, bool meater2) {
    if (meater2 && len >= 12) {
        meater_protocol::Meater2Reading reading;
        if (!meater_protocol::decodeMeater2(data, len, reading)) return;
        _probes[0].tipC = reading.tipC[0];
        _probes[0].ambientC = reading.ambientC;
        _probes[0].valid = true;
        _meater2ExtraTipC = reading.tipC[1];
        _meater2ExtraValid = true;
        _status = MeaterStatus::Connected;
        return;
    }
    meater_protocol::ClassicReading reading;
    if (!meater_protocol::decodeClassic(data, len, reading)) return;
    _probes[0].tipC = reading.tipC;
    _probes[0].ambientC = reading.ambientC;
    _probes[0].valid = true;
    _status = MeaterStatus::Connected;
}

void MeaterClient::onBatteryNotify(const uint8_t* data, size_t len) {
    int pct = meater_protocol::decodeBatteryPercent(data, len);
    if (pct >= 0) _probes[0].batteryPct = pct;
}

void MeaterClient::onDisconnect() {
    markStale();
    _status = MeaterStatus::Disconnected;
}

bool MeaterClient::onAdvertisedDevice(const char*, const char*, bool, bool) {
    return false;
}

bool MeaterClient::connectToAddress(const char*, bool) { return false; }
void MeaterClient::startScan() {}
void MeaterClient::stopScan() {}

#endif

#pragma once

#include "config.h"
#include "meater_protocol.h"
#include <stdint.h>

// Connection / probe status for Meater BLE backend
enum class MeaterStatus : uint8_t {
    Disabled = 0,
    Scanning,
    Connecting,
    Connected,
    Disconnected,   // was connected, now retrying
    Error
};

// Max simultaneous Meater probes we track (tip→meat mapping)
#define MEATER_MAX_PROBES 2

struct MeaterProbeReading {
    bool     valid;
    float    tipC;
    float    ambientC;
    int      batteryPct;   // -1 if unknown
    char     name[24];
    char     address[18];  // "AA:BB:CC:DD:EE:FF"
};

// BLE client that scans for MEATER probes, connects, and parses tip/ambient.
// Prefer local GATT over Meater cloud APIs.
class MeaterClient {
public:
    MeaterClient();

    // Initialize NimBLE stack. Call once when Meater mode is selected.
    bool begin();

    // Tear down connections and stop scanning.
    void end();

    // Drive state machine: scan / connect / reconnect / stale checks. Call every loop().
    void update();

    MeaterStatus getStatus() const { return _status; }
    const char*  getStatusString() const;

    // Aggregated temps mapped into Pit Claw's model (Celsius):
    //   pit   = ambient from first connected probe
    //   meat1 = tip from probe 0
    //   meat2 = tip from probe 1 (or Meater2 second tip sensor if single Pro probe)
    bool  hasPit() const;
    bool  hasMeat1() const;
    bool  hasMeat2() const;
    float getPitTempC() const;
    float getMeat1TempC() const;
    float getMeat2TempC() const;

    int   getBatteryPct(uint8_t probeIndex = 0) const;
    uint8_t getProbeCount() const;

    const MeaterProbeReading& getProbe(uint8_t index) const;

    bool isActive() const { return _active; }

    // Called from BLE notify/read callbacks (public for static trampolines)
    void onTempNotify(const uint8_t* data, size_t len, bool meater2);
    void onBatteryNotify(const uint8_t* data, size_t len);
    void onDisconnect();
    bool onAdvertisedDevice(const char* name, const char* address, bool hasClassicSvc, bool hasMeater2Svc);

private:
    void startScan();
    void stopScan();
    void markStale();
    bool connectToAddress(const char* address, bool meater2);

    bool               _active;
    MeaterStatus       _status;
    MeaterProbeReading _probes[MEATER_MAX_PROBES];
    float              _meater2ExtraTipC;
    bool               _meater2ExtraValid;
    unsigned long      _lastDataMs;
    unsigned long      _lastScanMs;
    unsigned long      _nextReconnectMs;
    uint8_t            _reconnectAttempt;
    bool               _wantMeater2;
    char               _pendingAddress[18];
    bool               _havePending;

#if !defined(NATIVE_BUILD) && !defined(SIMULATOR_BUILD)
    void* _client;  // NimBLEClient*
    int   _connectedIdx;
#endif
};

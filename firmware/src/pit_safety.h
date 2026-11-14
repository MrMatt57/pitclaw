#pragma once
#include "config.h"
#include <stdint.h>

// A fault inhibits outputs immediately. Reconnect must remain valid for 3s.
class PitSafety {
public:
    bool update(bool valid, uint32_t now) {
        if (!valid) { _tracking = false; _ready = false; return false; }
        if (!_tracking) { _tracking = true; _validSince = now; }
        _ready = (uint32_t)(now - _validSince) >= PIT_RECOVERY_HOLD_MS;
        return _ready;
    }
    bool ready() const { return _ready; }
private:
    bool _tracking = false;
    bool _ready = false;
    uint32_t _validSince = 0;
};

#pragma once
#include "concurrency/OSThread.h"
#include <Arduino.h>

class HeartbeatThread : public concurrency::OSThread
{
public:
    HeartbeatThread();

protected:
    int32_t runOnce() override;

private:
    // D30 on this variant is the NFC1 pin (P0.09).
    // The board variant header does not expose a D30 macro, so use the numeric pin value.
    const uint8_t heartbeatPin = 30;
    bool currentState = false;
};

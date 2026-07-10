#include "HeartbeatThread.h"
#include "configuration.h"
#include "DebugConfiguration.h"

HeartbeatThread::HeartbeatThread()
    : OSThread("Heartbeat")
{
    pinMode(heartbeatPin, OUTPUT);
    digitalWrite(heartbeatPin, LOW);
    currentState = false;
}

int32_t HeartbeatThread::runOnce()
{
    currentState = !currentState;
    digitalWrite(heartbeatPin, currentState ? HIGH : LOW);
    // Debug logging removed: D0 toggles are noisy when GPS forces the pin
    return 2500; // Toggle every 2.5s
}

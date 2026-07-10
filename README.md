# Meshtastic Firmware — NodeCommandModule

Fork of [meshtastic/firmware](https://github.com/meshtastic/firmware) with the addition of **NodeCommandModule**, which allows querying a node via direct message on the mesh.

## Supported Commands

Send a direct message to the node in the format `NodeName command`:

| Command | Response |
|---|---|
| `NodeName ping` | `pong` |
| `NodeName position` | Sends a real position packet on the mesh + `Sent` |
| `NodeName telemetry` | Sends real device + environment telemetry packets on the mesh + `Sent` |
| `NodeName where` | Replies with a Google Maps link of the last known GPS position |

Commands are **case insensitive**. Position and telemetry are transmitted as native Meshtastic packets — all nodes in the mesh receive them and update their nodeDB exactly as with automatic broadcasts.

## Supported Devices

| Device | File | Notes |
|---|---|---|
| Seeed XIAO nRF52840 Kit (I2C on D6/D7) | `firmware-seeed_xiao_nrf52840_kit_i2c-*.uf2` | Drag & drop in bootloader mode |
| Heltec WiFi LoRa 32 V3 | `firmware-heltec-v3-*.bin` | Update via [Web Flasher](https://flasher.meshtastic.org) |
| Heltec WiFi LoRa 32 V3 (first flash) | `firmware-heltec-v3-*.factory.bin` | First flash on a blank device |
| Heltec Wireless Stick Lite V3 | `firmware-heltec-wsl-v3-*.bin` | Update via [Web Flasher](https://flasher.meshtastic.org) |
| Heltec Wireless Stick Lite V3 (first flash) | `firmware-heltec-wsl-v3-*.factory.bin` | First flash on a blank device |
| RAK4631 | `firmware-rak4631-*.uf2` | Drag & drop in bootloader mode |

## How to Flash

**XIAO nRF52840 / RAK4631:** double-click the reset button, a USB drive appears, drag and drop the `.uf2` file onto it.

**Heltec V3 / Wireless Stick Lite V3:** go to [flasher.meshtastic.org](https://flasher.meshtastic.org), select your device, upload the `.bin` via "Custom firmware". For first flash on a blank device use the `.factory.bin`.

## Modified Files

- `src/modules/NodeCommandModule.h` — module header
- `src/modules/NodeCommandModule.cpp` — command implementation
- `src/modules/Modules.cpp` — module registration
- `src/modules/Telemetry/DeviceTelemetry.h` — added public wrapper `sendTelemetryPublic()`
- `src/modules/Telemetry/DeviceTelemetry.cpp` — added global pointer `deviceTelemetryModule`
- `src/modules/Telemetry/EnvironmentTelemetry.h` — added public wrapper `sendEnvTelemetryPublic()`
- `src/modules/Telemetry/EnvironmentTelemetry.cpp` — added global pointer `environmentTelemetryModule`

## Build
```bash
# Seeed XIAO nRF52840 Kit with I2C on D6/D7
pio run -e seeed_xiao_nrf52840_kit_i2c

# Seeed XIAO ESP32S3
pio run -e seeed-xiao-s3

# Heltec WiFi LoRa 32 V3
pio run -e heltec-v3

# Heltec Wireless Stick Lite V3
pio run -e heltec-wsl-v3

# RAK4631
pio run -e rak4631

# All at once
pio run -e seeed_xiao_nrf52840_kit_i2c -e heltec-v3 -e heltec-wsl-v3 -e rak4631 -e seeed-xiao-s3
```

## Build Notes (for slow machines / unstable SSH sessions)

For long builds on slow machines or over unstable SSH connections, run builds inside a terminal multiplexer such as `screen` or `tmux` to avoid losing progress if the SSH session disconnects:
```bash
screen -S build
cd ~/firmware
pio run -e seeed_xiao_nrf52840_kit_i2c -e heltec-v3 -e heltec-wsl-v3 -e rak4631
```

If the SSH session drops, reconnect and resume the `screen` session with:
```bash
screen -r build
```

## Credits

Based on [meshtastic/firmware](https://github.com/meshtastic/firmware) — follow the upstream repo for updates.

<!-- Removed Italian duplicate section; build notes consolidated above in English -->

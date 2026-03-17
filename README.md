# Meshtastic Firmware — NodeCommandModule

Fork di [meshtastic/firmware](https://github.com/meshtastic/firmware) con aggiunta del modulo **NodeCommandModule** che permette di interrogare un nodo via messaggio diretto sulla mesh.

## Comandi supportati

Invia un messaggio diretto al nodo con il formato `NomeNodo comando`:

| Comando | Risposta |
|---|---|
| `NomeNodo ping` | `pong` |
| `NomeNodo position` | Invia pacchetto posizione reale sulla mesh + `Sent` |
| `NomeNodo telemetry` | Invia pacchetto telemetria reale sulla mesh + `Sent` |

I comandi sono **case insensitive**. La posizione e la telemetria vengono trasmesse come pacchetti nativi Meshtastic — tutti i nodi nella mesh li ricevono e aggiornano il loro nodeDB esattamente come per i broadcast automatici.

## Dispositivi supportati e file firmware

| Dispositivo | File | Note |
|---|---|---|
| Seeed XIAO nRF52840 Kit (I2C su D6/D7) | `firmware-seeed_xiao_nrf52840_kit_i2c-*.uf2` | Drag&drop in modalità bootloader |
| Heltec WiFi LoRa 32 V3 | `firmware-heltec-v3-*.bin` | Aggiornamento via [Web Flasher](https://flasher.meshtastic.org) |
| Heltec WiFi LoRa 32 V3 (primo flash) | `firmware-heltec-v3-*.factory.bin` | Primo flash su dispositivo vergine |
| Heltec Wireless Stick Lite V3 | `firmware-heltec-wsl-v3-*.bin` | Aggiornamento via [Web Flasher](https://flasher.meshtastic.org) |
| Heltec Wireless Stick Lite V3 (primo flash) | `firmware-heltec-wsl-v3-*.factory.bin` | Primo flash su dispositivo vergine |

## Come flashare

**XIAO nRF52840:** doppio click sul tasto reset, compare un disco USB, trascina il `.uf2`.

**Heltec V3 / Wireless Stick Lite V3:** vai su [flasher.meshtastic.org](https://flasher.meshtastic.org), seleziona il dispositivo, carica il `.bin` tramite "Custom firmware".

## Modifiche al firmware originale

- `src/modules/NodeCommandModule.h` — header del modulo
- `src/modules/NodeCommandModule.cpp` — implementazione comandi
- `src/modules/Modules.cpp` — registrazione modulo
- `src/modules/Telemetry/DeviceTelemetry.h` — aggiunto wrapper pubblico `sendTelemetryPublic()`
- `src/modules/Telemetry/DeviceTelemetry.cpp` — aggiunto puntatore globale `deviceTelemetryModule`

## Build
```bash
# XIAO nRF52840 con I2C su D6/D7
pio run -e seeed_xiao_nrf52840_kit_i2c

# Heltec WiFi LoRa 32 V3
pio run -e heltec-v3

# Heltec Wireless Stick Lite V3
pio run -e heltec-wsl-v3
```

## Crediti

Basato su [meshtastic/firmware](https://github.com/meshtastic/firmware) — seguire il repo upstream per aggiornamenti.

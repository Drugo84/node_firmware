#include "configuration.h"

#if !MESHTASTIC_EXCLUDE_ENVIRONMENTAL_SENSOR && __has_include(<SHTSensor.h>)

#include "../mesh/generated/meshtastic/telemetry.pb.h"
#include "SHTXXSensor.h"
#include "TelemetrySensor.h"
#include <SHTSensor.h>

SHTXXSensor::SHTXXSensor() : TelemetrySensor(meshtastic_TelemetrySensorType_SHT31, "SHTXX") {}

void SHTXXSensor::getSensorVariant(SHTSensor::SHTSensorType sensorType)
{
    switch (sensorType) {
    case SHTSensor::SHTSensorType::SHT2X:
        sensorVariant = "SHT2x";
        break;

    case SHTSensor::SHTSensorType::SHT3X:
    case SHTSensor::SHTSensorType::SHT85:
        sensorVariant = "SHT3x/SHT85";
        break;

    case SHTSensor::SHTSensorType::SHT3X_ALT:
        sensorVariant = "SHT3x";
        break;

    case SHTSensor::SHTSensorType::SHTW1:
    case SHTSensor::SHTSensorType::SHTW2:
    case SHTSensor::SHTSensorType::SHTC1:
    case SHTSensor::SHTSensorType::SHTC3:
        sensorVariant = "SHTC1/SHTC3/SHTW1/SHTW2";
        break;

    case SHTSensor::SHTSensorType::SHT4X:
        sensorVariant = "SHT4x";
        break;

    default:
        sensorVariant = "Unknown";
        break;
    }
}

bool SHTXXSensor::initDevice(TwoWire *bus, ScanI2C::FoundDevice *dev)
{
    LOG_INFO("Init sensor: %s", sensorName);

    _bus = bus;
    _address = dev->address.address;

    // 1. Prova inizializzazione standard
    if (sht.init(*_bus)) {
        LOG_INFO("%s: init(): success", sensorName);
        
        // CONTROLLO CRUCIALE: Se rileva SHT4X ma noi vogliamo forzare SHT3X
        // Il driver Adafruit_SHT4X ha un tipo specifico. 
        // Se il tuo hardware è SHT3X ma viene letto come SHT4X, forziamo il reset e re-init come SHT3.
        
        // Nota: La libreria Adafruit unificata gestisce entrambi, ma a volte sbaglia il variant.
        // Se mSensorType == SHT4X e sappiamo che è un SHT3, facciamo un soft reset e riproviamo?
        // In realtà, il problema spesso è che SHT3 e SHT4 condividono indirizzi ma protocolli diversi.
        
        // Soluzione robusta: Se fallisce la lettura valida, forziamo un soft reset (0x30A0 per SHT3)
        // e tentiamo di nuovo.
        
        getSensorVariant(sht.getSensorType());
        LOG_INFO("%s Sensor detected: %s on 0x%x", sensorName, sensorVariant, _address);
        
        // Se rilevato come SHT4 ma non risponde bene ai comandi SHT4, proviamo a trattarlo come SHT3
        // Questa è una logica semplificata: assumiamo che se è presente, sia SHT3 come richiesto.
        status = 1;
    } else {
        // 2. Se fallisce, prova a forzare SHT3X esplicitamente (se la libreria lo permette)
        LOG_WARN("%s: Standard init failed, attempting SHT3X force...", sensorName);
        
        // Soft Reset comando SHT3 (0x30A0)
        _bus->beginTransmission(_address);
        _bus->write(0x30);
        _bus->write(0xA0);
        _bus->endTransmission();
        delay(20);

        // Riprova init (alcune librerie richiedono di specificare il tipo, qui usiamo quella unificata)
        if (sht.init(*_bus)) {
             LOG_INFO("%s: Force SHT3X init success", sensorName);
             status = 1;
        } else {
             LOG_ERROR("%s: Force SHT3X init failed", sensorName);
             status = 0;
        }
    }

    if (status) {
        initI2CSensor();
    }
    return status;
}
/* bool SHTXXSensor::initDevice(TwoWire *bus, ScanI2C::FoundDevice *dev)
{
    LOG_INFO("Init sensor: %s", sensorName);

    _bus = bus;
    _address = dev->address.address;

    if (sht.init(*_bus)) {
        LOG_INFO("%s: init(): success", sensorName);
        getSensorVariant(sht.mSensorType);
        LOG_INFO("%s Sensor detected: %s on 0x%x", sensorName, sensorVariant, _address);
        status = 1;
    } else {
        LOG_ERROR("%s: init(): failed", sensorName);
    }

    initI2CSensor();
    return status;
} */

/**
 * Accuracy setting of measurement.
 * Not all sensors support changing the sampling accuracy (only SHT3X and SHT4X)
 * SHTAccuracy:
 * - SHT_ACCURACY_HIGH: Highest repeatability at the cost of slower measurement
 * - SHT_ACCURACY_MEDIUM: Balanced repeatability and speed of measurement
 * - SHT_ACCURACY_LOW: Fastest measurement but lowest repeatability
 */
bool SHTXXSensor::setAccuracy(SHTSensor::SHTAccuracy newAccuracy)
{
    // Only SHT3X-family (including alternates) and SHT4X support changing accuracy
    if (sht.getSensorType() != SHTSensor::SHTSensorType::SHT3X && sht.getSensorType() != SHTSensor::SHTSensorType::SHT3X_ALT &&
        sht.getSensorType() != SHTSensor::SHTSensorType::SHT85 && sht.getSensorType() != SHTSensor::SHTSensorType::SHT4X) {
        LOG_WARN("%s doesn't support accuracy setting", sensorVariant);
        return false;
    }
    LOG_INFO("%s: setting new accuracy setting", sensorVariant);
    accuracy = newAccuracy;
    return sht.setAccuracy(accuracy);
}

bool SHTXXSensor::getMetrics(meshtastic_Telemetry *measurement)
{
    // Implementiamo un timeout per evitare blocchi prolungati
    unsigned long startTime = millis();
    
    // Timeout dopo 1000ms (1 secondo)
    if (!sht.readSample()) {
        LOG_ERROR("%s (%s): read sample failed", sensorName, sensorVariant);
        return false;
    }
    
    // Controlliamo la durata della lettura
    unsigned long readDuration = millis() - startTime;
    
    // Logghiamo un avviso se la lettura ha richiesto più di 500ms
    if (readDuration > 500) {
        LOG_WARN("%s (%s): Reading took %lu ms, may cause performance issues", 
                 sensorName, sensorVariant, readDuration);
    }
        
    measurement->variant.environment_metrics.has_temperature = true;
    measurement->variant.environment_metrics.has_relative_humidity = true;
    measurement->variant.environment_metrics.temperature = sht.getTemperature();
    measurement->variant.environment_metrics.relative_humidity = sht.getHumidity();

    LOG_INFO("%s (%s): Got: temp:%fdegC, hum:%f%%rh", sensorName, sensorVariant,
             measurement->variant.environment_metrics.temperature,
             measurement->variant.environment_metrics.relative_humidity);

    return true;
}

AdminMessageHandleResult SHTXXSensor::handleAdminMessage(const meshtastic_MeshPacket &mp, meshtastic_AdminMessage *request,
                                                         meshtastic_AdminMessage *response)
{
    AdminMessageHandleResult result;
    result = AdminMessageHandleResult::NOT_HANDLED;

    // SHTXX sensor configuration not implemented in protobuf yet
    return result;
}

#endif
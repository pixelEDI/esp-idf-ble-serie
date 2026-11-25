# ESP-IDF BLE-Serie

Vier kleine ESP-IDF-Beispiele für Bluetooth Low Energy mit NimBLE auf dem ESP32:

1. Advertising starten
2. GATT-Service und lesbare Characteristic
3. Sensordaten per Notification senden
4. LED per BLE-Write schalten

## Voraussetzungen

- ESP-IDF eingerichtet und `IDF_PATH` gesetzt
- ESP32-Board mit BLE-Unterstützung
- nRF Connect oder eine vergleichbare BLE-App

## Bauen und flashen

Jedes Beispiel ist ein eigenständiges ESP-IDF-Projekt. Zum Beispiel:

```bash
cd 01_nimble_advertising
idf.py set-target esp32c6
idf.py build flash monitor
```



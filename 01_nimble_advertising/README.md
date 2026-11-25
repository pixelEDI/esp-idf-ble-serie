# ESP-IDF BLE #1: NimBLE einrichten und Advertising

## Short-Idee, ca. 2 Minuten

**Text:** BLE ist kein WLAN. Das Gerät ruft nicht laut Daten ins Netzwerk, sondern macht sich mit kleinen Werbepaketen sichtbar. Das heißt Advertising. In ESP-IDF aktivieren wir dafür NimBLE, geben dem Gerät einen Namen und starten das Advertising.

**Code zeigen:**

```c
fields.name = (uint8_t *)"ESP32_BLE_01";
ble_gap_adv_start(...);
```

**Demo:** Flashen, nRF Connect öffnen, nach `ESP32_BLE_01` suchen. Noch gibt es keinen eigenen Service – aber das Gerät ist schon sichtbar.

**Merksatz:** Advertising macht sichtbar; GATT kommt im nächsten Schritt.

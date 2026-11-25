# ESP-IDF BLE #2: GAP, GATT, Services und Characteristics

## Short-Idee, ca. 3 Minuten

**Text:** GAP beantwortet: Wie wird das Gerät gefunden und wie wird verbunden? GATT beschreibt danach die Daten. Ein Service ist ein Ordner, eine Characteristic ist ein einzelner Wert darin. Genau wie im Buch bauen wir einen eigenen Service `0xFFF0` mit einer lesbaren Characteristic `0xFFF1`.

**Code zeigen:**

```c
.uuid = BLE_UUID16_DECLARE(0xFFF1),
.flags = BLE_GATT_CHR_F_READ,
```

**Demo:** Verbinden, Service aufklappen, Characteristic lesen: `Hallo BLE`.

**Merksatz:** GAP findet das Gerät; GATT beschreibt, was darin steckt.

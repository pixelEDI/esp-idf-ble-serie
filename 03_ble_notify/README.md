# ESP-IDF BLE #4: Live-Sensordaten mit Notify

## Short-Idee, ca. 3 Minuten

**Text:** Beim Lesen fragt der Client immer wieder nach. Notification ist das Gegenteil: Der ESP32 meldet sich selbst, sobald ein neuer Wert da ist. Dafür aktiviert die App die Notify-Eigenschaft und der ESP32 ruft `ble_gatts_notify_custom()` auf.

**Code zeigen:**

```c
.flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY
ble_gatts_notify_custom(conn, value_handle, ...);
```

**Demo:** Verbinden, Characteristic öffnen, Notify aktivieren. Die Temperaturwerte laufen live ein.

**Merksatz:** Read ist „Gib mir den Wert“; Notify ist „Ich sage dir, wenn er neu ist“.

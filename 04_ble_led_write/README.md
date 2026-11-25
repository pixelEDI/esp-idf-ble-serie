# ESP-IDF BLE #5: LED über Bluetooth steuern

## Short-Idee, ca. 2 Minuten

**Text:** Bisher haben wir gelesen. Jetzt schreibt das Smartphone einen Wert zum ESP32. `0` bedeutet aus, `1` bedeutet an. Die Characteristic bekommt deshalb `WRITE`; im Callback lesen wir das Byte und schalten GPIO 2.

**Code zeigen:**

```c
.flags = BLE_GATT_CHR_F_WRITE;
gpio_set_level(GPIO_NUM_2, b ? 1 : 0);
```

**Demo:** In nRF Connect `01` schreiben: LED an. `00` schreiben: LED aus. Bei manchen Boards ist die LED invertiert oder liegt auf einem anderen GPIO.

**Merksatz:** Write bringt einen Befehl vom Client zum ESP32.

#include "esp_random.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

#define NAME "ESP32_BLE_02"
#define SERVICE_UUID 0x181A
#define CHAR_UUID 0x2A6E

static uint8_t addr_type;

// Zufälligen Temperaturwert generieren
static int access(uint16_t c, uint16_t a, struct ble_gatt_access_ctxt *ctx,
                  void *arg) {
  int16_t temp = 2000 + (esp_random() % 1001);

  return os_mbuf_append(ctx->om, &temp, sizeof(temp)) == 0
             ? 0
             : BLE_ATT_ERR_INSUFFICIENT_RES;
}

// GATT Service und Characteristic
static const struct ble_gatt_svc_def services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SERVICE_UUID),
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = BLE_UUID16_DECLARE(CHAR_UUID),
                    .access_cb = access,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {0}},
    },
    {0}};

static void advertise(void);

// GAP Events
static int gap(struct ble_gap_event *e, void *arg) {
  if (e->type == BLE_GAP_EVENT_DISCONNECT ||
      e->type == BLE_GAP_EVENT_ADV_COMPLETE ||
      (e->type == BLE_GAP_EVENT_CONNECT && e->connect.status != 0)) {
    advertise();
  }

  return 0;
}

// BLE Advertising
static void advertise(void) {
  struct ble_hs_adv_fields f = {0};
  struct ble_gap_adv_params p = {0};

  f.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

  f.name = (uint8_t *)NAME;
  f.name_len = strlen(NAME);
  f.name_is_complete = 1;

  ESP_ERROR_CHECK(ble_gap_adv_set_fields(&f));

  p.conn_mode = BLE_GAP_CONN_MODE_UND;
  p.disc_mode = BLE_GAP_DISC_MODE_GEN;

  ESP_ERROR_CHECK(
      ble_gap_adv_start(addr_type, NULL, BLE_HS_FOREVER, &p, gap, NULL));
}

// Bluetooth bereit
static void sync_cb(void) {
  ESP_ERROR_CHECK(ble_hs_id_infer_auto(0, &addr_type));
  advertise();
}

// NimBLE FreeRTOS Task
static void host(void *p) {
  nimble_port_run();
  nimble_port_freertos_deinit();
}

void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(nimble_port_init());

  ble_hs_cfg.sync_cb = sync_cb;

  ble_svc_gap_init();
  ble_svc_gatt_init();

  ESP_ERROR_CHECK(ble_gatts_count_cfg(services));
  ESP_ERROR_CHECK(ble_gatts_add_svcs(services));

  ESP_ERROR_CHECK(ble_svc_gap_device_name_set(NAME));

  nimble_port_freertos_init(host);
}

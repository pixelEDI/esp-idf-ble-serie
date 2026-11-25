#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdint.h>
#include <string.h>

#define NAME "ESP32_pixeledi_03"
#define SERVICE_UUID 0x181A
#define CHAR_UUID 0x2A6E

static uint8_t addr_type;
static uint16_t value_handle;
static atomic_uint_least16_t conn = BLE_HS_CONN_HANDLE_NONE;
static atomic_int_least16_t value = 2000;
static TimerHandle_t timer;

// Aktuellen Temperaturwert auslesen
static int read_value(uint16_t c, uint16_t a, struct ble_gatt_access_ctxt *ctx,
                      void *arg) {
  int16_t temp = atomic_load(&value);

  return os_mbuf_append(ctx->om, &temp, sizeof(temp)) == 0
             ? 0
             : BLE_ATT_ERR_INSUFFICIENT_RES;
}

// GATT Service mit READ und NOTIFY
static const struct ble_gatt_svc_def svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SERVICE_UUID),
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = BLE_UUID16_DECLARE(CHAR_UUID),
                    .access_cb = read_value,
                    .val_handle = &value_handle,
                    .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                },
                {0}},
    },
    {0}};

// Zufälligen Temperaturwert generieren und senden
static void tick(TimerHandle_t t) {
  uint16_t connection = atomic_load(&conn);

  if (connection == BLE_HS_CONN_HANDLE_NONE)
    return;

  int16_t temp = 2000 + (esp_random() % 1001);
  atomic_store(&value, temp);

  struct os_mbuf *om = ble_hs_mbuf_from_flat(&temp, sizeof(temp));

  if (om != NULL) {
    ble_gatts_notify_custom(connection, value_handle, om);
  }
}

static void adv(void);

// GAP Events
static int gap(struct ble_gap_event *e, void *arg) {
  switch (e->type) {

  case BLE_GAP_EVENT_CONNECT:
    if (e->connect.status != 0)
      adv();
    break;

  case BLE_GAP_EVENT_DISCONNECT:
    atomic_store(&conn, BLE_HS_CONN_HANDLE_NONE);
    xTimerStop(timer, 0);
    adv();
    break;

  case BLE_GAP_EVENT_ADV_COMPLETE:
    adv();
    break;

  case BLE_GAP_EVENT_SUBSCRIBE:
    if (e->subscribe.attr_handle == value_handle) {
      if (e->subscribe.cur_notify) {
        atomic_store(&conn, e->subscribe.conn_handle);
        xTimerStart(timer, 0);
      } else {
        atomic_store(&conn, BLE_HS_CONN_HANDLE_NONE);
        xTimerStop(timer, 0);
      }
    }
    break;
  }

  return 0;
}

// Advertising
static void adv(void) {
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

static void sync_cb(void) {
  ESP_ERROR_CHECK(ble_hs_id_infer_auto(0, &addr_type));
  adv();
}

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

  ESP_ERROR_CHECK(ble_gatts_count_cfg(svcs));
  ESP_ERROR_CHECK(ble_gatts_add_svcs(svcs));
  ESP_ERROR_CHECK(ble_svc_gap_device_name_set(NAME));

  timer = xTimerCreate("ble", pdMS_TO_TICKS(1000), pdTRUE, NULL, tick);

  assert(timer);

  nimble_port_freertos_init(host);
}

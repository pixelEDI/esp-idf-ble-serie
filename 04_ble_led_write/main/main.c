#include "driver/gpio.h"
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <assert.h>
#include <string.h>

#define NAME "ESP32_pixeledi_04"
#define LED GPIO_NUM_15

static uint8_t addr_type;

// Eigene 128-Bit-UUIDs
static const ble_uuid128_t service_uuid =
    BLE_UUID128_INIT(0x01, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                     0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee);

static const ble_uuid128_t char_uuid =
    BLE_UUID128_INIT(0x02, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                     0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee);

// WRITE Callback
static int write_led(uint16_t c, uint16_t a, struct ble_gatt_access_ctxt *ctx,
                     void *arg) {
  uint8_t b = 0;

  if (OS_MBUF_PKTLEN(ctx->om) != 1)
    return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;

  if (os_mbuf_copydata(ctx->om, 0, 1, &b) != 0)
    return BLE_ATT_ERR_UNLIKELY;

  if (b > 1)
    return BLE_ATT_ERR_VALUE_NOT_ALLOWED;

  // Active LOW
  gpio_set_level(LED, b ? 0 : 1);

  return 0;
}

// GATT Service
static const struct ble_gatt_svc_def svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &service_uuid.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = &char_uuid.u,
                    .access_cb = write_led,
                    .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
                },
                {0}},
    },
    {0}};

static void adv(void);

// GAP Events
static int gap(struct ble_gap_event *e, void *arg) {
  if (e->type == BLE_GAP_EVENT_DISCONNECT ||
      e->type == BLE_GAP_EVENT_ADV_COMPLETE ||
      (e->type == BLE_GAP_EVENT_CONNECT && e->connect.status != 0)) {
    adv();
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
  // LED initialisieren und ausschalten
  gpio_reset_pin(LED);
  gpio_set_direction(LED, GPIO_MODE_OUTPUT);
  gpio_set_level(LED, 1);

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(nimble_port_init());

  ble_hs_cfg.sync_cb = sync_cb;

  ble_svc_gap_init();
  ble_svc_gatt_init();

  ESP_ERROR_CHECK(ble_gatts_count_cfg(svcs));
  ESP_ERROR_CHECK(ble_gatts_add_svcs(svcs));
  ESP_ERROR_CHECK(ble_svc_gap_device_name_set(NAME));

  nimble_port_freertos_init(host);
}

#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <assert.h>
#include <string.h>

#define DEVICE_NAME "ESP32_pixeledi"
static uint8_t own_addr_type;

static void advertise(void);
static int gap_event(struct ble_gap_event *event, void *arg) {
  if (event->type == BLE_GAP_EVENT_DISCONNECT ||
      event->type == BLE_GAP_EVENT_ADV_COMPLETE)
    advertise();
  return 0;
}

static void advertise(void) {
  struct ble_hs_adv_fields fields = {0};
  struct ble_gap_adv_params params = {0};
  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  fields.name = (uint8_t *)DEVICE_NAME;
  fields.name_len = strlen(DEVICE_NAME);
  fields.name_is_complete = 1;
  assert(ble_gap_adv_set_fields(&fields) == 0);
  params.conn_mode = BLE_GAP_CONN_MODE_UND;
  params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  assert(ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &params,
                           gap_event, NULL) == 0);
}

static void on_sync(void) {
  assert(ble_hs_id_infer_auto(0, &own_addr_type) == 0);
  advertise();
}
static void host_task(void *arg) {
  nimble_port_run();
  nimble_port_freertos_deinit();
}

void app_main(void) {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(nimble_port_init());
  ble_hs_cfg.sync_cb = on_sync;
  ble_svc_gap_init();
  ble_svc_gatt_init();
  ESP_ERROR_CHECK(ble_svc_gap_device_name_set(DEVICE_NAME));
  nimble_port_freertos_init(host_task);
}

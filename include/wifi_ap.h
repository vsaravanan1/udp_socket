#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// parameters for softAP connection
#define EXAMPLE_ESP_WIFI_SSID      "ieeewifissid"
#define EXAMPLE_ESP_WIFI_PASS      "1234567"
#define EXAMPLE_ESP_WIFI_CHANNEL   6
#define EXAMPLE_MAX_STA_CONN       2

void wifi_init_ap(void);
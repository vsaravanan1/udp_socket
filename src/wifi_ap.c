#include "wifi_ap.h"
// FreeRTOS Event Group that handles a WiFi connection
static EventGroupHandle_t s_wifi_event_group = NULL;
static esp_netif_t* network_interface = NULL;

const char* TAG = "WIFI_SOFTAP";


// event handler for wifi events (when a station connects)
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    }
}

// event handler for IP events (when the softAP assigns an IP address to a station)
static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == IP_EVENT && event_id == IP_EVENT_AP_STAIPASSIGNED) {
        ip_event_ap_staipassigned_t* assigned_ap = (ip_event_ap_staipassigned_t*)(event_data);
        ESP_LOGI(TAG, "Assgined ip address:" IPSTR, IP2STR(&assigned_ap->ip));
    }
}



void wifi_init_ap(void) {
    if (s_wifi_event_group == NULL) {
        s_wifi_event_group = xEventGroupCreate();
    }
    else {
        return;
    }
    
    // starts wifi softAP interface (allows us to use a struct to configure the softAP interface later)
    network_interface = esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN
        }
    };


    
    // these handler instance variables aren't strictly necessary. they're useful if we want to unregister the event handler from the default event loop later
    // any event in the wifi event group will cause event handler to be executed
    esp_event_handler_instance_t instance_any_id;
    // only "sta ip assigned" event in the ip event group will cause event handler to be executed
    esp_event_handler_instance_t instance_got_ip;

    // registers event handler function with specific event in event group
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, ip_event_handler, NULL, &instance_got_ip));
    
    // actual wifi configurationa
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

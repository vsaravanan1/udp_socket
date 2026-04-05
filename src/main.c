#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "wifi_ap.h"

#define PORT 8000
#define RX_BUF_LEN 128

static const char* TAG = "example";

static void udp_server_task(void* pvParameters) {
    char rx_buffer[RX_BUF_LEN];
    char addr_str[RX_BUF_LEN];
    int addr_family = AF_INET;
    int ip_protocol = 0;
    // struct that can hold IPv4 or IPv6 addresses
    struct sockaddr_storage dest_addr;
    
    while (1) {
        // server address configuration
        struct sockaddr_in* dest_addr_ip4 = (struct sockaddr_in*)&dest_addr;
        // configure udp server to listen to any client address
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        // chooses transport layer protocol based on socket type
        ip_protocol = IPPROTO_IP;
        
        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "socket created!");

        // set timeout for receiving data from clients
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        // bind socket to destination address struct
        int err = bind(sock, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind. errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        // client address buffer (filled when message is received)
        struct sockaddr_storage source_addr;
        socklen_t socklen = sizeof(source_addr);

        while (1) {
            ESP_LOGI(TAG, "Waiting for data");
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer)-1, 0, (struct sockaddr*)&source_addr, &socklen);
            // error during reception
            if (len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    ESP_LOGI(TAG, "receive timeout occured!");
                    continue;
                }
                else {
                    ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                    break;   
                }
            }
            // data received
            else {
                if (source_addr.ss_family == AF_INET) {
                    inet_ntoa_r(((struct sockaddr_in*)&source_addr)->sin_addr, addr_str, RX_BUF_LEN);
                    rx_buffer[len] = 0;
                    ESP_LOGI(TAG, "Received %d bytes from %s", len, addr_str);
                    ESP_LOGI(TAG, "%s", rx_buffer);
                }
            
                
                // could reply to client here
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }

    vTaskDelete(NULL);
}





void app_main() {
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_ap();
    xTaskCreatePinnedToCore(udp_server_task, "initialize udp server", 4096, NULL, 3, NULL, APP_CPU_NUM);
}
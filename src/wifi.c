#include <wifi.h>

#include <esp_log.h>
#include <esp_mac.h>
#include <esp_wifi.h>

#include <lwip/inet.h>

#include <string.h>

#define WIFI_AP_SSID           CONFIG_WIFI_AP_SSID
#define WIFI_AP_PASSWORD       CONFIG_WIFI_AP_PASSWORD
#define WIFI_AP_CHANNEL        CONFIG_WIFI_AP_CHANNEL
#define WIFI_AP_MAX_CONNECTION CONFIG_WIFI_AP_MAX_CONNECTION

#ifdef CONFIG_WIFI_AUTH_OPEN
#define WIFI_AP_AUTHMODE WIFI_AUTH_OPEN
#elif CONFIG_WIFI_AUTH_WEB
#define WIFI_AP_AUTHMODE WIFI_AUTH_WEB
#elif CONFIG_WIFI_AUTH_WPA_PSK
#define WIFI_AP_AUTHMODE WIFI_AUTH_WPA_PSK
#elif CONFIG_WIFI_AUTH_WPA2_PSK
#define WIFI_AP_AUTHMODE WIFI_AUTH_WPA2_PSK
#elif CONFIG_WIFI_AUTH_WPA_WPA2_PSK
#define WIFI_AP_AUTHMODE WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_WIFI_AUTH_WPA2_ENTERPRISE
#define WIFI_AP_AUTHMODE WIFI_AUTH_WPA2_ENTERPRISE
#elif CONFIG_WIFI_AUTH_WPA3_PSK
#define WIFI_AP_AUTHMODE WIFI_AUTH_WPA3_PSK
#elif CONFIG_WIFI_AUTH_WPA2_WPA3_PSK
#define WIFI_AP_AUTHMODE WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_WIFI_AUTH_WAPI_PSK
#define WIFI_AP_AUTHMODE WIFI_AUTH_WAPI_PSK
#elif CONFIG_WIFI_AUTH_OWE
#define WIFI_AP_AUTHMODE WIFI_AUTH_OWE
#endif

static const char *TAG = "softap";

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
  void *event_data)
{
  if (event_id == WIFI_EVENT_AP_STACONNECTED)
  {
    wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
  }
  else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
  {
    wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
  }
}

void wifi_init_softap(void)
{
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

  // Initialize and start WiFi
  wifi_config_t wifi_config = {
    .ap = {.ssid = WIFI_AP_SSID,
           .password = WIFI_AP_PASSWORD,
           .ssid_len = strlen(WIFI_AP_SSID),
           .channel = WIFI_AP_CHANNEL,
           .authmode = WIFI_AP_AUTHMODE,
           .max_connection = WIFI_AP_MAX_CONNECTION,
           .pmf_cfg = {.required = true}}
  };
  esp_wifi_set_mode(WIFI_MODE_AP);
  esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
  ESP_ERROR_CHECK(esp_wifi_start());

  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

  char ip_addr[16];
  inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
  ESP_LOGI(TAG, "Set up with IP: %s", ip_addr);

  ESP_LOGI(TAG, "Started. SSID:%s password:%s channel:%d", WIFI_AP_SSID, WIFI_AP_PASSWORD,
    WIFI_AP_CHANNEL);
}

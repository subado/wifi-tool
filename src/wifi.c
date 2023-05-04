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
#define WIFI_AP_IP_ADDR                                                                \
  PP_HTONL(LWIP_MAKEU32(CONFIG_WIFI_AP_IP_A, CONFIG_WIFI_AP_IP_B, CONFIG_WIFI_AP_IP_C, \
    CONFIG_WIFI_AP_IP_D))

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

static const char *AP_TAG = "ap";

static void connected_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
  void *event_data)
{
  wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
  ESP_LOGI(AP_TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
}

static void disconnected_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
  void *event_data)
{
  wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
  ESP_LOGI(AP_TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
}

void wifi_init(void)
{
  // Initialize default sta and ap as network interface instances (esp-netif)
  esp_netif_t *netif = esp_netif_create_default_wifi_sta();
  assert(netif);
  netif = esp_netif_create_default_wifi_ap();
  assert(netif);

  esp_netif_ip_info_t ip_info;
  ip_info.ip.addr = WIFI_AP_IP_ADDR;
  ip_info.gw.addr = WIFI_AP_IP_ADDR;
  IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
  esp_netif_set_ip_info(netif, &ip_info);

  ESP_LOGI(AP_TAG, "Set up with IP: " IPSTR, IP2STR(&ip_info.ip));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &connected_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED,
    &disconnected_handler, NULL));

  // Initialize and start WiFi
  wifi_config_t wifi_config = {
    .ap =
      {
           .ssid = WIFI_AP_SSID,
           .password = WIFI_AP_PASSWORD,
           .ssid_len = strlen(WIFI_AP_SSID),
           .channel = WIFI_AP_CHANNEL,
           .authmode = WIFI_AP_AUTHMODE,
           .max_connection = WIFI_AP_MAX_CONNECTION,
           .pmf_cfg =
           {
           .required = true,
           }, },
  };
  esp_wifi_set_mode(WIFI_MODE_APSTA);
  esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(AP_TAG, "Started. SSID:%s password:%s channel:%d", WIFI_AP_SSID, WIFI_AP_PASSWORD,
    WIFI_AP_CHANNEL);
}

#include <web_server.h>

#include <cJSON.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <esp_wifi.h>

static const char *TAG = "web_server";

static const char *authmode2str(wifi_auth_mode_t authmode)
{
  switch (authmode)
  {
  case WIFI_AUTH_OPEN:
    return "OPEN";
    break;
  case WIFI_AUTH_WEP:
    return "WEP";
    break;
  case WIFI_AUTH_WPA_PSK:
    return "WPA_PSK";
    break;
  case WIFI_AUTH_WPA2_PSK:
    return "WPA2_PSK";
    break;
  case WIFI_AUTH_WPA_WPA2_PSK:
    return "WPA_WPA2_PSK";
    break;
  case WIFI_AUTH_WPA2_ENTERPRISE:
    return "WPA2_ENTERPRISE";
    break;
  case WIFI_AUTH_WPA3_PSK:
    return "WPA3_PSK";
    break;
  case WIFI_AUTH_WPA2_WPA3_PSK:
    return "WPA2_WPA3_PSK";
    break;
  case WIFI_AUTH_WAPI_PSK:
    return "WAPI_PSK";
    break;
  case WIFI_AUTH_OWE:
    return "OWE";
    break;
  default:
    return "";
  }
}
static const char *cipher2str(wifi_cipher_type_t cipher)
{
  switch (cipher)
  {
  case WIFI_CIPHER_TYPE_NONE:
    return "NONE";
    break;
  case WIFI_CIPHER_TYPE_WEP40:
    return "WEP40";
    break;
  case WIFI_CIPHER_TYPE_WEP104:
    return "WEP104";
    break;
  case WIFI_CIPHER_TYPE_TKIP:
    return "TKIP";
    break;
  case WIFI_CIPHER_TYPE_CCMP:
    return "CCMP";
    break;
  case WIFI_CIPHER_TYPE_TKIP_CCMP:
    return "TKIP_CCMP";
    break;
  default:
    return "UNKNOWN";
    break;
  }
}

static esp_err_t wifi_scan_handler(httpd_req_t *req)
{
  esp_wifi_scan_start(NULL, true);

  uint16_t number;
  esp_wifi_scan_get_ap_num(&number);
  wifi_ap_record_t *ap_records = malloc(sizeof *ap_records * number);
  esp_wifi_scan_get_ap_records(&number, ap_records);

  cJSON *root = cJSON_CreateArray(), *ap = NULL;
  char *bssid = NULL;

  for (uint16_t i = 0; i < number; ++i)
  {
    ap = cJSON_CreateObject();

    cJSON_AddStringToObject(ap, "ssid", (const char *)ap_records[i].ssid);
    cJSON_AddStringToObject(ap, "authmode", authmode2str(ap_records[i].authmode));
    cJSON_AddStringToObject(ap, "cipher", cipher2str(ap_records[i].pairwise_cipher));
    cJSON_AddNumberToObject(ap, "rssi", ap_records[i].rssi);
    cJSON_AddNumberToObject(ap, "channel", ap_records[i].primary);

    asprintf(&bssid, MACSTR, MAC2STR(ap_records[i].bssid));
    cJSON_AddStringToObject(ap, "bssid", bssid);

    cJSON_AddItemToArray(root, ap);
    free(bssid);
  }

  httpd_resp_sendstr(req, cJSON_Print(root));

  cJSON_Delete(root);
  free(ap_records);
  return ESP_OK;
}

httpd_handle_t start_web_server(void)
{
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  ESP_ERROR_CHECK(httpd_start(&server, &config));

  httpd_uri_t wifi_scan_get_uri = {
    .uri = "/api/wifi/scan",
    .method = HTTP_GET,
    .handler = wifi_scan_handler,
    .user_ctx = NULL,
  };
  httpd_register_uri_handler(server, &wifi_scan_get_uri);
  ESP_LOGI(TAG, "Started");

  return server;
}

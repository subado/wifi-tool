#pragma once

#include <esp_http_server.h>

httpd_handle_t start_web_server(void);
void stop_web_server(httpd_handle_t server);

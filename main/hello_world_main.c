#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "connect_wifi.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "cJSON.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

#include "esp_crt_bundle.h"
// https://wokwi.com/projects/305566932847821378
#define LED_PIN 2
// skapa en funktion som körs på annan TASK
// 2. Fixa en WIFI-koppling

#define FIRMWARE_VERSION	0.5
#define UPDATE_JSON_URL		"https://raw.githubusercontent.com/aspcodenet/ESP2025-first/refs/heads/wifi/bin/firmware.json"



// receive buffer
char rcv_buffer[200];

// esp_http_client event handler
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    
	switch(evt->event_id) {
        case HTTP_EVENT_REDIRECT:
        case HTTP_EVENT_ERROR:
            break;
        case HTTP_EVENT_ON_CONNECTED:
            break;
        case HTTP_EVENT_HEADER_SENT:
            break;
        case HTTP_EVENT_ON_HEADER:
            break;
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
				strncpy(rcv_buffer, (char*)evt->data, evt->data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            break;
        case HTTP_EVENT_DISCONNECTED:
            break;
    }
    return ESP_OK;
}


// 1. Vi har en KONSTANT = FIRMWARE_VERSION I VÅR KOD
//2, När vi kör programmet kommer FIRMWARE_VERSION 0,1
// 3. Forever
//     Kolla Github https://raw.githubusercontent.com/aspcodenet/project-name2/main/bin/firmware.json
//     Kollar i filen vad är versionen där? 
//          0.2
//   jag är fortfarande 0.1
							//      Ladda ner, sparta på ny partition, sätt ny  active partition, reboot
//          esp_err_t ret = esp_https_ota(&ota_config);

// Programändring



void check_update_task(void *pvParameter) {
	int cnt = 0;
	while(1) {
        char buf[255];
        sprintf(buf, "%s?token=%d",UPDATE_JSON_URL,cnt);
        cnt++;
		printf("Looking for a new firmware at %s", buf);
	
		// configure the esp_http_client
		esp_http_client_config_t config = {
        .url = buf,
         .transport_type = HTTP_TRANSPORT_OVER_SSL,
         .crt_bundle_attach = esp_crt_bundle_attach,
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,        
        .timeout_ms = 30000,
		};
		esp_http_client_handle_t client = esp_http_client_init(&config);
	
		// downloading the json file
		esp_err_t err = esp_http_client_perform(client);
		if(err == ESP_OK) {
			
			// parse the json file	
			cJSON *json = cJSON_Parse(rcv_buffer);
			if(json == NULL) printf("downloaded file is not a valid json, aborting...\n");
			else {	
				cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
				cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");
				
				// check the version
				if(!cJSON_IsNumber(version)) printf("unable to read new version, aborting...\n");
				else {
					
					double new_version = version->valuedouble;
					if(new_version > FIRMWARE_VERSION) {
						
						printf("current firmware version (%.1f) is lower than the available one (%.1f), upgrading...\n", FIRMWARE_VERSION, new_version);
						if(cJSON_IsString(file) && (file->valuestring != NULL)) {
							printf("downloading and installing new firmware (%s)...\n", file->valuestring);
							
							esp_http_client_config_t ota_client_config = {
								.url = file->valuestring,
                                .transport_type = HTTP_TRANSPORT_OVER_SSL,
                                .crt_bundle_attach = esp_crt_bundle_attach,
                                .keep_alive_enable = true,
							};
                            esp_https_ota_config_t ota_config = {
                                    .http_config = &ota_client_config,
                            };                            
							esp_err_t ret = esp_https_ota(&ota_config);
							if (ret == ESP_OK) {
								printf("OTA OK, restarting...\n");
								esp_restart();
							} else {
								printf("OTA failed...\n");
							}
						}
						else printf("unable to read the new file name, aborting...\n");
					}
					else printf("current firmware version (%.1f) is greater or equal to the available one (%.1f), nothing to do...\n", FIRMWARE_VERSION, new_version);
				}
			}
		}
		else printf("unable to download the json file, aborting...\n");
		
		// cleanup
		esp_http_client_cleanup(client);
		
		printf("\n");
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}




void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
    ESP_LOGI("CH", "%d ret", ret);
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    connect_wifi();
    

   
    xTaskCreate(check_update_task, "sendToThingSpeak", 8192, NULL, 5, NULL);

    while(1){
        gpio_set_level(LED_PIN,1 );
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN,0 );
        vTaskDelay(5000 / portTICK_PERIOD_MS);        
    }
}
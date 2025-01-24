#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


// https://wokwi.com/projects/305566932847821378

// 2 stycken CORES - core 0 WIFI


// TASKS - 
// - vänta på klick på knappen och gör nåt -  while no clicked();
// - vänta på input på pinne och gör nåt while not inputonpin();
// - blinka led 2 var 1 sek
// TRE STYCKEN SEPARATA TASKS
// NON BLOCKING -> "rörig kod" - > state machine, alt interupts

#define LED_PIN 2

//Watch dog timer

void app_main(void)
{

    int y = 12;
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    ESP_LOGE("STEFANS-TAG","hj %d japp",y);


    while(1){
        y++;
        ESP_LOGI("MAIN","tjena %d moss",y);
        gpio_set_level(LED_PIN,1 );
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(LED_PIN,0 );
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}
/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "dirent.h"
#include "uto_eth_init.h"
#include "driver/gpio.h"
#include "simple_snmp_agent.h"

static const char *TAG = "main";

// Initialize spiffs 
void init_spiffs(void) 
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else {
        size_t total = 0, used = 0;
        esp_spiffs_info(NULL, &total, &used);
        ESP_LOGI(TAG, "SPIFFS mounted: total=%d, used=%d", total, used);
    }
}

void list_spiffs_files(const char *path) 
{
    DIR *dir = opendir(path);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory: %s", path);
        return;
    }

    struct dirent *entry;
    ESP_LOGI(TAG, "spiffs Listing files in: %s", path);
    while ((entry = readdir(dir)) != NULL) {
        ESP_LOGI(TAG, "  %s", entry->d_name);
    }

    closedir(dir);
}

/**
 */
#define RUN_LED GPIO_NUM_6	
static void toggle_gpio_task(void *pvParameter)
{	
	// TODO: define a pointer to update your mib...
	u32_t gpiovalue = 0;
	xgpio = &gpiovalue;
	
	// TODO: define your gpio's in struture as your needs...
	leds = (u32_t*) RUN_LED;			// RUN LED
	switches = (u32_t*) GPIO_NUM_53; 	// audio output
	
	//gpio_reset_pin(RUN_LED);	
	gpio_set_direction(RUN_LED, GPIO_MODE_OUTPUT);
	
	while(1) {
		ESP_LOGI(TAG, "run_led=%d",gpiovalue);
		gpio_set_level(RUN_LED, gpiovalue);	
		gpiovalue = !gpiovalue;
			
		vTaskDelay(pdMS_TO_TICKS(1000));  // 1000ms = 1초
	}
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
	init_spiffs();

    printf("Hello world!\n");

    ESP_ERROR_CHECK(esp_event_loop_create_default());	// Create default event loop that running in background
 	uto_eth_init();    // 이더넷 초기화

	xTaskCreate(&toggle_gpio_task, "toggle_gpio_task", 2048, NULL, 5, NULL);
    initialize_snmp();
	
    for (int i = 10; i >= 0; i--) {
        //printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

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

void init_sftp_client_task(void);
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
	init_spiffs();

    printf("Hello world!\n");

    ESP_ERROR_CHECK(esp_event_loop_create_default());	// Create default event loop that running in background
 	uto_eth_init();    // 이더넷 초기화


	list_spiffs_files("/spiffs");

	init_sftp_client_task();

    for (int i = 10; i >= 0; i--) {
        //printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

idf_component_register(SRCS "main.c" "uto_eth_init.c" "sftp_client.c"
                       PRIV_REQUIRES nvs_flash spi_flash spiffs esp_event esp_eth esp_netif lwip 
                       INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT)

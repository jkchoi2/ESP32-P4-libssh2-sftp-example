/*
 * ETHERNET 
 */
#pragma once

#include "esp_eth.h"
#include "esp_netif.h"
#include "ethernet_init.h"
#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

void uto_eth_init(void);
bool is_eth_connected(void);

#ifdef __cplusplus
}
#endif

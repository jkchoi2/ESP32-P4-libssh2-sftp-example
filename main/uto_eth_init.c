#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h" // BIT
#include "uto_eth_init.h"
#include "esp_log.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include <netdb.h>


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define CONNECTED_BIT           BIT0
#define WIFI_FAIL_BIT           BIT1

#define ETH_CONNECTION_TMO_MS   (10000)


#define EXAMPLE_STATIC_IP_ADDR      "192.168.0.132"
#define EXAMPLE_STATIC_NETMASK_ADDR "255.255.255.0"
#define EXAMPLE_STATIC_GW_ADDR      "192.168.0.1"

#define EXAMPLE_MAIN_DNS_SERVER    "168.126.63.1"
#define EXAMPLE_BACKUP_DNS_SERVER  "168.126.63.2"
#define EXAMPLE_STATIC_DNS_SERVER_MAIN      "168.126.63.1"
#define EXAMPLE_STATIC_DNS_SERVER_BACKUP    "168.126.63.2"


// config EXAMPLE_STATIC_DNS_RESOLVE_TEST    y

/* FreeRTOS event group to signal when we are connected*/
//static EventGroupHandle_t s_network_event_group;
static bool eth_connected = false;

static const char *TAG = "uto";


bool is_eth_connected(void) { return eth_connected; }


static esp_err_t example_set_dns_server(esp_netif_t *netif, uint32_t addr, esp_netif_dns_type_t type)
{
    if (addr && (addr != IPADDR_NONE)) {
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = addr;
        dns.ip.type = IPADDR_TYPE_V4;
        ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, type, &dns));
    }
    return ESP_OK;
}

static void example_set_static_ip(esp_netif_t *netif)
{
#if 1
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }
#endif
    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr(EXAMPLE_STATIC_IP_ADDR);
    ip.netmask.addr = ipaddr_addr(EXAMPLE_STATIC_NETMASK_ADDR);
    ip.gw.addr = ipaddr_addr(EXAMPLE_STATIC_GW_ADDR);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    ESP_LOGD(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", EXAMPLE_STATIC_IP_ADDR, EXAMPLE_STATIC_NETMASK_ADDR, EXAMPLE_STATIC_GW_ADDR);
    ESP_ERROR_CHECK(example_set_dns_server(netif, ipaddr_addr(EXAMPLE_MAIN_DNS_SERVER), ESP_NETIF_DNS_MAIN));
    ESP_ERROR_CHECK(example_set_dns_server(netif, ipaddr_addr(EXAMPLE_BACKUP_DNS_SERVER), ESP_NETIF_DNS_BACKUP));
}

void eth_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if( event_base==ETH_EVENT ) {
		switch( event_id ) {
			case ETHERNET_EVENT_CONNECTED:
        		example_set_static_ip(arg);
				break;
			case ETHERNET_EVENT_DISCONNECTED:
				eth_connected= false;
				break;
		}
    } 
	else if (event_base==IP_EVENT ) {
 		if( event_id==IP_EVENT_ETH_GOT_IP) {
        	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        	ESP_LOGI(TAG, "static ip:" IPSTR, IP2STR(&event->ip_info.ip));
        	//xEventGroupSetBits(s_network_event_group, CONNECTED_BIT);
			eth_connected= true;
		}
    }
}

void uto_eth_init(void)
{
    esp_eth_handle_t *eth_handles;
    uint8_t eth_port_cnt = 0;

    // s_network_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());  // Initialize the underlying TCP/IP stack, 한번만 호출해야함.

    ESP_ERROR_CHECK(ethernet_init_all(&eth_handles, &eth_port_cnt));	// Initialize Ethernet driver
    if (eth_port_cnt > 1) {
        ESP_LOGW(TAG, "multiple Ethernet devices detected, the first initialized is to be used!");
    }

	// Use ESP_NETIF_DEFAULT_ETH when just one Ethernet interface is used and you don't need to modify
	// default esp-netif configuration parameters.
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif,  esp_eth_new_netif_glue(eth_handles[0])));	// Attach Ethernet driver to TCP/IP stack

	ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &eth_event_handler, NULL));

    ESP_ERROR_CHECK(esp_eth_start(eth_handles[0]));

#if 0
    /* Waiting until the connection is established (CONNECTED_BIT) */
    EventBits_t bits = xEventGroupWaitBits(s_network_event_group, CONNECTED_BIT,pdFALSE,pdFALSE, pdMS_TO_TICKS(ETH_CONNECTION_TMO_MS));

    if( !(bits & CONNECTED_BIT) )
        ESP_LOGE(TAG, "Ethernet link not connected in %d msecs", ETH_CONNECTION_TMO_MS);
#endif
}

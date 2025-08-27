file(GLOB_RECURSE SNMP_SOURCES "${IDF_PATH}/components/lwip/lwip/src/apps/snmp/*.c" )

set(LWIP_DIR "${IDF_PATH}/components/lwip/lwip")
set(MAIN_DIR ".")
#set(MAIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# SNMPv3 agent
set(lwipsnmp_SRCS
    ${LWIP_DIR}/src/apps/snmp/snmp_asn1.c
    ${LWIP_DIR}/src/apps/snmp/snmp_core.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_icmp.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_interfaces.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_ip.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_snmp.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_system.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_tcp.c
    ${LWIP_DIR}/src/apps/snmp/snmp_mib2_udp.c
    ${LWIP_DIR}/src/apps/snmp/snmp_snmpv2_framework.c
    ${LWIP_DIR}/src/apps/snmp/snmp_snmpv2_usm.c
    ${LWIP_DIR}/src/apps/snmp/snmp_msg.c
    ${LWIP_DIR}/src/apps/snmp/snmpv3.c
    ${LWIP_DIR}/src/apps/snmp/snmp_netconn.c
    ${LWIP_DIR}/src/apps/snmp/snmp_pbuf_stream.c
    ${LWIP_DIR}/src/apps/snmp/snmp_raw.c
    ${LWIP_DIR}/src/apps/snmp/snmp_scalar.c
    ${LWIP_DIR}/src/apps/snmp/snmp_table.c
    ${LWIP_DIR}/src/apps/snmp/snmp_threadsync.c
    ${LWIP_DIR}/src/apps/snmp/snmp_traps.c
)

set(lwipExam_SRCS
    ${MAIN_DIR}/snmp/snmp_private_mib/lwip_prvmib.c
    ${MAIN_DIR}/snmp/snmp_v3/snmpv3_dummy.c
    ${MAIN_DIR}/snmp/snmp_example.c
)

idf_component_register(	SRCS "main.c" "uto_eth_init.c"  "my_mib.c" "simple_snmp_agent.c" ${lwipsnmp_SRCS}
                       	PRIV_REQUIRES nvs_flash spi_flash spiffs esp_event esp_eth esp_netif lwip driver
                       	INCLUDE_DIRS "."
						${IDF_PATH}/components/lwip/lwip/src/apps/snmp
        				${IDF_PATH}/components/lwip/lwip/src/include
					)

#LWIP Options override
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMIB2_STATS=1 -DLWIP_SNMP=1 -DSNMP_USE_RAW=0 -DSNMP_USE_NETCONN=1 -DSNMP_LWIP_MIB2=1")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMIB2_STATS=1 -DLWIP_SNMP=1 -DSNMP_USE_RAW=0 -DSNMP_USE_NETCONN=1 -DSNMP_LWIP_MIB2=1 -DLWIP_STATS=1")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLWIP_SNMP=1 -DLWIP_UDP -D=1  -DMIB2_STATS=1 -DSNMP_USE_RAW=0 -DSNMP_USE_NETCONN=1 -ULWIP_TCP" )
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLWIP_SNMP=1 " )
set_source_files_properties(
  ${IDF_PATH}/components/lwip/lwip/src/apps/snmp/snmp_traps.c
  PROPERTIES COMPILE_FLAGS "-Wno-dangling-pointer")

spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT)

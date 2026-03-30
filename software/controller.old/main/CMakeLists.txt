set(requires esp-tls nvs_flash esp_netif esp_http_server esp_wifi)
idf_build_get_property(target IDF_TARGET)

if(${target} STREQUAL "linux")
    list(APPEND requires esp_stubs protocol_examples_common)
else()
    list(APPEND requires esp_eth)
endif()

idf_component_register(SRCS "controller.c" "softap.c" "http_server.c"
                    INCLUDE_DIRS "." "include"
                    PRIV_REQUIRES ${requires})

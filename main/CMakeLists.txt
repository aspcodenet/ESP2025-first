idf_component_register(SRCS "hello_world_main.c" "connect_wifi.c"
                    PRIV_REQUIRES spi_flash
                    REQUIRES driver
                    REQUIRES esp_http_client
                    REQUIRES nvs_flash
                    REQUIRES esp_wifi
                    INCLUDE_DIRS "")

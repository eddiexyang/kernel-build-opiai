set(drv_mipi_depends drv_osal)
add_device_ko(LOCAL_MODULE drv_mipi_rx
                        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
                        TARGETE_DPENDS "${drv_mipi_depends}")
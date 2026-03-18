set(acodec_depends drv_aio)
add_device_ko(LOCAL_MODULE drv_acodec
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../../../
        TARGETE_DPENDS "${acodec_depends}")
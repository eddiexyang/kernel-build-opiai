set(hdmi_depends drv_osal drv_base)
add_device_ko(LOCAL_MODULE drv_hdmi
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        TARGETE_DPENDS "${hdmi_depends}")

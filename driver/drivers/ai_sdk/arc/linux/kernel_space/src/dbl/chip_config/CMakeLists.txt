if(NOT ${PRODUCT} STREQUAL ascend310Brc)
    set(dbl_chip_config_depends drv_pcie)
endif ()

add_device_ko(LOCAL_MODULE dbl_chip_config
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${dbl_chip_config_depends}"
)


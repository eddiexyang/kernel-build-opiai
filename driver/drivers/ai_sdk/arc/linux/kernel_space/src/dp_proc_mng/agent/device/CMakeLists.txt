if(${PRODUCT} STREQUAL ascend310Brc)
    set(drv_dp_proc_mng_depends drv_devmng drv_davinci_intf)
else ()
    set(drv_dp_proc_mng_depends drv_pcie drv_devmng drv_davinci_intf)
endif ()
add_device_ko(LOCAL_MODULE drv_dp_proc_mng
              KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
              MAKE_ARGS ${MAKE_ARGS}
              TARGETE_DPENDS "${drv_dp_proc_mng_depends}")

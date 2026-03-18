set(soc_scene_list "ascend310rc" "ascend310Brc" "helper310p")
if (${PRODUCT} IN_LIST soc_scene_list)
    set(ascend_xsmem_depends drv_davinci_intf drv_devmng)
else ()
    set(ascend_xsmem_depends drv_davinci_intf drv_devmng drv_pcie)
endif()

add_device_ko(LOCAL_MODULE ascend_xsmem
              KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
              MAKE_ARGS ${MAKE_ARGS}
              TARGETE_DPENDS "${ascend_xsmem_depends}")

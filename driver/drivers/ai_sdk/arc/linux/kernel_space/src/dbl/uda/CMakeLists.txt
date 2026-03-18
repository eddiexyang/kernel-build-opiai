
if (${PRODUCT_SIDE} STREQUAL host)
    if (${PRODUCT} STREQUAL helper310p)
        set(ascend_uda_depends drv_seclib_host)
    else ()
        set(ascend_uda_depends drv_seclib_host drv_davinci_intf_host)
    endif()
    add_host_ko(LOCAL_MODULE ascend_uda
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${ascend_uda_depends}")
else ()
    add_device_ko(LOCAL_MODULE ascend_uda
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS})
endif()

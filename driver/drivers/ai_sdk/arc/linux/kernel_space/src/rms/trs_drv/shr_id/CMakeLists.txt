set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_shrid_depends)
else()
    set(ascend_trs_shrid_depends drv_davinci_intf_host drv_devmng_host ascend_trs_core drv_seclib_host ascend_event_sched_host)
endif()

if (${PRODUCT_SIDE} STREQUAL host)
    add_host_ko(LOCAL_MODULE ascend_trs_shrid
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${ascend_trs_shrid_depends}")
endif()


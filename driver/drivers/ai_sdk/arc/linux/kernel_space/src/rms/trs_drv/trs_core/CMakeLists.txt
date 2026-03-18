set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_core_depends)
else()
    set(ascend_trs_core_depends ascend_trs_nvme_chan ascend_trs_id_allocator drv_davinci_intf drv_devmng ascend_uda ascend_event_sched)
    set(ascend_trs_core_host_depends ascend_trs_nvme_chan ascend_trs_id_allocator drv_davinci_intf_host drv_devmng_host
    drv_seclib_host ascend_uda ascend_event_sched_host)
endif()

if (${PRODUCT_SIDE} STREQUAL host)
    add_host_ko(LOCAL_MODULE ascend_trs_core
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${ascend_trs_core_host_depends}")
else ()
    add_device_ko(LOCAL_MODULE ascend_trs_core
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${ascend_trs_core_depends}")
endif()

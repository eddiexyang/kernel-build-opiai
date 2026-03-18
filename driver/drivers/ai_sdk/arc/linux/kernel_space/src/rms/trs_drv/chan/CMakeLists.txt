set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_nvme_chan_depends)
else()
    set(ascend_trs_nvme_chan_depends ascend_trs_id_allocator ascend_uda)
    set(ascend_trs_nvme_chan_host_depends ascend_trs_id_allocator drv_seclib_host ascend_uda)
endif()

if (${PRODUCT_SIDE} STREQUAL host)
    add_host_ko(LOCAL_MODULE ascend_trs_nvme_chan
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${ascend_trs_nvme_chan_host_depends}")
else ()
    add_device_ko(LOCAL_MODULE ascend_trs_nvme_chan
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${ascend_trs_nvme_chan_depends}")
endif()


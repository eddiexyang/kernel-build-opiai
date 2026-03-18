set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_sec_eh_adapt_depends)
else()
    set(ascend_trs_sec_eh_adapt_depends ascend_trs_core ascend_trs_nvme_chan ascend_trs_id_allocator ascend_soc_resmng drv_vpc_agent ascend_urd)
    #set(ascend_trs_sec_eh_adapt_depends ascend_trs_core ascend_trs_nvme_chan ascend_trs_id_allocator ascend_soc_resmng ascend_trs_cdqm drv_vpc_agent)
endif()

if (${PRODUCT_SIDE} STREQUAL host)
    list(APPEND MAKE_ARGS
        KBUILD_MODPOST_WARN=1
    )
    add_host_ko(LOCAL_MODULE ascend_trs_sec_eh_adapt
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${ascend_trs_sec_eh_adapt_depends}")
else ()
endif()

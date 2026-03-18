set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_soc_platform_depends)
else()
    set(ascend_soc_platform_depends ascend_soc_resmng ascend_trs_pm_adapt ascend_uda drv_pcie_host)
endif()

if (${PRODUCT_SIDE} STREQUAL host)
    list(APPEND MAKE_ARGS
        KBUILD_MODPOST_WARN=1
    )
    add_host_ko(LOCAL_MODULE ascend_soc_platform
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${ascend_soc_platform_depends}")
endif()

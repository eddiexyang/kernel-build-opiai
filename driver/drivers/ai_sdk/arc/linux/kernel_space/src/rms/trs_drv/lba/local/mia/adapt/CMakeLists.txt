set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_mia_adapt_depends)
elseif( ${PRODUCT} STREQUAL as31xm1)
    set(ascend_trs_mia_adapt_depends ascend_trs_pm_adapt ascend_soc_resmng ascend_trs_id_pool ascend_trs_nvme_chan ascend_trs_id_allocator ascend_trs_core drv_pcie)
elseif( ${PRODUCT} STREQUAL ascend310Brc)
    set(ascend_trs_mia_adapt_depends ascend_trs_pm_adapt ascend_soc_resmng ascend_trs_id_pool ascend_trs_nvme_chan ascend_trs_id_allocator ascend_trs_core ascend_virtmng_dev)
else()
    set(ascend_trs_mia_adapt_depends ascend_trs_pm_adapt ascend_soc_resmng ascend_trs_id_pool ascend_trs_nvme_chan ascend_trs_id_allocator ascend_trs_core drv_pcie ascend_virtmng_dev)
endif()

add_device_ko(LOCAL_MODULE ascend_trs_mia_adapt
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${ascend_trs_mia_adapt_depends}")

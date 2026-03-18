set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_pm_adapt_depends)
else()
    set(ascend_trs_pm_adapt_depends ascend_trs_sub_stars ascend_trs_core ascend_trs_nvme_chan ascend_trs_id_allocator ascend_soc_resmng drv_pcie_host drv_devmng_host drv_seclib_host ascend_uda ascend_urd)
    # set(ascend_trs_pm_adapt_depends ascend_trs_sub_stars ascend_trs_cdqm ascend_trs_core ascend_trs_nvme_chan ascend_trs_id_allocator ascend_soc_resmng drv_pcie_host drv_devmng_host drv_seclib_host ascend_uda)
endif()

add_host_ko(LOCAL_MODULE ascend_trs_pm_adapt
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${ascend_trs_pm_adapt_depends}")

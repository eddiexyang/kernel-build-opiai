set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_adapt_agent_depends)
else()
    set(ascend_trs_adapt_agent_depends ascend_trs_pm_adapt ascend_trs_core ascend_trs_id_pool drv_pcie ascend_soc_resmng)
    #set(ascend_trs_adapt_agent_depends ascend_trs_pm_adapt ascend_trs_cdqm ascend_trs_core ascend_trs_id_pool drv_pcie ascend_soc_resmng)
endif()

add_device_ko(LOCAL_MODULE ascend_trs_adapt_agent
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${ascend_trs_adapt_agent_depends}")

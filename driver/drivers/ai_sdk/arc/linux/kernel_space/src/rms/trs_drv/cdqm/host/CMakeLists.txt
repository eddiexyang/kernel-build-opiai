set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_cdqm_depends)
else()
    set(ascend_trs_cdqm_depends drv_davinci_intf_host ascend_trs_id_allocator drv_devmng_host ascend_soc_resmng drv_seclib_host drv_pcie_host)
endif()

add_host_ko(LOCAL_MODULE ascend_trs_cdqm
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${ascend_trs_cdqm_depends}")




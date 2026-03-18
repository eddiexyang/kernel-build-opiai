set(product_list "ascend310p" "ascend910" "ascend910B" "ascend920esl" "ascend310B" "ascend310Besl")
if (${PRODUCT} IN_LIST product_list)
        set(drv_virtmng_host_depends drv_pcie_host drv_seclib_host drv_vascend_stub drv_vpc_host ascend_uda ascend_soc_resmng)
else ()
        set(drv_virtmng_host_depends drv_pcie_host drv_seclib_host ascend_uda ascend_soc_resmng)
endif ()

add_host_ko(LOCAL_MODULE drv_virtmng_host
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${drv_virtmng_host_depends}")


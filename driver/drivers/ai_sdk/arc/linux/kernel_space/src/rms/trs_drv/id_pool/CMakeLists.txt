set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_id_pool_depends)
else()
    set(ascend_trs_id_pool_depends)
endif()

add_device_ko(LOCAL_MODULE ascend_trs_id_pool
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${ascend_trs_id_pool_depends}")

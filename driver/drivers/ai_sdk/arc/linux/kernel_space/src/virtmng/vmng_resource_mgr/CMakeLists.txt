set(ascend_virtmng_resmgr_depends drv_davinci_intf ascend_virtmng_dev)
add_device_ko(LOCAL_MODULE ascend_virtmng_resmgr
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${ascend_virtmng_resmgr_depends}")


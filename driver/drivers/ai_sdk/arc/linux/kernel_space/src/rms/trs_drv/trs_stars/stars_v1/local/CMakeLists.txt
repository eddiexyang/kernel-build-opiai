set(trs_stars_depends ascend_soc_resmng)
add_device_ko(LOCAL_MODULE ascend_trs_stars
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${ascend_trs_stars_depends}")

set(ascend_trs_sub_stars_depends ascend_soc_resmng ascend_uda)
add_host_ko(LOCAL_MODULE ascend_trs_sub_stars
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${ascend_trs_sub_stars_depends}")

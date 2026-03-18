set(kunpeng_trs_pm_adapt_depends ascend_trs_nvme_chan ascend_trs_id_allocator ascend_soc_resmng ascend_trs_id_pool ascend_trs_core)

add_device_ko(LOCAL_MODULE kunpeng_trs_pm_adapt
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${kunpeng_trs_pm_adapt_depends}")


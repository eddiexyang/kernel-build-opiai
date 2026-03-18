set(product_list "helper310p")

if("${PRODUCT}" IN_LIST product_list)
    set(ascend_trs_sec_eh_agent_depends)
else()
    set(ascend_trs_sec_eh_agent_depends ascend_trs_pm_adapt)
    #set(ascend_trs_sec_eh_agent_depends ascend_trs_cdqm ascend_trs_pm_adapt)
endif()

list(APPEND MAKE_ARGS
    KBUILD_MODPOST_WARN=1
)

add_host_ko(LOCAL_MODULE ascend_trs_sec_eh_agent
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${ascend_trs_sec_eh_agent_depends}")

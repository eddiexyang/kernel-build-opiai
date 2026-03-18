#aos llvm build
if(AOSCORE_LLVM_MUSL)
    add_subdirectory(comm)
else()
    #linux gcc build
    if( ${PRODUCT} STREQUAL ascend310p)
        set(ascend_event_sched_depends drv_pcie ascend_virtmng_dev drv_devmng hsm_firmware_update hsm_flash hsm_heartbeat)
    elseif( ${PRODUCT} STREQUAL ascend310Besl)
        set(ascend_event_sched_depends drv_pcie ascend_virtmng_dev drv_devmng drv_user_cfg drv_platform ascend_trs_nvme_chan)
    elseif( ${PRODUCT} STREQUAL ascend310B)
        set(ascend_event_sched_depends drv_pcie ascend_virtmng_dev drv_devmng drv_user_cfg drv_platform ascend_trs_nvme_chan)
    elseif( ${PRODUCT} STREQUAL as31xm1)
        set(ascend_event_sched_depends drv_pcie drv_devmng drv_devdrv drv_user_cfg drv_platform)
    elseif( ${PRODUCT} STREQUAL ascend310Brc)
        set(ascend_event_sched_depends ascend_virtmng_dev drv_devmng drv_user_cfg drv_platform ascend_trs_nvme_chan)
    elseif( ${PRODUCT} STREQUAL ascend310Brcesl)
        set(ascend_event_sched_depends ascend_virtmng_dev drv_devmng drv_user_cfg drv_platform ascend_trs_nvme_chan)
    elseif( ${PRODUCT} STREQUAL ascend910B)
        set(ascend_event_sched_depends drv_pcie ascend_virtmng_dev drv_devmng drv_platform ascend_trs_nvme_chan)
    else ()
        set(ascend_event_sched_depends drv_pcie ascend_virtmng_dev drv_devmng)
    endif ()
    add_device_ko(LOCAL_MODULE ascend_event_sched
                KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
                MAKE_ARGS ${MAKE_ARGS}
                TARGETE_DPENDS "${ascend_event_sched_depends}")
endif()

if(AOSCORE_LLVM_MUSL)
    add_library(drv_devdrv_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_devdrv)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
    set(DRIVER_SRC_BASE_DIR ${DRIVER_KERNEL_DIR}/src)

    target_compile_definitions(drv_devdrv_obj PRIVATE AOS_LLVM_BUILD)
    target_compile_definitions(drv_devdrv_obj PRIVATE CFG_FEATURE_IPC_NOTIFY)
    target_compile_definitions(drv_devdrv_obj PRIVATE CFG_SOC_PLATFORM_MINIV2)
    target_compile_definitions(drv_devdrv_obj PRIVATE CFG_FEATURE_RUNTIME_NO_THREAD)
    target_compile_definitions(drv_devdrv_obj PRIVATE CFG_SOC_PLATFORM_MDC)
    target_compile_definitions(drv_devdrv_obj PRIVATE CFG_SOC_PLATFORM_MDC_V51)
    target_compile_definitions(drv_devdrv_obj PRIVATE SUPPORT_FLEXIBLE_BIND_CORE)
    target_link_libraries(drv_devdrv_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

    target_include_directories(drv_devdrv_obj PRIVATE
        ${DRIVER_KERNEL_DIR}/inc
        ${TOP_DIR}/libc_sec/include
        ${TOP_DIR}/inc/driver
        ${TOP_DIR}/inc/soc/hi1910p
        ${TOP_DIR}/drivers/config/user_config/miniv2
        ${TOP_DIR}/config/user_config/miniv2
        ${DRIVER_SRC_BASE_DIR}/drv_devmng/drv_devmng_inc/
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_dev
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_id
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/shm
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/logic
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_platform/ts_platform_device/ascend610
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_platform/ts_platform_device
        ${DRIVER_SRC_BASE_DIR}/drv_devmng/drv_devmng_device/ascend910
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_device
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_device/ascend610
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_device/ascend610/tsdrv_conflict_check
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_conflict_check/ascend610
        ${DRIVER_SRC_BASE_DIR}/common
        ${DRIVER_KERNEL_DIR}/src/drv_notify
        ${DRIVER_KERNEL_DIR}/src/event_sched/device/comm/
        ${TOP_DIR}/abl/bbox/inc/bbox
        ${TOP_DIR}/inc/toolchain/bbox
        ${TOP_DIR}/inc/aicpu/common
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/devdrv_callback
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_device/tsdrv_virt
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/shm
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_nvme/logic
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_common/tsdrv_osal
        ${DRIVER_SRC_BASE_DIR}/tsdrv/ts_drv/ts_drv_vm
    )

    target_sources(drv_devdrv_obj PRIVATE
        ts_drv_common/devdrv_cqsq.c
        ts_drv_common/devdrv_devinit.c
        tsdrv_drvops.c
        tsdrv_get_ssid.c
        devdrv_devinfo.c
        ts_drv_common/devdrv_functional_cqsq.c
        ts_drv_common/devdrv_mailbox.c
        ts_drv_common/tsdrv_delay_recycle.c
        ts_drv_common/devdrv_id.c
        ts_drv_common/devdrv_recycle.c
        ts_drv_common/devdrv_shm.c
        ts_drv_common/tsdrv_dev/tsdrv_device.c
        ts_drv_common/tsdrv_dev/tsdrv_ctx.c
        ts_drv_common/tsdrv_dev/tsdrv_capacity.c
        ts_drv_common/tsdrv_dev/tsdrv_hwinfo.c
        ts_drv_common/tsdrv_dev/tsdrv_hwinfo_platform.c
        ts_drv_common/tsdrv_dev/tsdrv_dev_dfx.c
        ts_drv_common/tsdrv_dev/tsdrv_id_dfx.c
        ts_drv_common/tsdrv_dev/tsdrv_mailbox_dfx.c
        ts_drv_common/tsdrv_dev/tsdrv_comm_fops.c
        ts_drv_common/tsdrv_dev/tsdrv_ctx_dfx.c
        ts_drv_common/tsdrv_dev/tsdrv_event_id_dfx.c
        ts_drv_common/tsdrv_dev/tsdrv_id_config_dfx.c
        ts_drv_common/tsdrv_id/tsdrv_id.c
        ts_drv_common/tsdrv_id/tsdrv_sync.c
        ts_drv_common/tsdrv_nvme/tsdrv_nvme.c
        ts_drv_common/tsdrv_nvme/shm/shm_sqcq.c
        ts_drv_common/tsdrv_nvme/logic/logic_cq.c
        ts_drv_common/tsdrv_nvme/logic/tsdrv_logic_cq.c
        ts_drv_common/devdrv_callback/devdrv_cb.c
        ts_drv_common/devdrv_callback/devdrv_cbctrl.c
        ts_drv_common/devdrv_callback/tsdrv_cb_pm.c
        ts_drv_common/devdrv_ipc_notify.c
        ts_drv_common/devdrv_nsm.c
        ascend610/devdrv_config_p2p.c
        ts_drv_common/tsdrv_conflict_check/ascend610/tsdrv_set_runtime_conflict_check.c
        ascend610/tsdrv_conflict_check/tsdrv_bar_init.c
        ts_drv_common/tsdrv_rts_streamid.c
        ts_drv_common/tsdrv_osal/tsdrv_osal_intr.c
        ts_drv_common/tsdrv_osal/tsdrv_osal_mm.c
        ts_drv_common/tsdrv_osal/tsdrv_osal_bbox.c
        ascend610/tsdrv_status.c
        ascend610/tsdrv_ts_node.c
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_devdrv_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS drv_devdrv_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(drv_devdrv ALL DEPENDS drv_devdrv_obj ${DEVICE_LOCAL_MODULE}.ko)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)
else()
    set(product_list "helper310p")
    if("${PRODUCT}" IN_LIST product_list)
    set(drv_devdrv_depends  drv_devmng svm ascend_event_sched)
    elseif( ${PRODUCT} STREQUAL ascend310p)
    set(drv_devdrv_depends  drv_devmng drv_pcie drv_dfm mntn_bbox drv_davinci_intf svm hsm_firmware_update hsm_flash
    hsm_heartbeat ascend_event_sched)
    elseif( ${PRODUCT} STREQUAL ascend310Besl)
    set(drv_devdrv_depends  drv_devmng drv_davinci_intf drv_pcie drv_dfm mntn_bbox ascend_event_sched)
    elseif( ${PRODUCT} STREQUAL ascend310B)
    set(drv_devdrv_depends  drv_devmng drv_davinci_intf drv_pcie drv_dfm mntn_bbox ascend_event_sched)
    elseif( ${PRODUCT} STREQUAL as31xm1)
    set(drv_devdrv_depends  drv_devmng drv_davinci_intf drv_pcie drv_dfm mntn_bbox ascend_event_sched)
    elseif( ${PRODUCT} STREQUAL ascend610)
    set(drv_devdrv_depends  drv_devmng drv_davinci_intf drv_pcie drv_dfm mntn_bbox ascend_event_sched)
    elseif( ${PRODUCT} STREQUAL bs9sx1a)
    set(drv_devdrv_depends  drv_devmng drv_davinci_intf drv_pcie drv_dfm mntn_bbox ascend_event_sched)
    else()
    set(drv_devdrv_depends  drv_devmng drv_davinci_intf drv_pcie drv_dfm mntn_bbox svm ascend_event_sched)
    endif()
    list(APPEND MAKE_ARGS
        KBUILD_MODPOST_WARN=1
    )
    add_device_ko(LOCAL_MODULE drv_devdrv
                KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
                MAKE_ARGS ${MAKE_ARGS}
                TARGETE_DPENDS "${drv_devdrv_depends}")
endif()

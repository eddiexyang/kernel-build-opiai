if(AOSCORE_LLVM_MUSL)
    add_library(drv_platform_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_platform)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
    set(DRIVER_SRC_BASE_DIR ${DRIVER_KERNEL_DIR}/src)

    include(${FEATURE_CMAKE_PATH})

    target_compile_definitions(drv_platform_obj PRIVATE AOS_LLVM_BUILD)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_FEATURE_TS_IPC)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_SOC_PLATFORM_MINI)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_SOC_PLATFORM_MINIV2)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_FEATURE_FAULT)
    target_compile_definitions(drv_platform_obj PRIVATE DCFG_FEATURE_IPC_CRC)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_FEATURE_TS_HB_DFX)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_SOC_PLATFORM_MDC_V51)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_FEATURE_FAULT_FPDC)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_FEATURE_BOARD_ID_EXPAND)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_FEATURE_SAFETY_FAULT)
    target_compile_definitions(drv_platform_obj PRIVATE CFG_FEATURE_PARTIAL_GOOD)
    target_compile_definitions(drv_platform_obj PRIVATE SNAPSHOT_ENABLE)
    target_compile_definitions(drv_platform_obj PRIVATE ${CONFIG_DEFINES})
    target_compile_definitions(drv_platform_obj PRIVATE COMMON_FUNC_PREFIX=tsdrv)
    target_link_libraries(drv_platform_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

    target_include_directories(drv_platform_obj PRIVATE
        ${TOP_DIR}/inc
        ${TOP_DIR}/drivers/firmware/hiss/inc
        ${TOP_DIR}/tee/inc/tee
        ${TOP_DIR}/tee/inc
        ${TOP_DIR}/inc/tee
        ${TOP_DIR}/inc/driver
        ${TOP_DIR}/abl/bbox/inc/bbox
        ${TOP_DIR}/abl/bbox/inc/bbox/device
        ${TOP_DIR}/inc/toolchain/bbox
        ${TOP_DIR}/inc/toolchain/bbox/device
        ${TOP_DIR}/abl/libc_sec/include
        ${TOP_DIR}/inc/aicpu/common

        ${DRIVER_KERNEL_DIR}/inc
        ${DRIVER_KERNEL_DIR}/inc/ascend_platform
        ${DRIVER_KERNEL_DIR}/src/drv_davinci_intf/drv_davinci_intf_device
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_vm
        ${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_inc
        ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor
        ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/config
        ${DRIVER_KERNEL_DIR}/src/dms/config
        ${DRIVER_KERNEL_DIR}/src/dms/smf/event
        ${DRIVER_KERNEL_DIR}/src/dms/status
        ${DRIVER_KERNEL_DIR}/src/dms/include
        ${DRIVER_KERNEL_DIR}/src/dms/time/device
        ${DRIVER_KERNEL_DIR}/src/dms/dfm_template/
        ${DRIVER_KERNEL_DIR}/src/fpdc
        ${DRIVER_KERNEL_DIR}/src/fpdc/receiver
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_intr
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_ipc
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_mem
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_pdata
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_fault
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_fault/subsys_fault
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_fault/subsys_fault/common
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_fault/subsys_fault/config
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_fault/subsys_fault/aic
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_fault/subsys_fault/aiv
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_fault/subsys_fault/ts
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_pm
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_soc
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_firmware
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_parse
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_heartbeat
        ${DRIVER_KERNEL_DIR}/src/faultmng/fault_fpdc
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_device
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_device/tsdrv_virt
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_common/tsdrv_osal
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_common/tsdrv_dev
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_common/
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_common/tsdrv_sync
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_common/tsdrv_id
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_firmware/tsdrv_file_check

        ${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_device/ascend910/mdc
        ${DRIVER_KERNEL_DIR}/src/drv_notify
        ${TOP_DIR}/inc/soc/hi1910p
        ${TOP_DIR}/drivers/config/user_config/miniv2
        ${TOP_DIR}/config/user_config/miniv2
        ${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_device/ascend910
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_soc/ascend610
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_firmware/ascend610
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/tsdrv_fault/ascend610
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_device/ascend610
    )

    target_sources(drv_platform_obj PRIVATE
        devdrv_platform.c
        tsdrv_intr/tsdrv_intr.c
        tsdrv_pdata/tsdrv_pdata.c
        tsdrv_fault/tsdrv_fault_init.c
        tsdrv_fault/subsys_fault/common/subsys_safety.c
        tsdrv_fault/subsys_fault/aic/aic_dms_init.c
        tsdrv_fault/subsys_fault/aic/aic_safety_disp.c
        tsdrv_fault/subsys_fault/aic/aic_safety_smmu.c
        tsdrv_fault/subsys_fault/aiv/aiv_dms_init.c
        tsdrv_fault/subsys_fault/aiv/aiv_safety_disp.c
        tsdrv_fault/subsys_fault/aiv/aiv_safety_smmu.c
        tsdrv_fault/subsys_fault/ts/ts_dms_init.c
        tsdrv_fault/subsys_fault/ts/ts_safety_disp.c
        tsdrv_fault/subsys_fault/ts/ts_safety_smmu.c
        tsdrv_fault/subsys_fault/ts/ts_safety_sche.c
        tsdrv_soc/tsdrv_soc_smc.c
        tsdrv_firmware/tsdrv_firmware_load.c
        tsdrv_firmware/tsdrv_firmware_reader.c
        tsdrv_pm/tsdrv_pm.c
        tsdrv_parse/tsdrv_parse.c
        tsdrv_heartbeat/tsdrv_heartbeat.c
        tsdrv_heartbeat/tsdrv_heartbeat_dfx.c
        tsdrv_fault/tsdrv_l2buff_safety.c
        tsdrv_fault/tsdrv_subsys_safety.c
        tsdrv_fault/tsdrv_aic_safety.c
        tsdrv_fault/tsdrv_aiv_safety.c
        tsdrv_fault/tsdrv_subsys_safety_ras.c
        tsdrv_fault/tsdrv_safety_fault.c
        ascend610/devdrv_parse_pdata.c
        tsdrv_intr/ascend610/tsdrv_intr_platform.c
        tsdrv_pdata/ascend610/tsdrv_pdata_platform.c
        tsdrv_soc/ascend610/tsdrv_soc_pm.c
        tsdrv_soc/tsdrv_soc_sec.c
        tsdrv_pm/ascend610/tsdrv_pm.c
        tsdrv_firmware/ascend610/tsdrv_adapt_firmware_load.c
        tsdrv_firmware/ascend610/tsdrv_aicpu_config.c
        tsdrv_firmware/tsdrv_file_check/tsdrv_soc_check/tsdrv_file_soc_check.c
        tsdrv_ipc/ascend610/tsdrv_ipc_platform.c
        tsdrv_fault/ascend610/tsdrv_fault_platform.c
        tsdrv_fault/ascend610/tsdrv_ipc_fault_init.c
        tsdrv_fault/ascend610/tsdrv_ipc_fault_rcv.c
        tsdrv_ipc/tsdrv_ipc.c
        tsdrv_ipc/tsdrv_ipc_dfx.c
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_platform_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS drv_platform_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(drv_platform ALL DEPENDS drv_platform_obj ${DEVICE_LOCAL_MODULE}.ko)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)
else()
    set(product_list_910 "ascend910")
    set(product_list_310 "ascend310" "ascend310rc")

    if(${PRODUCT} IN_LIST product_list_910)
        set(drv_platform_depends
                drv_devdrv
                drv_devmng
                drv_fpdc
                drv_icm
                ipc_drv
                drv_pcie
                drv_user_cfg
                tee_drv
                mntn_bbox
            )
    elseif(${PRODUCT} IN_LIST product_list_310)
        set(drv_platform_depends
                drv_devdrv
                drv_devmng
                drv_icm
                ipc_drv
                drv_pcie
                tee_drv
            )
    elseif( ${PRODUCT} STREQUAL ascend310p)
        set(drv_platform_depends drv_devmng drv_pcie tee_drv ipc_drv drv_icm drv_devdrv drv_pkicms hsm_firmware_update hsm_flash hsm_heartbeat)
    elseif( ${PRODUCT} STREQUAL ascend310Besl)
        set(drv_platform_depends drv_pkicms drv_pcie drv_devdrv drv_devmng ipc_drv drv_icm drv_user_cfg hsm_firmware_update hsm_flash hsm_heartbeat)
    elseif( ${PRODUCT} STREQUAL ascend310B)
        set(drv_platform_depends drv_pkicms)
    elseif( ${PRODUCT} STREQUAL ascend310Brc)
        set(drv_platform_depends drv_pkicms drv_pcie drv_devdrv drv_devmng ipc_drv drv_icm drv_user_cfg hsm_firmware_update hsm_flash hsm_heartbeat)
    elseif( ${PRODUCT} STREQUAL ascend310Brcesl)
        set(drv_platform_depends drv_pkicms drv_pcie drv_devdrv drv_devmng ipc_drv drv_icm drv_user_cfg hsm_firmware_update hsm_flash hsm_heartbeat)
    elseif( ${PRODUCT} STREQUAL ascend910B)
        set(drv_platform_depends drv_pkicms drv_devmng drv_pcie drv_devdrv ipc_drv drv_icm)
    elseif( ${PRODUCT} STREQUAL ascend610)
        set(drv_platform_depends drv_pkicms drv_pcie drv_devdrv drv_devmng ipc_drv drv_icm drv_fpdc)
    elseif( (${PRODUCT} STREQUAL ascend610Liteesl) OR (${PRODUCT} STREQUAL ascend610Lite))
        set(drv_platform_depends drv_pkicms drv_pcie drv_devdrv drv_devmng ipc_drv drv_icm drv_fpdc)
    elseif( ${PRODUCT} STREQUAL bs9sx1a)
        set(drv_platform_depends drv_pkicms drv_pcie drv_devdrv drv_devmng ipc_drv drv_icm drv_fpdc)
    else()
        set(drv_platform_depends drv_pkicms)
    endif()
    list(APPEND MAKE_ARGS
        KBUILD_MODPOST_WARN=1
    )
    add_device_ko(LOCAL_MODULE drv_platform
                KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
                MAKE_ARGS ${MAKE_ARGS}
                TARGETE_DPENDS "${drv_platform_depends}")
endif()

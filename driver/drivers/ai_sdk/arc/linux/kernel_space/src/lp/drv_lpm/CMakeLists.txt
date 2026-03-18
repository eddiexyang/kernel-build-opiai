if(AOSCORE_LLVM_MUSL)
    add_library(drv_lpm_fault_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_lpm_fault)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(drv_lpm_fault_obj PRIVATE
        common/lpm_fault_channel.c
        common/lpm_fault_common.c
        report/lpm_fault_config.c
        detect/safety/lpm_fault_crg.c
        dfx/debugfs/lpm_fault_debugfs.c
        dfx/debugfs/lpm_debugfs_base.c
        detect/heartbeat/lpm_fault_heartbeat.c
        common/lpm_fault_ipc.c
        report/lpm_fault_report.c
        detect/safety/lpm_fault_safety.c
        detect/safety/lpm_fault_aosub_safety.c
        detect/startup/lpm_fault_startup.c
        init/lpm_init.c
        external/lpm_fault_from_ipc.c
    )

    target_compile_options(drv_lpm_fault_obj PRIVATE
        -Wall
        -fno-common
        -Wfloat-equal
        -Wformat=2
        -Wcast-align
        -Wundef
        -Wstrict-prototypes
        -Wunused
        -O2
        -Wextra
        -Wno-unused-parameter
        -Wno-sign-compare
        -fstack-protector-all
    )

    target_compile_definitions(drv_lpm_fault_obj PRIVATE AOS_LLVM_BUILD)
    target_link_libraries(drv_lpm_fault_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

    target_include_directories(drv_lpm_fault_obj PRIVATE
        ${DRIVER_CHIP_KERNEL_DIR}/src/lp
        ${DRIVER_KERNEL_DIR}/inc
        ${DRIVER_KERNEL_DIR}/src/dms/include
        ${DRIVER_KERNEL_DIR}/src/dms/config
        ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor
        ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/config
        ${DRIVER_KERNEL_DIR}/src/dms/smf/event
        ${DRIVER_KERNEL_DIR}/src/dms/status
        ${DRIVER_KERNEL_DIR}/src/dms/dfm_template
        ${DRIVER_KERNEL_DIR}/src/fpdc
        ${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_inc
        ${TOP_DIR}/inc/driver
        ${TOP_DIR}/inc/driver/errcode
        ${TOP_DIR}/inc/common
        ${TOP_DIR}/drivers/firmware/hiss/inc
        ${TOP_DIR}/abl/bbox/inc/bbox
        ${TOP_DIR}/abl/ascend_hal/user_space/src/dms
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/common
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/init
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/report
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/detect/heartbeat
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/detect/startup
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/detect/safety
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/detect/ras
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/external
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/dfx
        ${DRIVER_KERNEL_DIR}/src/lp/drv_lpm/dfx/debugfs
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_lpm_fault_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    target_compile_definitions(drv_lpm_fault_obj PRIVATE
        CFG_SOC_PLATFORM_MDC_V51
        LPM_FAULT_RUN_IN_AOS
        LPM_IPC_V2
        LPM_FAULT_HEARTBEAT
        LPM_FAULT_AO_SAFETY
        LPM_FAULT_STARTUP
        LPM_FAULT_SAFETY
        LPM_REPORT_FROM_IPC
        CONFIG_DEBUG_FS
        LPM_REPORT
        LPM_BUILD_DEBUG
        CFG_FEATURE_PARTIAL_GOOD
        DRV_LPM_FAULT
    )

    add_custom_target(drv_lpm_fault ALL DEPENDS drv_lpm_fault_obj ${DEVICE_LOCAL_MODULE}.ko)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)

else()
set(drv_lpm_fault_depends drv_devmng ipc_drv drv_icm)
add_device_ko(LOCAL_MODULE drv_lpm_fault
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${drv_lpm_fault_depends}")
endif()
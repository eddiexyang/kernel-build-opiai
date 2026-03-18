#aos llvm build
if(AOSCORE_LLVM_MUSL)
    add_library(drv_mttcan_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_mttcan)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(drv_mttcan_obj PRIVATE
        bind_core.c
        can_drv_core.c
        can_drv_fault_mgr.c
        can_drv_mttcan.c
        can_drv_api.c
        can_drv_default_cfg.c
        can_drv_interrupt.c
        can_drv_sysfs.c
        can_drv_dms_init.c
        can_drv_feature.c
    )

    target_compile_definitions(drv_mttcan_obj PRIVATE AOS_LLVM_BUILD CFG_SOC_PLATFORM_MDC_V51)
    target_compile_definitions(drv_mttcan_obj PRIVATE CFG_FEATURE_MORE_PID_PRIORITY)
    target_link_libraries(drv_mttcan_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

    target_include_directories(drv_mttcan_obj PRIVATE
        ${DRIVER_CHIP_KERNEL_DIR}/src/mttcan/
        ${DRIVER_KERNEL_DIR}/inc/
        ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/config
        ${DRIVER_KERNEL_DIR}/src/dms/config/
        ${DRIVER_KERNEL_DIR}/src/dms/include/
        ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/
        ${DRIVER_KERNEL_DIR}/src/dms/event/
        ${DRIVER_KERNEL_DIR}/src/dms/smf/event/
        ${DRIVER_KERNEL_DIR}/src/dms/dfm_template
        ${DRIVER_KERNEL_DIR}/src/dms/core
        ${DRIVER_KERNEL_DIR}/src/fpdc
        $(TOP_DIR)/inc/driver

        ${TOP_DIR}/inc/driver/
        ${TOP_DIR}/inc/driver/errcode/
        ${TOP_DIR}/inc/common/
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_mttcan_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    target_compile_definitions(drv_mttcan_obj PRIVATE
        CONFIG_CAN_FAULT_REPORT
        SUPPORT_MTTCAN_FAULT_RECOVERY
        SUPPORT_MTTCAN_DFX
        SUPPORT_FLEXIBLE_BIND_CORE
        RUN_IN_V1
        RUN_IN_AOS
        # aoscore debug add CFG_BUILD_DEBUG
        $<$<CONFIG:Debug>:CFG_BUILD_DEBUG>
    )
    add_custom_target(drv_mttcan ALL DEPENDS drv_mttcan_obj ${DEVICE_LOCAL_MODULE}.ko)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)

else()

    add_device_ko(LOCAL_MODULE drv_mttcan
                KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
                MAKE_ARGS ${MAKE_ARGS})
endif()

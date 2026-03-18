if(AOSCORE_LLVM_MUSL)
    add_library(drv_ts_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_ts)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_compile_definitions(drv_ts_obj PRIVATE AOS_LLVM_BUILD)
    target_compile_definitions(drv_ts_obj PRIVATE CFG_SOC_PLATFORM_MINIV2)
    target_compile_definitions(drv_ts_obj PRIVATE SUPPORT_FLEXIBLE_BIND_CORE)
    target_compile_definitions(drv_ts_obj PRIVATE CFG_SOC_PLATFORM_MDC_V51)
    target_compile_definitions(drv_ts_obj PRIVATE CFG_ASCEND_FLORENCE_ASCEND610)
    target_link_libraries(drv_ts_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

    target_include_directories(drv_ts_obj PRIVATE
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_task_channel/miniv2/
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_task_channel/cloud/
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_device/ascend610/
        ${DRIVER_KERNEL_DIR}/src/common/ 
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_task_channel/
        ${DRIVER_KERNEL_DIR}/src/event_sched/
        ${DRIVER_KERNEL_DIR}/inc/
        ${DRIVER_USER_DIR}/src/event_sched/
        ${TOP_DIR}/inc/driver/
        ${TOP_DIR}/ace/csruntime/inc/tsch/
        ${TOP_DIR}/inc/driver/errcode/
        ${TOP_DIR}/inc/common/
        ${TOP_DIR}/libc_sec/include
        ${TOP_DIR}/abl/bbox/inc/bbox
        ${TOP_DIR}/inc/toolchain/bbox
        ${TOP_DIR}/inc/aicpu/common/
    )

    target_sources(drv_ts_obj PRIVATE
        ts_drv_init.c
        miniv2/ts_drv_sysfs.c
        miniv2/hwts_drv_miniv2.c
        miniv2/ts_aisle_ipc.c
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_ts_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS drv_ts_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(drv_ts ALL DEPENDS drv_platform drv_ts_obj ${DEVICE_LOCAL_MODULE}.ko)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)
else()
    if( ${PRODUCT} STREQUAL ascend310p)
        set(drv_ts_depends drv_dfm ascend_event_sched drv_platform drv_devdrv drv_devmng drv_icm ipc_drv drv_pcie hsm_firmware_update hsm_flash hsm_heartbeat)
    elseif( ${PRODUCT} STREQUAL ascend310Besl)
        set(drv_ts_depends drv_dfm ascend_event_sched drv_devdrv drv_devmng)
    elseif( ${PRODUCT} STREQUAL ascend910B)
        set(drv_ts_depends drv_dfm ascend_event_sched drv_devdrv drv_devmng)
    else ()
        set(drv_ts_depends drv_dfm ipc_drv drv_icm drv_devmng ascend_event_sched drv_platform)
    endif ()
    add_device_ko(LOCAL_MODULE drv_ts
                KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
                MAKE_ARGS ${MAKE_ARGS}
                TARGETE_DPENDS "${drv_ts_depends}")
    #add_device_ko_target(drv_ts ${CMAKE_CURRENT_SOURCE_DIR})
    #add_dependencies(drv_ts drv_dfm)
endif()

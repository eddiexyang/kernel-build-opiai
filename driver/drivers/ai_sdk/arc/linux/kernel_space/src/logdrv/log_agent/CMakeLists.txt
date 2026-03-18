add_library(log_agent_obj OBJECT)
set(DEVICE_LOCAL_MODULE drv_log_agent)
set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

target_sources(log_agent_obj PRIVATE
    log_drv_agent.c
    log_xpc_mng.c
    log_drv_conn/log_drv_sqcq.c
    log_xpc/log_xpc.c
    log_xpc/log_xpc_agent.c
    log_bind_core/drv_cpu_type.c
)

target_include_directories(log_agent_obj PRIVATE
    ${TOP_DIR}/inc/driver
    ${DRIVER_KERNEL_DIR}/inc
    ${DRIVER_KERNEL_DIR}/src/logdrv/
    ${DRIVER_KERNEL_DIR}/src/logdrv/log_agent
    ${DRIVER_KERNEL_DIR}/src/logdrv/log_drv_conn/
    ${DRIVER_KERNEL_DIR}/src/logdrv/log_drv_res/
    ${DRIVER_KERNEL_DIR}/src/logdrv/log_xpc
    ${DRIVER_KERNEL_DIR}/src/logdrv/log_bind_core

    ${DRIVER_KERNEL_DIR}/src/tsdrv/ts_drv/ts_drv_device/
    ${DRIVER_KERNEL_DIR}/src/tsdrv/ts_drv/ts_drv_common/
    ${DRIVER_KERNEL_DIR}/src/tsdrv/ts_drv/ts_drv_common/tsdrv_dev/
    ${DRIVER_KERNEL_DIR}/src/tsdrv/ts_platform/ts_platform_device/
    ${DRIVER_KERNEL_DIR}/src/tsdrv/ts_drv/ts_drv_device/ascend610/
    ${DRIVER_KERNEL_DIR}/src/ipcdrv
    ${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_device/ascend910
)

target_compile_definitions(log_agent_obj PRIVATE CFG_SOC_PLATFORM_MDC_V51)
target_compile_definitions(log_agent_obj PRIVATE CFG_FEATURE_HISTORY_CONFIG)
target_compile_definitions(log_agent_obj PRIVATE AOS_LLVM_BUILD)
target_compile_options(log_agent_obj PRIVATE -Wall)
target_link_libraries(log_agent_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

add_custom_command(
    OUTPUT ${DEVICE_LOCAL_MODULE}.ko
    COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:log_agent_obj> -o ${DEVICE_LOCAL_MODULE}.ko
    DEPENDS log_agent_obj
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND_EXPAND_LISTS
)
add_custom_target(drv_log_agent ALL DEPENDS log_agent_obj ${DEVICE_LOCAL_MODULE}.ko)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)

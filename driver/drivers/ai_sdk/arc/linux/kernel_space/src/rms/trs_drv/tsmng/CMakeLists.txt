if(AOSCORE_LLVM_MUSL) #aos core build add_library(tsmng_obj OBJECT)
    add_library(tsmng_obj OBJECT)
    set(DEVICE_LOCAL_MODULE ascend_trs_tsmng)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(tsmng_obj PRIVATE tsmng_common.c
        ascend910/tsmng_notifier.c
        ascend910/tsmng_pm.c
        ascend910/tsmng_group.c
    )

    target_include_directories(tsmng_obj PRIVATE
        ${TOP_DIR}/inc/driver
        ${TOP_DIR}/libc_sec/include
        ${DRIVER_KERNEL_DIR}/inc
        ${DRIVER_KERNEL_DIR}/src/rms/trs_drv/tsmng
        ${DRIVER_KERNEL_DIR}/src/rms/trs_drv/tsmng/ascend910
        ${DRIVER_KERNEL_DIR}/src/tsdrv/ts_platform/ts_platform_device/ascend610
        ${DRIVER_KERNEL_DIR}/src/dms/include
        ${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_inc
        ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor
        ${DRIVER_KERNEL_DIR}/src/dms/smf/event
        ${DRIVER_KERNEL_DIR}/src/dms/config
        ${DRIVER_KERNEL_DIR}/src/tsdrv/ts_drv/ts_drv_common
        ${DRIVER_KERNEL_DIR}/src/dms/core
        ${DRIVER_KERNEL_DIR}/src/dbl/dev_urd
        ${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_device/ascend910
        ${DRIVER_KERNEL_DIR}/src/common
    )

    target_compile_definitions(tsmng_obj PRIVATE
        AOS_LLVM_BUILD
        CFG_SOC_PLATFORM_MINIV2
        CFG_SOC_PLATFORM_MINI
        CFG_SOC_PLATFORM_MDC_V51
        CFG_FEATURE_IPC_CRC
        CFG_FEATURE_LP_ENABLE
    )

    target_compile_options(tsmng_obj PRIVATE -Wall)

    target_link_libraries(tsmng_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:tsmng_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS tsmng_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(ascend_trs_tsmng ALL DEPENDS tsmng_obj ${DEVICE_LOCAL_MODULE}.ko)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)
else() #linux build
set(ascend_trs_tsmng_depends )
add_device_ko(LOCAL_MODULE ascend_trs_tsmng KO_SRC_FOLDER
              ${CMAKE_CURRENT_SOURCE_DIR}
              MAKE_ARGS ${MAKE_ARGS}
              TARGETE_DPENDS "${ascend_trs_tsmng_depends}")
endif()

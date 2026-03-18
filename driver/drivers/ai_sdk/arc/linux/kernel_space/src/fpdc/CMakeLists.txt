if (NOT ${PRODUCT_SIDE} STREQUAL device)
    return()
endif()

if(AOSCORE_LLVM_MUSL)
    add_library(drv_fpdc_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_fpdc)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(drv_fpdc_obj PRIVATE
        fpdc_main.c
        distribute/fpdc_distribute_notify.c
        receiver/fpdc_ras_receiver.c
        log_restrain/log_restrain.c
    )

    target_compile_definitions(drv_fpdc_obj PRIVATE
	    AOS_LLVM_BUILD
	    CFG_SOC_PLATFORM_MDC_V51
	    DRV_SUPPORT_SYSCOUNT
    )
    target_link_libraries(drv_fpdc_obj PRIVATE
	    $<BUILD_INTERFACE:utils_intf_pub>
    )

    target_include_directories(drv_fpdc_obj PRIVATE
        ${TOP_DIR}/libc_sec/include
        ${TOP_DIR}/inc/driver
        ${DRIVER_KERNEL_DIR}/inc
        ${DRIVER_KERNEL_DIR}/src/dms/include
        ${DRIVER_KERNEL_DIR}/src/dms/config
        ${DRIVER_KERNEL_DIR}/src/fpdc/
        ${DRIVER_KERNEL_DIR}/src/fpdc/include
        ${DRIVER_KERNEL_DIR}/src/fpdc/aoscore
        ${DRIVER_KERNEL_DIR}/src/common
    )

    if ("${PRODUCT}" MATCHES "^ascend61*" )  # ascend610/bs9sx1a
        target_compile_definitions(drv_fpdc_obj PRIVATE CFG_SOC_PLATFORM_MDC)
    endif()

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_fpdc_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS drv_fpdc_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(drv_fpdc ALL DEPENDS drv_fpdc_obj ${DEVICE_LOCAL_MODULE}.ko)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)
else()
   list(APPEND MAKE_ARGS
       PRODUCT_SIDE=device
   )

   add_device_ko(LOCAL_MODULE drv_fpdc
       KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
       MAKE_ARGS ${MAKE_ARGS})
endif()

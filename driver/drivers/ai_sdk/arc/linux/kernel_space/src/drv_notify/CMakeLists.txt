if (AOSCORE_LLVM_MUSL)
    add_library(drv_centre_notify_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_centre_notify)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(drv_centre_notify_obj PRIVATE
        drv_notify.c
    )

    include(${FEATURE_CMAKE_PATH})

    target_compile_definitions(drv_centre_notify_obj PRIVATE
        ${CONFIG_DEFINES}
        AOS_LLVM_BUILD
        CFG_SOC_PLATFORM_MDC_V51
        CFG_FEATURE_BOARD_ID_EXPAND
    )

    target_link_libraries(drv_centre_notify_obj
        PRIVATE $<BUILD_INTERFACE:utils_intf_pub>
    )

    target_include_directories(drv_centre_notify_obj PRIVATE
        ${TOP_DIR}/abl/libc_sec/include
        ${DRIVER_KERNEL_DIR}/inc
        ${DRIVER_KERNEL_DIR}/inc/ascend_platform
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_centre_notify_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS drv_centre_notify_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(drv_centre_notify ALL DEPENDS drv_centre_notify_obj ${DEVICE_LOCAL_MODULE}.ko)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)

else ()
    set(drv_centre_notify_depends )
    add_device_ko(LOCAL_MODULE drv_centre_notify
                KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
                MAKE_ARGS ${MAKE_ARGS}
                TARGETE_DPENDS "${drv_centre_notify_depends}")
endif()

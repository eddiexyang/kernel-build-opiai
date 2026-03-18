if(AOSCORE_LLVM_MUSL)
    add_library(drv_icm_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_icm)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
    set(DRIVER_SRC_BASE_DIR ${DRIVER_KERNEL_DIR}/src)

    target_compile_definitions(drv_icm_obj PRIVATE AOS_LLVM_BUILD)
    target_compile_definitions(drv_icm_obj PRIVATE CFG_SOC_PLATFORM_MINIV2)
    target_compile_definitions(drv_icm_obj PRIVATE CFG_SOC_PLATFORM_MINIV2_MDC)
    target_compile_definitions(drv_icm_obj PRIVATE CFG_SOC_PLATFORM_MDC_V51)
    if ("${PRODUCT}" STREQUAL "ascend610Lite")
        target_compile_definitions(drv_icm_obj PRIVATE CFG_SOC_PLATFORM_MDC_V51_LITE)
    endif()
    target_link_libraries(drv_icm_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

    target_include_directories(drv_icm_obj PRIVATE
        ${DRIVER_KERNEL_DIR}/inc
        ${DRIVER_OPEN_KERNEL_DIR}/inc
    )

    target_sources(drv_icm_obj PRIVATE
        icm_core.c
        icm_interface.c
        icm_ipc.c
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_icm_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS drv_icm_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(drv_icm ALL DEPENDS drv_icm drv_icm_obj ${DEVICE_LOCAL_MODULE}.ko)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)
else()
    set(drv_icm_depends ipc_drv)
    add_device_ko(LOCAL_MODULE drv_icm
                KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
                MAKE_ARGS ${MAKE_ARGS}
                TARGETE_DPENDS "${drv_icm_depends}")
endif()

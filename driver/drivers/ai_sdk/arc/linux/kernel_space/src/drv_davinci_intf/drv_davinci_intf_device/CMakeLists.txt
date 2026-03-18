#aos llvm build
if (AOSCORE_LLVM_MUSL)
    add_library(drv_davinci_intf_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_davinci_intf)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(drv_davinci_intf_obj PRIVATE
        davinci_intf_process.c
        davinci_intf_init.c
        davinci_intf_info.c
    )

    target_compile_definitions(drv_davinci_intf_obj PRIVATE 
        AOS_LLVM_BUILD 
        CFG_SOC_PLATFORM_MDC_V51
    )

    target_link_libraries(drv_davinci_intf_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)
    target_include_directories(drv_davinci_intf_obj PRIVATE
        ${TOP_DIR}/libc_sec/include
        ${DRIVER_KERNEL_DIR}/inc
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_davinci_intf_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS drv_davinci_intf_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(drv_davinci_intf ALL DEPENDS drv_davinci_intf_obj ${DEVICE_LOCAL_MODULE}.ko)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)

else()
#linux gcc build
    set(drv_davinci_intf_depends svm)
    add_device_ko(LOCAL_MODULE drv_davinci_intf
                KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
                MAKE_ARGS ${MAKE_ARGS}
                TARGETE_DPENDS "${drv_davinci_intf_depends}")
endif()

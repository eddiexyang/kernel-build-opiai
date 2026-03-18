list(APPEND MAKE_ARGS
    KBUILD_MODPOST_WARN=1
)

if(AOSCORE_LLVM_MUSL)
    add_library(ascend_urd_obj OBJECT)
    set(DEVICE_LOCAL_MODULE ascend_urd)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(ascend_urd_obj PRIVATE
        urd_init.c
        urd_feature.c
        urd_acc_ctrl.c
        urd_kv.c
        urd_notifier.c
    )

    target_compile_definitions(ascend_urd_obj PRIVATE
	    AOS_LLVM_BUILD
	    CFG_SOC_PLATFORM_MDC_V51
    )

    target_link_libraries(ascend_urd_obj PRIVATE
	    $<BUILD_INTERFACE:utils_intf_pub>
    )

    target_include_directories(ascend_urd_obj PRIVATE
        ${TOP_DIR}/inc/driver
        ${TOP_DIR}/abl/libc_sec/include
        ${DRIVER_KERNEL_DIR}/inc
        ${DRIVER_KERNEL_DIR}/src/common
    )

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )  # ascend610/bs9sx1a
        target_compile_definitions(ascend_urd_obj PRIVATE CFG_BUILD_DEBUG)
    endif()

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:ascend_urd_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS ascend_urd_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(ascend_urd ALL DEPENDS ascend_urd_obj ${DEVICE_LOCAL_MODULE}.ko)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)
else()
    if (${PRODUCT_SIDE} STREQUAL host)
        set(ascend_urd_depends drv_pcie_host drv_seclib_host drv_davinci_intf_host)
        list(APPEND MAKE_ARGS
            KBUILD_MODPOST_WARN=1
        )

        add_host_ko(LOCAL_MODULE ascend_urd
            KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
            MAKE_ARGS ${MAKE_ARGS}
            TARGETE_DPENDS "${ascend_urd_depends}")
    else ()
        if( ${PRODUCT} STREQUAL ascend310)
            set(ascend_urd_depends drv_pcie drv_davinci_intf)
        elseif( ${PRODUCT} STREQUAL ascend310Brc)
            set(ascend_urd_depends drv_davinci_intf drv_user_cfg)
        else ()
            set(ascend_urd_depends drv_pcie drv_davinci_intf drv_user_cfg)
        endif()

        add_device_ko(LOCAL_MODULE ascend_urd
            KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
            MAKE_ARGS ${MAKE_ARGS}
            TARGETE_DPENDS "${ascend_urd_depends}")
    endif()
endif()

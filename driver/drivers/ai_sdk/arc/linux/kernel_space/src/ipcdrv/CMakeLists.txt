#aos llvm build
if (AOSCORE_LLVM_MUSL)
    add_library(ipc_drv_obj OBJECT)
    set(DEVICE_LOCAL_MODULE ipc_drv)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(ipc_drv_obj PRIVATE
        hisi_mailbox.c
        hisi_mailbox_asm.c
        hisi_mailbox_dev.c
        hisi_rproc.c
        drv_snapshot.c
    )

    add_subdirectory(mdc)

    target_compile_definitions(ipc_drv_obj PRIVATE AOS_LLVM_BUILD)
    target_compile_definitions(ipc_drv_obj PRIVATE CFG_SOC_PLATFORM_ESL_FPGA)
    target_compile_definitions(ipc_drv_obj PRIVATE CFG_SOC_PLATFORM_MDC_V51)
    target_compile_definitions(ipc_drv_obj PRIVATE CFG_SOC_PLATFORM_MINIV2)
    target_compile_definitions(ipc_drv_obj PRIVATE CFG_SOC_PLATFORM_MINIV2_MDC)
    target_link_libraries(ipc_drv_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)
    target_include_directories(ipc_drv_obj PRIVATE
        ${TOP_DIR}/abl/libc_sec/include
        ${TOP_DIR}/inc/driver
        ${TOP_DIR}/inc/driver/errcode
        ${TOP_DIR}/inc/common
        ${DRIVER_KERNEL_DIR}/inc
        ${DRIVER_KERNEL_DIR}/src/ipcdrv
        ${DRIVER_KERNEL_DIR}/src/ipcdrv/mdc
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:ipc_drv_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS ipc_drv_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )
    add_custom_target(ipc_drv ALL DEPENDS ipc_drv_obj ${DEVICE_LOCAL_MODULE}.ko)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)
else()
#linux gcc build
# set(ipc_drv_depends drv_devmng)
add_device_ko(LOCAL_MODULE ipc_drv
			KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
			MAKE_ARGS ${MAKE_ARGS}
            # TARGETE_DPENDS "${ipc_drv_depends}")
)
endif()

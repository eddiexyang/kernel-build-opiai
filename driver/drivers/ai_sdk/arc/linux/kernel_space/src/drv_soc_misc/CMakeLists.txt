if (NOT ${PRODUCT_SIDE} STREQUAL device)
    return()
endif()

if(AOSCORE_LLVM_MUSL)
    add_library(drv_soc_misc_obj OBJECT)
    set(DEVICE_LOCAL_MODULE drv_soc_misc)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(drv_soc_misc_obj PRIVATE
        soc_misc_init.c
        soc_misc_common.c
        faultmgr/soc_misc_dms_init.c
        faultmgr/soc_misc_fault.c
        faultmgr/soc_misc_safety.c
        faultmgr/soc_misc_ras.c
        faultmgr/soc_misc_cpucore.c
        faultmgr/l3d/l3d_dms_init.c
        faultmgr/l3d/l3d_safety.c
        faultmgr/l3d/l3d_arm_ras.c
        faultmgr/l3d/l3d_irq_ras.c
        faultmgr/l3t/l3t_dms_init.c
        faultmgr/l3t/l3t_safety.c
        faultmgr/l3t/l3t_arm_ras.c
        faultmgr/l3t/l3t_irq_ras.c
        faultmgr/soc_misc_ring.c
        faultmgr/soc_misc_rbrg.c
        faultmgr/soc_misc_mn.c
        faultmgr/soc_misc_cpucluster.c
        faultmgr/soc_misc_io_subsys.c
        faultmgr/soc_misc_hac_subsys.c
        faultmgr/soc_misc_peri_subsys.c
        faultmgr/soc_misc_mbigen_peri.c
        faultmgr/soc_misc_mbigen_io.c
        faultmgr/soc_misc_gic_ao.c
        faultmgr/soc_misc_err_info.c

        faultmgr/soc_misc_safety_conf.c
    )

    target_compile_definitions(drv_soc_misc_obj PRIVATE
        AOS_LLVM_BUILD
        CFG_FEATURE_FAULT_MANAGER
        CFG_FEATURE_SAFETY_MANAGER
        CFG_FEATURE_BASE_FAULT
        CFG_FEATURE_RING_FAULT
        CFG_FEATURE_MN_FAULT
        CFG_SOC_PLATFORM_MDC_V51
        CFG_FEATURE_RBRG_FAULT
        CFG_FEATURE_DFM_RAS_FAULT
    )

    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        target_compile_definitions(drv_soc_misc_obj PRIVATE
            CFG_BUILD_DEBUG
        )
    endif()

    target_link_libraries(drv_soc_misc_obj PRIVATE
	    $<BUILD_INTERFACE:utils_intf_pub>
    )

    target_include_directories(drv_soc_misc_obj PRIVATE
        ${AOS_CORE_INC_DIR}/build/include/core
        ${TOP_DIR}/inc/driver
        ${TOP_DIR}/libc_sec/include
        ${TOP_DIR}/abl/ascend_hal/user_space/src/dms
        ${TOP_DIR}/drivers/ai_sdk/driver/linux/kernel_space/src/PCA6416
        ${DRIVER_KERNEL_DIR}/inc
        ${DRIVER_KERNEL_DIR}/src/dbl/dev_urd
        ${DRIVER_KERNEL_DIR}/src/dms/include
        ${DRIVER_KERNEL_DIR}/src/dms/config
        ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor
        ${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/config
        ${DRIVER_KERNEL_DIR}/src/dms/smf/event
        ${DRIVER_KERNEL_DIR}/src/dms/core
        ${DRIVER_KERNEL_DIR}/src/dms/dfm_template
        ${DRIVER_KERNEL_DIR}/src/fpdc
        ${DRIVER_KERNEL_DIR}/src/faultmng/fault_fpdc
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_common
        ${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
        ${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_inc
        ${DRIVER_KERNEL_DIR}/src/drv_soc_misc
        ${DRIVER_KERNEL_DIR}/src/drv_soc_misc/features
        ${DRIVER_KERNEL_DIR}/src/drv_soc_misc/chipinfo
        ${DRIVER_KERNEL_DIR}/src/drv_soc_misc/faultmgr/ascend610
        ${DRIVER_KERNEL_DIR}/src/drv_soc_misc/faultmgr/l3d
        ${DRIVER_KERNEL_DIR}/src/drv_soc_misc/faultmgr/l3t
        ${DRIVER_KERNEL_DIR}/src/drv_soc_misc/faultmgr
        ${DRIVER_KERNEL_DIR}/src/drv_notify
        # for temporary compile
        ${DRIVER_KERNEL_DIR}/src/fpdc/aoscore
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_soc_misc_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS drv_soc_misc_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(drv_soc_misc ALL DEPENDS drv_soc_misc_obj ${DEVICE_LOCAL_MODULE}.ko)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)

else() # for AOS-Linux

    if( ${PRODUCT} STREQUAL ascend310p)
        set(drv_soc_misc_depends drv_pcie drv_devmng hsm_firmware_update hsm_flash hsm_heartbeat)
    elseif ( ${PRODUCT} STREQUAL ascend910)
        set(drv_soc_misc_depends drv_devmng drv_fpdc drv_pcie)
    elseif( ${PRODUCT} STREQUAL ascend310Besl)
        set(drv_soc_misc_depends drv_devmng drv_user_cfg)
    elseif( ${PRODUCT} STREQUAL ascend310B)
        set(drv_soc_misc_depends drv_devmng drv_user_cfg)
    elseif( ${PRODUCT} STREQUAL ascend310Brc)
        set(drv_soc_misc_depends drv_devmng drv_user_cfg)
    elseif( ${PRODUCT} STREQUAL as31xm1)
        set(drv_soc_misc_depends drv_devmng drv_user_cfg)
    elseif( ${PRODUCT} STREQUAL ascend310Brcesl)
        set(drv_soc_misc_depends drv_devmng drv_user_cfg)
    elseif ( ${PRODUCT} STREQUAL ascend910B)
        set(drv_soc_misc_depends drv_devmng drv_pcie drv_fpdc)
    elseif ( ${PRODUCT} STREQUAL ascend610)
        set(drv_soc_misc_depends drv_devmng drv_fault drv_pcie drv_fpdc)
    elseif ( ${PRODUCT} STREQUAL bs9sx1a)
        set(drv_soc_misc_depends drv_devmng drv_fault drv_pcie drv_fpdc)
    elseif ( ${PRODUCT} STREQUAL ascend610Lite)
        set(drv_soc_misc_depends drv_devmng drv_fault drv_pcie drv_fpdc)
    elseif ( ${PRODUCT} STREQUAL ascend610Liteesl)
        set(drv_soc_misc_depends drv_devmng drv_fault drv_pcie drv_fpdc)
    else ()
        set(drv_soc_misc_depends drv_devmng)
    endif ()

    list(APPEND MAKE_ARGS
            PRODUCT_SIDE=device
            KBUILD_MODPOST_WARN=1
        )
    add_device_ko(LOCAL_MODULE drv_soc_misc
        KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
        MAKE_ARGS ${MAKE_ARGS}
        TARGETE_DPENDS "${drv_soc_misc_depends}")
endif()

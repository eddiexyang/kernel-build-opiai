if (${PRODUCT_SIDE} STREQUAL device)
	if (AOSCORE_LLVM_MUSL)
		#aos llvm build
		add_library(drv_devmng_obj OBJECT)
		#add_subdirectory(adapt)
		add_subdirectory(config)
		add_subdirectory(core)
		#add_subdirectory(custom)
		#add_subdirectory(devid)
		add_subdirectory(event)
		add_subdirectory(power)
		#add_subdirectory(sensor)
		add_subdirectory(status)
		add_subdirectory(time)
		add_subdirectory(ts)
		add_subdirectory(product)
		add_subdirectory(aos_core)
		add_subdirectory(xpc)

		set(DEVICE_LOCAL_MODULE drv_devmng)
		set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

		target_compile_definitions(drv_devmng_obj PRIVATE
			SUPPORT_ASCEND_PM
			#SUPPORT_ASCEND_IPC
			AOS_LLVM_BUILD
			CFG_SOC_PLATFORM_MINIV2
			CFG_SOC_PLATFORM_MDC_V51
			CFG_FEATURE_PARTIAL_GOOD
			CFG_FEATURE_LP_ENABLE
			DRV_SUPPORT_SYSCOUNT
			CFG_SURPPORT_PCIE_HOST_DEVICE_COMM
		)
		if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
			target_compile_definitions(drv_devmng_obj PRIVATE CFG_BUILD_DEBUG)
		endif()

		target_link_libraries(drv_devmng_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)
		target_compile_options(drv_devmng_obj PRIVATE
			-Werror
		)

		target_include_directories(drv_devmng_obj PRIVATE
			${DRIVER_KERNEL_DIR}/src/dms/config
			${DRIVER_KERNEL_DIR}/src/dms/core
			${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_device/ascend910
			${TOP_DIR}/abl/bbox/inc/bbox
			${DRIVER_KERNEL_DIR}/src/dms/smf/event
			${DRIVER_KERNEL_DIR}/src/dms/event
			${DRIVER_KERNEL_DIR}/src/virtmng
			${DRIVER_KERNEL_DIR}/src/dms/smf/sensor
			${DRIVER_KERNEL_DIR}/src/dms/dtm
			${DRIVER_KERNEL_DIR}/src/dms/smf
			${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/config
			${DRIVER_KERNEL_DIR}/src/dms/status
			${DRIVER_KERNEL_DIR}/src/dms/time
			${DRIVER_KERNEL_DIR}/src/dms/time/device
			${DRIVER_KERNEL_DIR}/src/dms/ts
			${DRIVER_KERNEL_DIR}/src/dms/power
			${DRIVER_KERNEL_DIR}/src/dbl/drv_urd
			${DRIVER_KERNEL_DIR}/src/dms/product
			${DRIVER_KERNEL_DIR}/src/fpdc
			${TOP_DIR}/abl/libc_sec/include
			${DRIVER_KERNEL_DIR}/inc
			${DRIVER_KERNEL_DIR}/inc/ascend_platform
			${DRIVER_KERNEL_DIR}/src/dms
			${DRIVER_KERNEL_DIR}/src/dms/include
			${DRIVER_KERNEL_DIR}/src/common
			${DRIVER_KERNEL_DIR}/src/ipcdrv
			${DRIVER_KERNEL_DIR}/src/ipcdrv/mdc
			${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_inc
			${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_drv/ts_drv_common
			${DRIVER_KERNEL_DIR}/src/tsdrv/mdc/ts_platform/ts_platform_device/ascend610
			${DRIVER_CHIP_KERNEL_DIR}/src/mttcan
			${TOP_DIR}/inc/driver
			${DRIVER_KERNEL_DIR}/src/dms/aos_core
			${TOP_DIR}/drivers/firmware/hiss/inc
			${DRIVER_KERNEL_DIR}/src/dms/xpc
			${TOP_DIR}/abl/bbox/inc/bbox/device/
		)

		add_custom_command(
			OUTPUT ${DEVICE_LOCAL_MODULE}.ko
			COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_devmng_obj> -o ${DEVICE_LOCAL_MODULE}.ko
			DEPENDS drv_devmng_obj
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND_EXPAND_LISTS
		)

		add_custom_target(drv_devmng ALL DEPENDS drv_mttcan drv_davinci_intf ascend_dms_smf ascend_urd ascend_dms_dtm drv_devmng_obj ${DEVICE_LOCAL_MODULE}.ko)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
					DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)

	else ()
	#linux build
	if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		execute_process(
			COMMAND bash -c "cp ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/unreleased/debug/devdrv_info_debug.* ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/drv_devmng/drv_devmng_device/ascend310/"
			COMMAND bash -c "cp ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/unreleased/debug/devdrv_info_debug.* ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/drv_devmng/drv_devmng_device/ascend910/devdrv_dsmi/"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	endif() # end Debug

	if( ${PRODUCT} STREQUAL ascend310)
		set(drv_devmng_depends tee_drv drv_icm drv_pcie drv_dfm mntn_bbox drv_davinci_intf ascend_urd ascend_dms_dtm ascend_dms_smf)
	elseif (${PRODUCT} STREQUAL ascend910)
		set(drv_devmng_depends drv_nor_flash tee_drv drv_icm drv_pcie drv_dfm mntn_bbox drv_davinci_intf ascend_urd ascend_dms_dtm ascend_dms_smf drv_user_cfg ascend_virtmng_dev drv_cpld)
	elseif (${PRODUCT} STREQUAL ascend310p)
        set(drv_devmng_depends ascend_virtmng_dev drv_user_cfg drv_icm drv_pcie drv_dfm mntn_bbox drv_davinci_intf ascend_dms_smf ascend_urd ascend_dms_dtm hsm_firmware_update hsm_flash hsm_heartbeat)
	elseif (${PRODUCT} STREQUAL ascend910B)
		set(drv_devmng_depends drv_icm drv_user_cfg drv_pcie drv_dfm mntn_bbox drv_davinci_intf ascend_urd ascend_dms_dtm drv_cpld ascend_virtmng_dev ascend_dms_smf)
	elseif (${PRODUCT} STREQUAL ascend310Brc OR ${PRODUCT} STREQUAL helper310p)
		set(drv_devmng_depends drv_icm drv_user_cfg drv_dfm mntn_bbox drv_davinci_intf ascend_urd ascend_dms_dtm ascend_dms_smf)
	else ()
		set(drv_devmng_depends drv_icm drv_user_cfg drv_pcie drv_dfm mntn_bbox drv_davinci_intf ascend_urd ascend_dms_dtm ascend_dms_smf)
	endif ()

	list(APPEND MAKE_ARGS
		KBUILD_MODPOST_WARN=1
	)

	add_device_ko(LOCAL_MODULE drv_devmng
		KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
		MAKE_ARGS ${MAKE_ARGS}
		TARGETE_DPENDS "${drv_devmng_depends}")
	endif() # end BUILD_MOD
endif()

if (${PRODUCT_SIDE} STREQUAL host)
	set(product_list "ascend310p" "ascend910" "ascend910B" "ascend920esl")
	if (${PRODUCT} IN_LIST product_list)
		set(drv_devmng_host_depends drv_pcie_host  ascend_urd ascend_dms_dtm drv_seclib_host drv_virtmng_host drv_vpc_host drv_davinci_intf_host drv_vascend_stub ascend_uda ascend_soc_resmng ascend_dms_smf)
		set(drv_vdevmng_depends drv_pcie_host ascend_dms_smf ascend_urd ascend_dms_dtm drv_seclib_host drv_virtmng_host drv_vpc_host drv_davinci_intf_host drv_vpcie drv_vascend_stub drv_vpc_agent ascend_uda ascend_soc_resmng)
	else ()
		set(drv_devmng_host_depends drv_pcie_host drv_seclib_host ascend_dms_smf ascend_urd ascend_dms_dtm drv_davinci_intf_host ascend_uda ascend_soc_resmng)
	endif()

	list(APPEND MAKE_ARGS
		KBUILD_MODPOST_WARN=1
	)

	add_host_ko(LOCAL_MODULE drv_devmng_host
		KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
		MAKE_ARGS ${MAKE_ARGS}
		TARGETE_DPENDS "${drv_devmng_host_depends}")

		add_host_ko(LOCAL_MODULE drv_vdevmng
		KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
		MAKE_ARGS ${MAKE_ARGS}
		TARGETE_DPENDS "${drv_vdevmng_depends}")
endif()

add_subdirectory(dtm)
add_subdirectory(mng)
add_subdirectory(smf)

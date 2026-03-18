if(AOSCORE_LLVM_MUSL) # for aos-core
	add_library(drv_memory_obj OBJECT)
	set(DEVICE_LOCAL_MODULE drv_memory)
	set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
	set(DRV_MEMORY_DEPENDS ipc_drv drv_devmng drv_fpdc)

	target_sources(drv_memory_obj PRIVATE
		${DRIVER_KERNEL_DIR}/src/drv_memory/memory_init.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_node.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_report.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_ipc.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_610/safety_ras_init_610.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_610/safety_ras_ipc_610.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_610/safety_ras_subctrl_610.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_610/safety_ras_dha_610.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_610/safety_ras_dha_dfx_610.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_610/safety_ras_dha_report_610.c
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras/safety_ras_610/safety_ras_common_610.c
	)

	target_include_directories(drv_memory_obj PRIVATE
		${TOP_DIR}/inc/driver
		${TOP_DIR}/abl/ascend_hal/user_space/src/dms
		${DRIVER_KERNEL_DIR}/inc
		${DRIVER_KERNEL_DIR}/src/drv_memory
		${DRIVER_KERNEL_DIR}/src/drv_memory/safety_ras
		${DRIVER_KERNEL_DIR}/src/fpdc
		${DRIVER_KERNEL_DIR}/src/fpdc/receiver
		${DRIVER_KERNEL_DIR}/src/dms/include
		${DRIVER_KERNEL_DIR}/src/dms/config
		${DRIVER_KERNEL_DIR}/src/dms/smf/sensor
		${DRIVER_KERNEL_DIR}/src/dms/smf/sensor/config
		${DRIVER_KERNEL_DIR}/src/dms/smf/event
		${DRIVER_KERNEL_DIR}/src/dms/dfm_template
		${DRIVER_KERNEL_DIR}/src/drv_devmng/drv_devmng_inc
		${DRIVER_KERNEL_DIR}/src/dbl/dev_urd
	)

	target_compile_definitions(drv_memory_obj PRIVATE AOS_LLVM_BUILD)
	target_compile_definitions(drv_memory_obj PRIVATE CFG_FEATURE_MORE_PID_PRIORITY)
	target_compile_definitions(drv_memory_obj PRIVATE CFG_SOC_PLATFORM_MDC_V51)
	target_compile_definitions(drv_memory_obj PRIVATE CFG_MEMORY_SAFETY_RAS)

	target_compile_options(drv_memory_obj PRIVATE
		-Wall
		-Werror
		-Wfloat-equal
		-fstack-protector-all
	)

	target_link_libraries(drv_memory_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

	add_custom_command(
		OUTPUT ${DEVICE_LOCAL_MODULE}.ko
		COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_memory_obj> -o ${DEVICE_LOCAL_MODULE}.ko
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND_EXPAND_LISTS
	)

	add_custom_target(${DEVICE_LOCAL_MODULE} ALL DEPENDS drv_memory_obj ${DEVICE_LOCAL_MODULE}.ko)

	add_dependencies(${DEVICE_LOCAL_MODULE} ${DRV_MEMORY_DEPENDS})

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
		DESTINATION ${TARGET_KO_DIRECTORY} OPTIONAL)
else() # for aos-linux
	list(APPEND MAKE_ARGS
		KBUILD_MODPOST_WARN=1
	)

	if( ${PRODUCT} STREQUAL ascend310)
		set(drv_memory_depends drv_devmng)
	elseif( ${PRODUCT} STREQUAL ascend310rc)
		set(drv_memory_depends drv_devmng)
	elseif( ${PRODUCT} STREQUAL ascend310p)
		set(drv_memory_depends drv_devmng drv_fpdc drv_icm drv_nor_flash hsm_firmware_update hsm_flash hsm_heartbeat)
	elseif( ${PRODUCT} STREQUAL ascend610)
		set(drv_memory_depends ipc_drv drv_devmng drv_fpdc)
	elseif( ${PRODUCT} STREQUAL bs9sx1a)
		set(drv_memory_depends ipc_drv drv_devmng drv_fpdc)
	elseif( ${PRODUCT} STREQUAL ascend610Lite)
		set(drv_memory_depends ipc_drv drv_devmng drv_fpdc)
	elseif( ${PRODUCT} STREQUAL ascend610Liteesl)
		set(drv_memory_depends ipc_drv drv_devmng drv_fpdc)
	elseif ( ${PRODUCT} STREQUAL ascend910B)
		set(drv_memory_depends drv_devmng drv_fpdc)
	elseif ( ${PRODUCT} STREQUAL ascend310Besl)
		set(drv_memory_depends ipc_drv drv_devmng)
	else()
		set(drv_memory_depends
			drv_devmng
			drv_fpdc
			drv_icm
			drv_nor_flash
		)
	endif()

	add_device_ko(LOCAL_MODULE drv_memory
		KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
		MAKE_ARGS ${MAKE_ARGS}
		TARGETE_DPENDS "${drv_memory_depends}"
	)
endif()
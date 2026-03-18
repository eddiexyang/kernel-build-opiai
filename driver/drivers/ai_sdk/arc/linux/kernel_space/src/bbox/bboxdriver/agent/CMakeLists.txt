set(product_list "ascend610" "bs9sx1a")
if (${PRODUCT} IN_LIST product_list)
#aos llvm build
if (AOSCORE_LLVM_MUSL)
    add_library(mntn_bbox_agent_obj OBJECT)
    set(DEVICE_LOCAL_MODULE mntn_bbox_agent)
    set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)

    target_sources(mntn_bbox_agent_obj PRIVATE
        bbox_agent.c
        bbox_agent_pub.c
        bbox_agent_os.c
        ../common/bbox_xpc_common.c
        ../common/bbox_common.c
        ../config/bbox_config_dts.c
        ../config/bbox_config.c
        ../register/bbox_sctrl_register.c
    )
    target_compile_definitions(mntn_bbox_agent_obj PRIVATE AOS_LLVM_BUILD)
    target_link_libraries(mntn_bbox_agent_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)
    target_include_directories(mntn_bbox_agent_obj PRIVATE
                ${TOP_DIR}/abl/bbox/inc/bbox
                ${TOP_DIR}/abl/bbox/inc/bbox/device
                ${TOP_DIR}/inc/toolchain/bbox
                ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver
                ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver/agent
                ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver/common
                ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver/config
                ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver/register
                ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/bbox/bboxdriver/os_adapter
    )

    add_custom_command(
        OUTPUT ${DEVICE_LOCAL_MODULE}.ko
        COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:mntn_bbox_agent_obj> -o ${DEVICE_LOCAL_MODULE}.ko
        DEPENDS mntn_bbox_agent_obj
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )
    add_custom_target(mntn_bbox_agent ALL DEPENDS mntn_bbox_agent_obj ${DEVICE_LOCAL_MODULE}.ko)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
                DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)
endif ()
endif ()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT}/src/sdk_hilink_dfx.c ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT}/src/sdk_hilink_dfx.h
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT}/src/
    COMMAND cp -lrf ${TOP_DIR}/drivers/ai_sdk/driver/linux/kernel_space/src/unreleased/serdes_h25/sdk_hilink_dfx.c ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT}/src/sdk_hilink_dfx.c
    COMMAND cp -lrf ${TOP_DIR}/drivers/ai_sdk/driver/linux/kernel_space/src/unreleased/serdes_h25/sdk_hilink_dfx.h ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT}/src/sdk_hilink_dfx.h
)
add_custom_target(drv_serdes_dfx ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT}/src/sdk_hilink_dfx.c ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT}/src/sdk_hilink_dfx.h)
endif() # end Debug

add_device_ko(LOCAL_MODULE drv_serdes
            KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
            MAKE_ARGS ${MAKE_ARGS})

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
add_dependencies(drv_serdes drv_serdes_dfx)
endif() # end Debug
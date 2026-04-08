cmake_minimum_required(VERSION 3.16.3)
project(dtbtool)

# 编译dtbTool
set(BIN_DTBTOOL ${CMAKE_CURRENT_BINARY_DIR}/dtbTool CACHE STRING "dtbTool")

set(SRC_DTBTOOL
    ${CMAKE_CURRENT_SOURCE_DIR}/dtbtool.c
)

add_custom_command(OUTPUT ${BIN_DTBTOOL}
    COMMAND gcc ${SRC_DTBTOOL} -o ${BIN_DTBTOOL}
    -s -O1 -Wall -Wextra -Wfloat-equal -fno-common -std=gnu90
    DEPENDS ${SRC_DTBTOOL}
)

add_custom_target(private_target_dtbtool ALL DEPENDS ${BIN_DTBTOOL})

install(FILES ${BIN_DTBTOOL}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    DESTINATION bin
    OPTIONAL
)



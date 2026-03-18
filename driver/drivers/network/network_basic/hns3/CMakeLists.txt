if("${PRODUCT}" STREQUAL "ascend910B" OR "${PRODUCT}" STREQUAL "ascend920emu" OR "${PRODUCT}" STREQUAL "ascend920esl")
add_subdirectory(network_custom)
endif()
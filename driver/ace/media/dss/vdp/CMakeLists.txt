if (${PRODUCT_SIDE} STREQUAL device)

add_subdirectory(drv)
add_subdirectory(api)
add_subdirectory(stub)
endif()


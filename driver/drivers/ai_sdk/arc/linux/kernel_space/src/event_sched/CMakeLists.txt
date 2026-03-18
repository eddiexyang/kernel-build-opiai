if (${PRODUCT_SIDE} STREQUAL device)
add_subdirectory(device)
endif()

if (${PRODUCT_SIDE} STREQUAL host)
add_subdirectory(host)
endif()
if (${PRODUCT_SIDE} STREQUAL host)
else ()
    add_subdirectory(agent)
endif()

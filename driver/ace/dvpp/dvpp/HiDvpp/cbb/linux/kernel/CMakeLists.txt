if(AOSCORE_LLVM_MUSL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/aos-core.cmake)
else()
    set(drv_osal_depends svm)
    add_device_ko(LOCAL_MODULE drv_osal
                            TARGETE_DPENDS "${drv_osal_depends}"
                            KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})

    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT}/src/drv_osal.o
                DESTINATION ${CMAKE_INSTALL_PREFIX}/obj OPTIONAL)
endif()
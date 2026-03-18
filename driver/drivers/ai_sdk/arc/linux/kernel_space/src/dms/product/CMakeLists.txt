if (${PRODUCT_SIDE} STREQUAL host)

    target_include_directories(drv_devmng_obj PRIVATE
        ${TOP_DIR}/inc/driver/
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/inc
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/sensor
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/smf/sensor
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/event
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/smf/event
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/config
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/include
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/core
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/product
    )

    target_compile_definitions(drv_devmng_obj PRIVATE
        CFG_FEATURE_DMS_PRODUCT_HOST
        $<$<STREQUAL:${PRODUCT},ascend910>:CFG_SOC_PLATFORM_CLOUD>
    )

    target_sources(drv_devmng_obj PRIVATE
        dms_product_host.c
        dms_product.c
    )

else()

    target_include_directories(drv_devmng_obj PRIVATE
        ${TOP_DIR}/inc/driver/
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/inc
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/sensor
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/smf/sensor
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/event
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/smf/event
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/config
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/include
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/core
        ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/src/dms/product
    )

    target_sources(drv_devmng_obj PRIVATE
        dms_product.c
    )

endif ()
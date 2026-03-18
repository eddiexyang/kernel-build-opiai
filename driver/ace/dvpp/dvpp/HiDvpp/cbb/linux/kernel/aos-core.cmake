add_library(drv_osal_obj OBJECT)
set(DEVICE_LOCAL_MODULE drv_osal)
set(TARGET_KO_DIRECTORY ${CMAKE_INSTALL_PREFIX}/obj)
target_sources(drv_osal_obj PRIVATE
        osal.c 
        osal_addr.c 
        osal_atomic.c 
        osal_barrier.c
        osal_debug.c 
        osal_cache.c 
        osal_device.c 
        osal_fileops.c
        osal_interrupt.c
        osal_math.c
        osal_mutex.c
        osal_platform.c
        osal_proc.c
        osal_schedule.c
        osal_semaphore.c
        osal_spinlock.c
        osal_string.c
        osal_task.c
        osal_timer.c
        osal_vmalloc.c
        osal_wait.c
        osal_workqueue.c
        osal_iommu.c
        osal_of.c
        osal_notifier.c
        osal_property.c
        osal_acpi.c
        osal_bootdot.c
        himedia/base.c
        himedia/himedia.c
        
)
target_compile_definitions(drv_osal_obj PRIVATE AOS_LLVM_BUILD ENABLE_ADVSMARTP BUILD_DRV_OSAL __KERNEL__)
target_compile_definitions(drv_osal_obj PRIVATE CFG_FEATURE_MORE_PID_PRIORITY)
target_link_libraries(drv_osal_obj PRIVATE $<BUILD_INTERFACE:utils_intf_pub>)

set(ARCDIR hidvpp)
set(BASE_PATH ${TOP_DIR}/ace/media/mediabase/src)
set(BASE_INC_PATH ${TOP_DIR}/ace/media/mediabase/inc)
target_include_directories(drv_osal_obj PRIVATE
    ${BASE_INC_PATH}/external/mediabase
    ${BASE_INC_PATH}/mediabase/include 
    ${BASE_INC_PATH}/mediabase/include/adapt 
    ${BASE_INC_PATH}/mediabase/osal
    ${BASE_PATH}/osal/linux/kernel/himedia 
    ${BASE_PATH}/osal/linux/kernel 
    ${BASE_PATH}/base/arch/${ARCDIR}/include 
    ${TOP_DIR}/abl/libc_sec/include 
    ${TOP_DIR}/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-gnu-5.4/aarch64-linux-gnu/include 
    ${TOP_DIR}/kernel/linux-source/arch/arm/include 
    ${TOP_DIR}/kernel/linux-source/arch/arm64/include 
    ${TOP_DIR}/ace/media/inc/media/camera/camerav2 
    ${TOP_DIR}/ace/media/inc/media/camera/camerav2/adapt
    ${TOP_DIR}/abl/slog/inc/toolchain
)


add_custom_command(
    OUTPUT ${DEVICE_LOCAL_MODULE}.ko
    COMMAND ${CMAKE_LINKER} -r $<TARGET_OBJECTS:drv_osal_obj> -o ${DEVICE_LOCAL_MODULE}.ko
    DEPENDS drv_osal_obj
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND_EXPAND_LISTS
)
add_custom_target(drv_osal ALL DEPENDS drv_osal_obj ${DEVICE_LOCAL_MODULE}.ko)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_LOCAL_MODULE}.ko
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib OPTIONAL)

set(MEDIABASE_PATH  ${TOP_DIR}/ace/media/mediabase)
set(OSAL_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Ascend310B 和 TSCH 合设, 补充ts接口
add_library(
    osal_liteos
    OBJECT
    osal_addr.c
    osal_atomic.c
    osal_barrier.c
    osal_cache.c
    osal_debug.c
    osal_device.c
    osal_fileops.c
    osal_interrupt.c
    osal_math.c
    osal_mutex.c
    osal_proc.c
    osal_schedule.c
    osal_semaphore.c
    osal_spinlock.c
    osal_string.c
    osal_task.c
    osal_timer.c
    osal_vmalloc.c
    osal_wait.c
    #osal_workqueue.c
)

target_include_directories(osal_liteos PRIVATE
    ${MEDIABASE_PATH}/inc/mediabase/include
    ${MEDIABASE_PATH}/inc/mediabase/include
    ${MEDIABASE_PATH}/inc/mediabase/osal
    ${TOP_DIR}/ace/dvpp/inc/dvpp
    ${TOP_DIR}/inc/external/dvpp
    ${TOP_DIR}/ace/dvpp/inc/external/dvpp
    ${MEDIABASE_PATH}/src/base/arch/include
    ${MEDIABASE_PATH}/src/base/arch/hidvpp/include
)

target_link_libraries(osal_liteos PRIVATE
    $<BUILD_INTERFACE:dvpp_liteos_pub>
)

target_compile_definitions(osal_liteos PRIVATE
    USE_DVPP_FWK
)

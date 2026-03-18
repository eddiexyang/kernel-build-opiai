cmake_minimum_required(VERSION 3.14.1)

project(hdmi_mpi)

set(HDMI_CHIP archv200)

set(MPP_DIR ${TOP_DIR}/ace/media/dss/dss_mpp)
set(MPP_COMMON_DIR ${MPP_DIR}/common)
set(MPP_BASE_DIR ${MPP_COMMON_DIR}/base)
set(HDMI_DIR ${MPP_DIR}/hdmi)
set(HDMI_MKP_DIR ${HDMI_DIR}/mkp)
set(HDMI_MPI_DIR ${HDMI_DIR}/mpi)

#add for hdmi in master
set(VDP_DIR ${TOP_DIR}/ace/media/dss/vdp)
set(MEDIABASE_DIR  ${TOP_DIR}/ace/media/mediabase/inc/mediabase)
set(MEDIABASE_BASE_DIR ${MEDIABASE_DIR}/base)
set(HDMI_MASTER_DIR ${VDP_DIR}/hdmi)
set(HDMI_MASTER_MKP_DIR ${HDMI_MASTER_DIR}/mkp)
set(HDMI_MASTER_MPI_DIR ${HDMI_MASTER_DIR}/mpi)

# specify include dir
include_directories(
    ${MPP_DIR}/include
    ${MPP_BASE_DIR}/ext_inc
    ${MPP_BASE_DIR}/include
    ${MPP_BASE_DIR}/arch/${HDMI_CHIP}/include
    ${HDMI_DIR}/include
    ${HDMI_MKP_DIR}

    ${VDP_DIR}/dfx
    ${MEDIABASE_BASE_DIR}/ext_inc
    ${MEDIABASE_BASE_DIR}/include
    ${TOP_DIR}/ace/media/mediabase/src/base/arch/arch11/include

    ${HDMI_MASTER_DIR}/include
    ${HDMI_MASTER_MKP_DIR}

    ${HDMI_MKP_DIR}/dfx
)

# COMMON dir
set(MB_TOP_INC_DIR ${TOP_DIR}/ace/media/mediabase/inc)
set(MB_TOP_SRC_DIR ${TOP_DIR}/ace/media/mediabase/src)
set(MB_INC_EXT_DIR ${MB_TOP_INC_DIR}/external/mediabase)
set(MB_INC_COMMON_DIR ${MB_TOP_INC_DIR}/mediabase)
set(MB_SRC_BASE_ARCH_DIR ${MB_TOP_SRC_DIR}/base/arch/arch11/include)

# H8 adapt dir
set(H8_ADAPT_TOP_DIR ${MB_INC_COMMON_DIR}/h8_include)

# specify include dir
include_directories(
    ${TOP_DIR}/drivers/ai_sdk/arc/linux/kernel_space/inc/
    ${TOP_DIR}/ace/media/mediabase/inc/mediabase/sys/ext_inc/
    ${TOP_DIR}/ace/media/dss/vdp/inc/vdp/

    ${MB_INC_EXT_DIR}
    ${MB_INC_COMMON_DIR}/include
    ${MB_INC_COMMON_DIR}/include/adapt
    ${MB_INC_COMMON_DIR}/base/ext_inc
    ${MB_INC_COMMON_DIR}/base/adapt
    ${MB_INC_COMMON_DIR}/base/inner
    ${MB_SRC_BASE_ARCH_DIR}
    ${H8_ADAPT_TOP_DIR}/include
    ${H8_ADAPT_TOP_DIR}/base/ext_inc
    ${H8_ADAPT_TOP_DIR}/base/arch/${HDMI_CHIP}/include
    ${H8_ADAPT_TOP_DIR}/sys/include
    ${HDMI_DIR}/include
    ${HDMI_MKP_DIR}
    ${TOP_DIR}/abl/slog/inc/toolchain/
    ${TOP_DIR}/libc_sec/include/

    ${MB_INC_COMMON_DIR}/osal
    ${H8_ADAPT_TOP_DIR}/osal/include
    ${H8_ADAPT_TOP_DIR}/osal/linux
    ${H8_ADAPT_TOP_DIR}/base/init/include
    ${H8_ADAPT_TOP_DIR}/base/mkp/include
    ${H8_ADAPT_TOP_DIR}/base/arch/include
    ${H8_ADAPT_TOP_DIR}/sys/ext_inc
    ${H8_ADAPT_TOP_DIR}/sys/init/include
    ${H8_ADAPT_TOP_DIR}/sys/mkp/include
    ${H8_ADAPT_TOP_DIR}/sys/arch/include
    ${H8_ADAPT_TOP_DIR}/sys/arch/archv200/include
    ${H8_ADAPT_TOP_DIR}/base/arch/archv200/include/
    ${MB_TOP_SRC_DIR}/sys/include/adapt
    ${TOP_DIR}/ace/media/dss/inc/vdp
    ${TOP_DIR}/ace/media/dss/vdp/hdmi/include/


)

add_definitions(-DTHREADED)
add_definitions(-DHDMI_SUPPORT_DUAL_CHANNEL)

#add ot lib

add_library(ot_hdmi_mpi SHARED
    mpi_hdmi_com.c
    ot_mpi_hdmi.c
)

target_compile_options(ot_hdmi_mpi PRIVATE
    -Wall -fPIC -c -O2 -fstack-protector-all
    -lpthread -Wextra -Wfloat-equal -fno-common
)

target_link_libraries(ot_hdmi_mpi PRIVATE
    $<BUILD_INTERFACE:intf_pub>
    $<BUILD_INTERFACE:slog_headers>
    pthread
    c_sec
    slog
    ascend_hal -ldl
)

set(INSTALL_BASE_DIR "")
set(INSTALL_LIBRARY_DIR lib)
include(CMakePackageConfigHelpers)

install(TARGETS ot_hdmi_mpi OPTIONAL
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
)

#add mid lib

add_library(mid_hdmi_mpi SHARED
    mpi_hdmi_com.c
    ot_mpi_hdmi.c
    mid_mpi_hdmi.c
)

target_compile_options(mid_hdmi_mpi PRIVATE
    -Wall -fPIC -c -O2 -fstack-protector-all
    -lpthread -Wextra -Wfloat-equal -fno-common
)

target_link_libraries(mid_hdmi_mpi PRIVATE
    $<BUILD_INTERFACE:intf_pub>
    $<BUILD_INTERFACE:slog_headers>
    pthread
    c_sec
    slog
    ascend_hal -ldl
)

set(INSTALL_BASE_DIR "")
set(INSTALL_LIBRARY_DIR lib)
include(CMakePackageConfigHelpers)

install(TARGETS mid_hdmi_mpi OPTIONAL
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
)

#add acl lib

add_library(acl_hdmi_mpi SHARED
    hi_mpi_hdmi.c
)

target_compile_options(acl_hdmi_mpi PRIVATE
    -Wall -fPIC -c -O2 -fstack-protector-all
    -lpthread -Wextra -Wfloat-equal -fno-common
)

target_link_libraries(acl_hdmi_mpi PRIVATE
    $<BUILD_INTERFACE:intf_pub>
    $<BUILD_INTERFACE:slog_headers>
    pthread
    mid_hdmi_mpi
    c_sec
    slog
    ascend_hal -ldl
)

set(INSTALL_BASE_DIR "")
set(INSTALL_LIBRARY_DIR lib)
include(CMakePackageConfigHelpers)

install(TARGETS acl_hdmi_mpi OPTIONAL
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
)
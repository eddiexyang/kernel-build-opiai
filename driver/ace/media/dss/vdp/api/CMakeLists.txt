cmake_minimum_required(VERSION 3.14.1)

project(tde_api)

set(MPP_DIR ${TOP_DIR}/ace/media/dss/vdp)
set(INC_DIR ${TOP_DIR}/ace/media/dss/inc)
set(TDE_DIR ${MPP_DIR}/tde)
set(TDE_DRV_DIR ${TDE_DIR}/drv)
set(TDE_API_DIR ${TDE_DIR}/api)

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
    ${MB_INC_EXT_DIR}
    ${MB_INC_COMMON_DIR}/include
    ${MB_INC_COMMON_DIR}/include/adapt
    ${MB_INC_COMMON_DIR}/base/ext_inc
    ${MB_INC_COMMON_DIR}/base/adapt
    ${MB_INC_COMMON_DIR}/base/inner
    ${MB_INC_COMMON_DIR}/osal
    ${MB_INC_COMMON_DIR}/h8_include/osal/include
    ${MB_SRC_BASE_ARCH_DIR}
    ${H8_ADAPT_TOP_DIR}/include
    ${H8_ADAPT_TOP_DIR}/base/ext_inc
    ${H8_ADAPT_TOP_DIR}/base/arch/archv200/include
    ${TOP_DIR}/abl/slog/inc/toolchain
    ${MPP_DIR}
    ${MPP_DIR}/dfx
    ${INC_DIR}/vdp
    ${TDE_DIR}/include
    ${TDE_DRV_DIR}/include
    ${TDE_DRV_DIR}/tde/include
    ${TDE_API_DIR}/tde/include
)

add_definitions(-DTHREADED)
add_definitions(-DENV_SMMU_SUPPORT)

##########################################################
add_library(mid_tde_mpi SHARED
    tde/src/mid_mpi_tde.c
    tde/src/mpi_tde_drv_para.c
)

target_compile_options(mid_tde_mpi PRIVATE
    -Wall -fPIC -c -O2 -fstack-protector-all
    -lpthread -Wextra -Wfloat-equal -fno-common
)

target_link_libraries(mid_tde_mpi PRIVATE
    $<BUILD_INTERFACE:intf_pub>
    pthread
    c_sec
    slog
)

set(INSTALL_BASE_DIR "")
set(INSTALL_LIBRARY_DIR lib)
include(CMakePackageConfigHelpers)

install(TARGETS mid_tde_mpi OPTIONAL
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
)
##########################################################
add_library(acl_tde_mpi SHARED
    tde/src/hi_mpi_tde.c
)

target_compile_options(acl_tde_mpi PRIVATE
    -Wall -fPIC -c -O2 -fstack-protector-all
    -lpthread -Wextra -Wfloat-equal -fno-common
)

target_link_libraries(acl_tde_mpi PRIVATE
    $<BUILD_INTERFACE:intf_pub>
    pthread
    c_sec
    mid_tde_mpi
)

set(INSTALL_BASE_DIR "")
set(INSTALL_LIBRARY_DIR lib)
include(CMakePackageConfigHelpers)

install(TARGETS acl_tde_mpi OPTIONAL
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
)
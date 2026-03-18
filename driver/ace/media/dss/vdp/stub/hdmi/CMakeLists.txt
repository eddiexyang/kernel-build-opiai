set(LOCAL_PATH ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT HI_PYTHON)
    set(HI_PYTHON python3)
endif()
set(inc_path ${TOP_DIR}/ace/media/dss/inc/vdp)
set(out_path ${CMAKE_CURRENT_BINARY_DIR}/hdmi/lib64/stub)
set(stub_path ${TOP_DIR}/ace/media/dss/vdp/stub)
set(COMMON_SRC_DIR ${TOP_DIR}/ace/media/mediabase/src)
set(COMMON_INC_DIR ${TOP_DIR}/ace/media/mediabase/inc)

set(MPP_DIR ${TOP_DIR}/ace/media/dss/vdp)
set(INC_DIR ${TOP_DIR}/ace/media/dss/inc)
set(TDE_DIR ${MPP_DIR}/hdmi)
set(TDE_API_DIR ${TDE_DIR}/api)

add_custom_command(
    OUTPUT ${out_path}/hi_mpi_hdmi_stub.c
    COMMAND mkdir -p ${out_path}
    COMMAND echo "${out_path}/hi_mpi_hdmi_stub.c"
    COMMAND ${HI_PYTHON} ${stub_path}/gen_stubapi.py ${inc_path} ${out_path}/hi_mpi_hdmi_stub.c hdmi
    DEPENDS ${inc_path} ${stub_path}/gen_stubapi.py
)


add_library(stub_acl_hdmi_mpi SHARED
    ${out_path}/hi_mpi_hdmi_stub.c
)


set_target_properties(stub_acl_hdmi_mpi
    PROPERTIES
    OUTPUT_NAME acl_hdmi_mpi
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/stub
)

target_compile_options(stub_acl_hdmi_mpi PRIVATE
    -O2
    -g
    -fpie
    -fstack-protector-all
    -fpic
)

target_compile_definitions(stub_acl_hdmi_mpi PRIVATE
)

target_link_options(stub_acl_hdmi_mpi PRIVATE
    -Wl,-z,relro
    -Wl,-z,now
    -Wl,-z,noexecstack
    -fvisibility=hidden
)

include_directories(
    ${COMMON_INC_DIR}/external/mediabase/
    ${INC_DIR}/vdp
)


target_link_libraries(stub_acl_hdmi_mpi PRIVATE
    $<BUILD_INTERFACE:intf_pub>
)

INSTALL(TARGETS stub_acl_hdmi_mpi
    LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}/${CMAKE_SYSTEM_PROCESSOR}/stub
    OPTIONAL
)

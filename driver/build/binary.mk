

ifneq ($(LLT_TEST),)
include $(BUILD_SYSTEM)/llt_base_rules.mk
else
include $(BUILD_SYSTEM)/base_rules.mk
endif

ifneq ($(LOCAL_SHARED_LIBRARY),)
$(error you should use LOCAL_SHARED_LIBRARIES instead of LOCAL_SHARED_LIBRARY in $(LOCAL_MODULE))
endif

ifneq ($(LOCAL_STATIC_LIBRARY),)
$(error you should use LOCAL_STATIC_LIBRARIES instead of LOCAL_STATIC_LIBRARY in $(LOCAL_MODULE))
endif

ifneq ($(LOCAL_WHOLE_STATIC_LIBRARY),)
$(error you should use LOCAL_WHOLE_STATIC_LIBRARIES instead of LOCAL_WHOLE_STATIC_LIBRARY in $(LOCAL_MODULE))
endif

ifeq ($(strip $(product)), onetrack)
component_name := $(shell cd $(TOPDIR)build/product/onetrack/product_modules && grep -sR "$(LOCAL_MODULE)" | awk -F '.' '{print $1}')
ifneq ($(component_name),)
include $(strip $(wildcard $(TOPDIR)build/product/onetrack/define/common_define.mk))
include $(strip $(wildcard $(TOPDIR)build/product/onetrack/define/$(component_name)_define.mk))
endif
endif


my_cc := $(LOCAL_CC)
my_toolchain := $(LOCAL_TOOLCHAIN_PREFIX)
my_cxx := $(LOCAL_CXX)
my_ar := $(LOCAL_AR)
my_strip := $(LOCAL_STRIP)
my_protoc := $(LOCAL_PROTOC)
my_src_files := $(LOCAL_SRC_FILES)
my_cflags := $(LOCAL_CFLAGS)
my_cppflags := $(LOCAL_CPPFLAGS)
my_asflags := $(LOCAL_ASFLAGS)
my_protoflags :=
my_c_includes := $(LOCAL_C_INCLUDES)
my_static_library := $(LOCAL_STATIC_LIBRARIES)
my_whole_static_library := $(LOCAL_WHOLE_STATIC_LIBRARIES)
my_shared_library := $(LOCAL_SHARED_LIBRARIES)
my_third_party_library := $(LOCAL_THIRD_PARTY_LIBRARY)
my_ldflags := $(LOCAL_LDFLAGS)
my_ldlibs := $(LOCAL_LDLIBS)
my_ld_dirs := $(LOCAL_LD_DIRS)
my_global_cflags := $($(my_prefix)GLOBAL_CFLAGS)
my_global_cppflags := $($(my_prefix)GLOBAL_CPPFLAGS)
my_global_ldflags := $($(my_prefix)GLOBAL_LDFLAGS)
my_global_arflags := $($(my_prefix)GLOBAL_ARFLAGS)
my_global_ld_dirs := $($(my_prefix)GLOBAL_LD_DIRS)
my_additional_dependencies := $(LOCAL_ADDITIONAL_DEPENDENCIES)
my_export_c_include_dirs := $(LOCAL_EXPORT_C_INCLUDE_DIRS)
my_export_c_include_deps := $(LOCAL_EXPORT_C_INCLUDE_DEPS)
my_header_library := $(LOCAL_HEADER_LIBRARIES)
my_import_c_include_lib := $(LOCAL_IMPORT_C_INCLUDE_LIB)

ifneq ($(my_toolchain),)

ifeq ($(strip $(my_cc)),)
my_cc := $(my_toolchain)gcc
endif

ifeq ($(strip $(my_cxx)),)
my_cxx := $(my_toolchain)g++
endif

ifeq ($(strip $(my_ar)),)
my_ar := $(my_toolchain)ar
endif

ifeq ($(strip $(my_strip)),)
my_strip := $(my_toolchain)strip
endif

else

ifeq ($(strip $(my_cc)),)
my_cc := $($(my_prefix)CC)
endif

ifeq ($(strip $(my_cxx)),)
my_cxx := $($(my_prefix)CXX)
endif

ifeq ($(strip $(my_ar)),)
my_ar := $($(my_prefix)AR)
endif

ifeq ($(strip $(my_strip)),)
my_strip := $($(my_prefix)STRIP)
endif

ifeq ($(strip $(my_protoc)),)
my_protoc := $($(my_prefix)PROTOC)
endif

endif

ifeq ($(strip $(my_cc)),gcc)
my_strip := strip
endif

my_ld_dirs += -L$(intermediates_lib_dir)
my_lib_rpath_dir := $(PWD)/$(intermediates_lib_dir)

ifeq ($(strip $(depend_on_ros)),true)
ros_libs_dir := /opt/x86/ros/kinetic/lib/
my_ld_dirs += -L$(ros_libs_dir)
ros_include_dir := /opt/x86/ros/kinetic/include/
ros_include_dir += /opt/x86/ros/kinetic/include/opencv-3.3.1-dev
my_c_includes += $(ros_include_dir)
endif

ifeq ($(strip $(TARGET_BUILD_TYPE)),debug)
my_cflags += -DCFG_BUILD_DEBUG
my_cppflags += -DCFG_BUILD_DEBUG
else
my_cflags += -DCFG_BUILD_NDEBUG
my_cppflags += -DCFG_BUILD_NDEBUG
endif

ifeq ($(strip $(TARGET_BUILD_TYPE)),release)
my_global_ldflags += -Wl,--build-id=none
endif

$(LOCAL_BUILT_MODULE): PRIVATE_CC := $(my_cc)
$(LOCAL_BUILT_MODULE): PRIVATE_CXX := $(my_cxx)
$(LOCAL_BUILT_MODULE): PRIVATE_AR := $(my_ar)
$(LOCAL_BUILT_MODULE): PRIVATE_CFLAGS := $(my_cflags)
$(LOCAL_BUILT_MODULE): PRIVATE_CPPFLAGS := $(my_cppflags)
$(LOCAL_BUILT_MODULE): PRIVATE_ASFLAGS := $(my_asflags)
$(LOCAL_BUILT_MODULE): PRIVATE_PROTO_INCLUDES :=
$(LOCAL_BUILT_MODULE): PRIVATE_GLOBAL_CFLAGS := $(my_global_cflags)
$(LOCAL_BUILT_MODULE): PRIVATE_GLOBAL_CPPFLAGS := $(my_global_cppflags)
$(LOCAL_BUILT_MODULE): PRIVATE_LDFLAGS := $(my_ldflags)
$(LOCAL_BUILT_MODULE): PRIVATE_LDLIBS := $(my_ldlibs)
$(LOCAL_BUILT_MODULE): PRIVATE_GLOBAL_LDFLAGS := $(my_global_ldflags)
$(LOCAL_BUILT_MODULE): PRIVATE_GLOBAL_ARFLAGS := $(my_global_arflags)
$(LOCAL_BUILT_MODULE): PRIVATE_GLOBAL_LD_DIRS := $(my_global_ld_dirs)
$(LOCAL_BUILT_MODULE): PRIVATE_LIB_RPATH_DIR := $(my_lib_rpath_dir)

#set system shared library
my_system_shared_library :=

#set system static library
my_system_static_library :=

#Strip '/' from the beginning of each src file
my_src_files := $(patsubst /%,%,$(my_src_files))

#compile .proto to .cc and .h
proto_inc_dotdot_sources := $(filter ../%.proto,$(my_c_includes))
ifneq (,$(proto_inc_dotdot_sources))
$(warning you should not define .proto file start with ../ in LOCAL_C_INCLUDES, like ../xxx.proto, not supported!)
endif
proto_inc_normal_sources := $(filter-out ../%,$(filter %.proto,$(my_c_includes)))
#delete all proto files in my_c_includes
my_c_includes := $(filter-out %.proto,$(my_c_includes))

proto_dotdot_sources := $(filter ../%.proto,$(my_src_files))
ifneq (,$(proto_dotdot_sources))
$(warning you should not define .proto file start with ../ in LOCAL_SRC_FILES, like ../xxx.proto, not supported!)
endif
proto_normal_sources := $(filter-out ../%,$(filter %.proto,$(my_src_files)))

proto_sources := $(proto_normal_sources) $(proto_inc_normal_sources)

#define proto source to be compiled to objects
proto_generated_objects :=
proto_generated_headers :=

ifneq ($(proto_sources),)

proto_sources_path := $(addprefix $(LOCAL_PATH)/,$(sort $(dir $(proto_sources))))
#proto output configuration
proto_generated_obj_dir := $(intermediates)/proto
proto_generated_sources_dir := $(intermediates)/proto
protobuf_lib_dir := $(HOST_OUT_ROOT)/protoc_bin
protobuf_lib_include := $(TOPDIR)third_party/protobuf/lib

ifneq (,$(filter protobuf-c, $(LOCAL_PROTOC_TYPE)))
proto_source_suffix := .c
protoc-c_executable := $(HOST_OUT_ROOT)/protoc-c
PRIVATE_PROTOC-C := $(protoc-c_executable)
PROTO_EXECUTABLE := PROTOC-C
my_protoflags += --c_out=$(proto_generated_sources_dir)/$(LOCAL_PATH)
proto_pb_suffix := pb-c
my_c_includes += $(TOPDIR)third_party/protobuf_c/include
else
proto_source_suffix := .cc
PRIVATE_PROTOC := $(protobuf_lib_dir)/protoc
PROTO_EXECUTABLE := PROTOC
proto_pb_suffix := pb
my_protoflags += --cpp_out=$(PROTO_OUT_ROOT)
endif

#add protoc output dir to c includes
my_c_includes += $(intermediates)/proto/$(LOCAL_PATH)
# Auto-export the generated proto source dir.
my_export_c_include_dirs += $(intermediates)/proto/$(LOCAL_PATH)
my_export_c_include_dirs += $(proto_generated_sources_dir)


proto_sources_fullpath := $(addprefix $(LOCAL_PATH)/,$(proto_normal_sources))
proto_normal_src_fullpath := $(addprefix $(LOCAL_PATH)/,$(proto_normal_sources) $(proto_inc_normal_sources))

proto_gen_normal_sources := $(addprefix $(proto_generated_sources_dir)/,\
    $(patsubst %.proto,%.$(proto_pb_suffix)$(proto_source_suffix),$(proto_normal_src_fullpath)))

#proto generated headers and objects
proto_generated_headers := $(patsubst %.$(proto_pb_suffix)$(proto_source_suffix),%.$(proto_pb_suffix).h, \
    $(proto_gen_normal_sources))
proto_generated_objects := $(addprefix $(proto_generated_obj_dir)/,\
    $(patsubst %.proto,%.$(proto_pb_suffix).o,$(proto_sources_fullpath)))

#add .proto generated headers to my_additional_dependencies
my_additional_dependencies += $(proto_generated_headers)

$(proto_generated_headers): $(proto_generated_sources_dir)/%.$(proto_pb_suffix).h: $(proto_generated_sources_dir)/%.$(proto_pb_suffix)$(proto_source_suffix)
	@echo "Updated header file $@"
	$(hide) touch $@

#transform proto file to .cc
$(proto_gen_normal_sources): $(proto_generated_sources_dir)/%.$(proto_pb_suffix)$(proto_source_suffix): %.proto $(PRIVATE_$(PROTO_EXECUTABLE))
ifeq ($(proto_source_suffix),.c)
	+$(transform-proto-to-c)
else
	+$(transform-proto-to-cc)
endif

#compile LOCAL_C_INCLUDES proto .h
ifeq ($(proto_generated_objects),)
$(LOCAL_BUILT_MODULE): $(proto_generated_headers)
endif
#transform all proto generated .cc file to .o
$(LOCAL_BUILT_MODULE): PRIVATE_PROTOFLAGS := $(my_protoflags) $(LOCAL_PROTOFLAGS)
$(LOCAL_BUILT_MODULE): PRIVATE_PROTO_LIB_DIR := $(protobuf_lib_dir)
$(LOCAL_BUILT_MODULE): PRIVATE_MODULE_PATH := $(LOCAL_PATH)
$(proto_generated_objects): $(proto_generated_obj_dir)/%.o:$(proto_generated_sources_dir)/%$(proto_source_suffix) $(proto_generated_headers)
ifeq ($(proto_source_suffix),.c)
	$(transform-c-to-o)
else
	$(transform-cpp-to-o)
endif

endif

# attention! Any change to my_c_includes can take effects before this!
$(LOCAL_BUILT_MODULE): PRIVATE_C_INCLUDES := $(my_c_includes)

#filter .cpp files start with ../
dotdot_sources := $(filter ../%.cpp,$(my_src_files))

#create dependency between dot_cpp_obj and dot_cpp_src
dotdot_objects :=
$(foreach s,$(dotdot_sources),\
  $(eval $(call compile-dotdot-cpp-file,$(s),\
    $(my_additional_dependencies),\
    dotdot_objects)))

#filter .cpp files
cpp_normal_files := $(filter-out ../%,$(filter %.cpp,$(my_src_files)))
cpp_normal_objects := $(addprefix $(intermediates)/,$(cpp_normal_files:.cpp=.o))

ifneq ($(strip $(cpp_normal_objects)),)
$(cpp_normal_objects): $(intermediates)/%.o:$(TOPDIR)$(LOCAL_PATH)/%.cpp $(my_additional_dependencies)
	$(transform-cpp-to-o)
endif

ifneq ($(PCLINT),)
cpp_lint_objects := $(addprefix $(intermediates)/,$(cpp_normal_files:.cpp=.lint))
$(cpp_lint_objects): $(intermediates)/%.lint:$(TOPDIR)$(LOCAL_PATH)/%.cpp $(my_additional_dependencies)
	$(lint-cpp-files)
endif

cpp_objects := $(cpp_normal_objects) $(dotdot_objects)

#filter .c files start with ../
dotdot_sources := $(filter ../%.c, $(my_src_files))
#create dependency between dot_c_obj and dot_c_src
dotdot_objects :=
$(foreach s, $(dotdot_sources),\
  $(eval $(call compile-dotdot-c-file,$(s),\
    $(my_additional_dependencies),\
    dotdot_objects)))

#filter .c files
c_normal_files := $(filter-out ../%,$(filter %.c,$(my_src_files)))
c_normal_objects := $(addprefix $(intermediates)/,$(c_normal_files:.c=.o))
ifneq ($(strip $(c_normal_objects)),)
$(c_normal_objects): $(intermediates)/%.o: $(TOPDIR)$(LOCAL_PATH)/%.c $(my_additional_dependencies)
	$(transform-c-to-o)
endif

ifneq ($(PCLINT),)
c_lint_objects := $(addprefix $(intermediates)/,$(c_normal_files:.c=.lint))
$(c_lint_objects): $(intermediates)/%.lint: $(TOPDIR)$(LOCAL_PATH)/%.c $(my_additional_dependencies)
	$(lint-c-files)
endif

c_objects := $(c_normal_objects) $(dotdot_objects)

#filter .cc files start with ../
dotdot_sources := $(filter ../%.cc, $(my_src_files))
#create dependency between dot_c_obj and dot_c_src
dotdot_objects :=
$(foreach s, $(dotdot_sources),\
  $(eval $(call compile-dotdot-cc-file,$(s),\
    $(my_additional_dependencies),\
    dotdot_objects)))

#filter .cc files
cc_normal_files := $(filter-out ../%,$(filter %.cc,$(my_src_files)))
cc_normal_objects := $(addprefix $(intermediates)/,$(cc_normal_files:.cc=.o))
ifneq ($(strip $(cc_normal_objects)),)
$(cc_normal_objects): $(intermediates)/%.o: $(TOPDIR)$(LOCAL_PATH)/%.cc $(my_additional_dependencies)
	$(transform-cpp-to-o)
endif

ifneq ($(PCLINT),)
cc_lint_objects := $(addprefix $(intermediates)/,$(cc_normal_files:.cc=.lint))
#filter big file cause of process stucked
cc_lint_objects := $(filter-out %fp16_math.lint, $(cc_lint_objects))
$(cc_lint_objects): $(intermediates)/%.lint: $(TOPDIR)$(LOCAL_PATH)/%.cc $(my_additional_dependencies)
	$(lint-cpp-files)
endif

cc_objects := $(cc_normal_objects) $(dotdot_objects)

#filter .s files start with ../
dotdot_sources := $(filter ../%.s, $(my_src_files))
#create dependency between dot_s_obj and dot_s_src
dotdot_objects :=
$(foreach s, $(dotdot_sources),\
  $(eval $(call compile-dotdot-s-file,$(s),\
    $(my_additional_dependencies),\
    dotdot_objects)))

#filter .s files
s_normal_files := $(filter-out ../%,$(filter %.s,$(my_src_files)))
s_normal_objects := $(addprefix $(intermediates)/,$(s_normal_files:.s=.o))
ifneq ($(strip $(s_normal_objects)),)
$(s_normal_objects): $(intermediates)/%.o: $(TOPDIR)$(LOCAL_PATH)/%.s $(my_additional_dependencies)
	$(transform-s-to-o)
endif

s_objects := $(s_normal_files) $(dotdot_objects)

normal_objects := $(cpp_objects) $(c_objects) $(cc_objects) $(s_objects)

gen_o_objects :=

ifneq ($(PCLINT),)
all_objects := $(cpp_lint_objects) $(c_lint_objects) $(cc_lint_objects)
else
all_objects := $(proto_generated_objects) $(normal_objects) $(gen_o_objects)
endif

####################################################
## Import includes
####################################################

import_includes := $(intermediates)/import_includes
import_includes_deps := $(strip \
    $(foreach l, $(my_import_c_include_lib), \
      $(call intermediates-dir-for,$($(my_kind)_OUT_INTERMEDIATES),SHARED_LIBRARIES,$(l))/export_includes))

$(import_includes): PRIVATE_IMPORT_EXPORT_INCLUDES := $(import_includes_deps)
$(import_includes) : $(import_includes_deps)
	@echo Import includes file: $@
	$(hide) mkdir -p $(dir $@) && rm -f $@
ifdef import_includes_deps
	$(hide) for f in $(PRIVATE_IMPORT_EXPORT_INCLUDES); do \
		cat $$f >> $@; \
	done
else
	$(hide) touch $@
endif


$(LOCAL_BUILT_MODULE): PRIVATE_IMPORT_INCLUDES := $(import_includes)

# add_objects depends import_includes
$(all_objects) : $(import_includes)

####################################################



$(LOCAL_BUILT_MODULE): PRIVATE_ALL_OBJECTS := $(all_objects)

built_shared_libraries := \
  $(addprefix $(intermediates_lib_dir)/, \
    $(addsuffix $(TARGET_SHLIB_SUFFIX), \
      $(my_shared_library)))

built_static_libraries := \
  $(foreach lib,$(my_static_library), \
   $(call intermediates-dir-for, $($(my_kind)_OUT_INTERMEDIATES),STATIC_LIBRARIES,$(lib))/$(strip $(lib))$(strip $(STATIC_LIB_SUFFIX)))

built_whole_libraries := \
  $(foreach lib,$(my_whole_static_library), \
   $(call intermediates-dir-for, $($(my_kind)_OUT_INTERMEDIATES),STATIC_LIBRARIES,$(lib))/$(strip $(lib))$(strip $(STATIC_LIB_SUFFIX)))

my_ld_dirs += $(call normalize-libraries-path,$(built_shared_libraries))

ifneq ($(LOCAL_LD_DIRS_NO_SORT),true)
  my_ld_dirs := $(sort $(my_ld_dirs))
endif

$(LOCAL_BUILT_MODULE): PRIVATE_LD_DIRS := $(my_ld_dirs)
$(LOCAL_BUILT_MODULE): PRIVATE_INTERMEDIATES_DIR := $(intermediates)
$(LOCAL_BUILT_MODULE): PRIVATE_ALL_SHARED_LIBRARIES := $(built_shared_libraries)
$(LOCAL_BUILT_MODULE): PRIVATE_ALL_STATIC_LIBRARIES := $(built_static_libraries)
$(LOCAL_BUILT_MODULE): PRIVATE_ALL_WHOLE_STATIC_LIBRARIES := $(built_whole_libraries)

all_libraries := $(built_shared_libraries) $(built_static_libraries) $(built_whole_libraries)



###########################################################
# Export includes
###########################################################

export_includes := $(intermediates)/export_includes
export_cflags := $(foreach d,$(my_export_c_include_dirs),-I $(d))

$(export_includes): PRIVATE_EXPORT_CFLAGS := $(export_cflags)

# Re-export requested headers from shared libraries.
export_include_deps := $(strip \
   $(foreach l,$(LOCAL_EXPORT_SHARED_LIBRARY_HEADERS), \
     $(call intermediates-dir-for,$($(my_kind)_OUT_INTERMEDIATES),SHARED_LIBRARIES,$(l))/export_includes))
# Re-export requested headers from static libraries.
export_include_deps += $(strip \
   $(foreach l,$(LOCAL_EXPORT_STATIC_LIBRARY_HEADERS), \
     $(call intermediates-dir-for,$($(my_kind)_OUT_INTERMEDIATES),STATIC_LIBRARIES,$(l))/export_includes))
# Re-export requested headers from header libraries.
export_include_deps += $(strip \
   $(foreach l,$(LOCAL_EXPORT_HEADER_LIBRARY_HEADERS), \
     $(call intermediates-dir-for,$($(my_kind)_OUT_INTERMEDIATES),HEADER_LIBRARIES,$(l))/export_includes))
$(export_includes): PRIVATE_REEXPORTED_INCLUDES := $(export_include_deps)
# By adding $(my_generated_sources) it makes sure the headers get generated
# before any dependent source files get compiled.
$(export_includes) : $(my_export_c_include_deps) $(my_generated_sources) $(export_include_deps) $(LOCAL_EXPORT_C_INCLUDE_DEPS)
	@echo Export includes file: $< -- $@
	$(hide) mkdir -p $(dir $@) && rm -f $@.tmp && touch $@.tmp
ifdef export_cflags
	$(hide) echo "$(PRIVATE_EXPORT_CFLAGS)" >>$@.tmp
endif
ifdef export_include_deps
	$(hide) for f in $(PRIVATE_REEXPORTED_INCLUDES); do \
		cat $$f >> $@.tmp; \
	done
endif
	$(hide) if cmp -s $@.tmp $@ ; then \
		rm $@.tmp ; \
	else \
		mv $@.tmp $@ ; \
	fi
export_cflags :=

$(LOCAL_BUILT_MODULE) : | $(export_includes)
#############################################################

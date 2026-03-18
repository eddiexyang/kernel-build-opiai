define my-dir
$(strip \
$(eval LOCAL_MODULE_MAKEFILE := $$(lastword $$(MAKEFILE_LIST))) \
$(patsubst %/,%, $(dir $(LOCAL_MODULE_MAKEFILE))) \
)
endef


define get-module-built-file
$(foreach m,$(1),\
  $(eval m := $(strip m)) \
  $(eval $(2) += $(ALL_MODULES.$(m).BUILT)) \
)
endef

define get-device-installed-file
$(foreach m,$(1),\
  $(addprefix $(DEVICE_OUT_ROOT)/,$(m)) \
)
endef

define copy-imgs-to-spc-out
@for img in $(2);do \
  img_type=`echo $$img | awk -F "/" '{print $$2}'`; \
  src_dir=$(OUT_ROOT)/$$img_type; \
  mkdir -p $(1); \
  src_img=`echo $$img | awk -F "/" '{print $$3}'`; \
  cp -rf $$src_dir/$$src_img  $(1)/$$src_img; \
  echo "copy $$src_dir/$$src_img to $(1)/$$src_img"; \
done
endef

define get-host-installed-file
$(foreach m,$(1),\
  $(addprefix $(HOST_OUT_ROOT)/,$(m)) \
)
endef

define transform-cpp-to-o
@mkdir -p $(dir $@)
$(PRIVATE_CXX) \
	$(addprefix -I , $(PRIVATE_C_INCLUDES)) \
	-c \
	$(PRIVATE_GLOBAL_CFLAGS) \
	$(PRIVATE_GLOBAL_CPPFLAGS) \
	$(PRIVATE_CFLAGS) \
	$(PRIVATE_CPPFLAGS) \
	-MD -MF $(patsubst %.o,%.d,$@) -o $@ $<
endef

define transform-c-to-o
@mkdir -p $(dir $@)
$(PRIVATE_CC) \
	$(addprefix -I , $(PRIVATE_C_INCLUDES)) \
	-c \
	$(PRIVATE_GLOBAL_CFLAGS) \
	$(PRIVATE_CFLAGS) \
	-MD -MF $(patsubst %.o,%.d,$@) -o $@ $<
endef

define transform-s-to-o
@mkdir -p $(dir $@)
$(PRIVATE_CC) \
	$(addprefix -I , $(PRIVATE_C_INCLUDES)) \
	-c \
	$(PRIVATE_ASFLAGS) \
	-MD -MF $(patsubst %.o,%.d,$@) -o $@ $<
endef

define transform-proto-to-cc
@mkdir -p $(dir $@)
@mkdir -p $(PROTO_OUT_ROOT)
@echo "Protoc: $@ <= $<"
LD_LIBRARY_PATH=$(PRIVATE_PROTO_LIB_DIR):$$LD_LIBRARY_PATH \
    $(PRIVATE_PROTOC) \
    $(addprefix --proto_path=, $(dir $<))  \
    $(addprefix --cpp_out=, $(dir $@)) \
    $(PRIVATE_PROTOFLAGS)  \
    $<
endef

define transform-proto-to-c
mkdir -p $(dir $@)
@echo "Protoc-c: $@ <= $<"
LD_LIBRARY_PATH=$(PRIVATE_PROTO_LIB_DIR):$$LD_LIBRARY_PATH \
    $(PRIVATE_PROTOC) \
    $(addprefix --proto_path=, $(PRIVATE_MODULE_PATH))  \
    $(PRIVATE_PROTOFLAGS)  \
    $<
endef

#------------------------------------
#modified by y00187525 2017/09/01
#add pclint check
#------------------------------------
define lint-c-files
@echo "PC-lint : $<"
@mkdir -p $(dir $@)
-wine $(LINT_PATH)/LINT-NT.EXE \
	$(addprefix -I, $(PRIVATE_C_INCLUDES)) \
	$(filter -I% -D%, \
	$(PRIVATE_GLOBAL_CFLAGS) \
	$(PRIVATE_CFLAGS)) \
	$(if $(filter %aarch64-linux-gnu-gcc,$(PRIVATE_CC)),$(LINT_PATH)/davinci_arm64.lnt,$(LINT_PATH)/davinci_x86.lnt) \
	$< > $@
endef

define lint-cpp-files
@echo "PC-lint : $<"
@mkdir -p $(dir $@)
-wine $(LINT_PATH)/LINT-NT.EXE \
	$(addprefix -I, $(PRIVATE_C_INCLUDES)) \
	$(filter -I% -D%, \
	$(PRIVATE_GLOBAL_CFLAGS) \
	$(PRIVATE_GLOBAL_CPPFLAGS) \
	$(PRIVATE_CFLAGS) \
	$(PRIVATE_CPPFLAGS)) \
	$(if $(filter %aarch64-linux-gnu-g++,$(PRIVATE_CXX)),$(LINT_PATH)/davinci_arm64.lnt,$(LINT_PATH)/davinci_x86.lnt) \
	$< > $@
endef

define normalize-libraries
$(foreach so,$(filter %.so,$(1)),-l$(patsubst lib%.so,%,$(notdir $(so))))
endef

define intermediates-dir-for
$(strip \
  $(eval _out_dir := $(strip $(1))) \
  $(eval _out_type := $(strip $(2))) \
  $(eval _module_name := $(strip $(3))) \
  $(_out_dir)/$(_out_type)/$(_module_name)_intermediates \
)
endef


DOTDOT_REPLACEMENT := dotdot/

define compile-dotdot-cpp-file
o := $(intermediates)/$(patsubst %.cpp,%.o,$(subst ../,$(DOTDOT_REPLACEMENT),$(1)))
$$(o) : $(TOPDIR)$(LOCAL_PATH)/$(1) $(2)
	$$(transform-cpp-to-o)
-include $$(o:%.o=%.d)
$(3) += $$(o)
endef

define compile-dotdot-proto-file
cc := $(intermediates)/proto/$(LOCAL_PATH)/$(patsubst %.proto,%.pb$(3),$(subst ../,$(DOTDOT_REPLACEMENT),$(1)))
$$(cc) : $(TOPDIR)$(LOCAL_PATH)/$(1) $(4)
	$$(transform-proto-to-cc)
$(2) += $$(cc)
endef

define compile-dotdot-cc-file
o := $(intermediates)/$(patsubst %.cc,%.o,$(subst ../,$(DOTDOT_REPLACEMENT),$(1)))
$$(o) : $(TOPDIR)$(LOCAL_PATH)/$(1) $(2)
	$$(transform-cpp-to-o)
-include $$(o:%.o=%.d)
$(3) += $$(o)
endef

define compile-dotdot-c-file
o := $(intermediates)/$(patsubst %.c,%.o,$(subst ../,$(DOTDOT_REPLACEMENT),$(1)))
$$(o) : $(TOPDIR)$(LOCAL_PATH)/$(1) $(2)
	$$(transform-c-to-o)
-include $$(o:%.o=%.d)
$(3) += $$(o)
endef

define compile-dotdot-s-file
o := $(intermediates)/$(patsubst %.s,%.o,$(subst ../,$(DOTDOT_REPLACEMENT),$(1)))
$$(o) : $(TOPDIR)$(LOCAL_PATH)/$(1) $(2)
	$$(transform-s-to-o)
$(3) += $$(o)
endef

define copy-file-to-target
@mkdir -p $(dir $@)
@cp -pf $< $@
endef

define copy-files-to-folder
@mkdir -p $@
@cp -prf $</* $@/
endef

define copy-one-file
$(2): $(1)
	@echo "copy $$< to $$@"
	@$$(copy-file-to-target)
endef

define create-deps-src-to-dest
$(2): $(1)
endef

define transform-prebuilt-to-target
$(copy-file-to-target)
endef

define transform-prebuilt-to-folder
$(copy-files-to-folder)
endef

define all-makefiles-under
$(wildcard $(1)/*/module.mk)
endef

define all-subdir-makefiles
$(call all-makefiles-under,$(call my-dir))
endef

define _find-src-files_inner
$(eval _fpf := $(sort $(wildcard $(strip $(1))/$(strip $(2))))) \
$(if $(_fpf),$(sort $(strip $(_fpf))),\
  $(eval parent_dir := $(1)/..) \
  $(call _find-src-files_inner,$(parent_dir),$(2)) \
)
endef

#find relative path of src files which are depended on llt
define find-src-files
$(patsubst $(1)/%,%,\
  $(strip \
    $(foreach f, \
      $(shell find $(2) -name "*.cc"), \
      $(call _find-src-files_inner,$(1),$(f)) \
    ) \
  ) \
  $(strip \
    $(foreach f, \
      $(shell find $(2) -name "*.c"), \
      $(call _find-src-files_inner,$(1),$(f)) \
    ) \
  ) \
  $(strip \
    $(foreach f, \
      $(shell find $(2) -name "*.cpp"), \
      $(call _find-src-files_inner,$(1),$(f)) \
    ) \
  ) \
)
endef

define transform-o-to-executable
@mkdir -p $(dir $@)
$(PRIVATE_CXX)  \
	-Wl,-rpath-link,$(PRIVATE_LIB_RPATH_DIR) \
	$(PRIVATE_ALL_OBJECTS) \
	-Wl,--whole-archive \
	$(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES) \
	-Wl,--no-whole-archive \
	-Wl,--start-group \
	-Wl,--no-undefined  \
	$(PRIVATE_ALL_STATIC_LIBRARIES) \
	-Wl,--end-group \
	$(call normalize-libraries,$(PRIVATE_ALL_SHARED_LIBRARIES)) \
	-o $@ \
	$(PRIVATE_GLOBAL_LD_DIRS) \
	$(PRIVATE_GLOBAL_LDFLAGS) \
	$(PRIVATE_LD_DIRS) \
	$(PRIVATE_LDFLAGS) \
	$(PRIVATE_LDLIBS)
endef

define transform-o-to-shared-lib
@mkdir -p $(dir $@)
$(PRIVATE_CXX) \
	-Wl,-rpath-link,$(PRIVATE_LIB_RPATH_DIR) \
	-Wl,-soname,$(notdir $@) \
	-shared \
	$(PRIVATE_ALL_OBJECTS) \
	-Wl,--whole-archive \
	$(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES) \
	-Wl,--no-whole-archive \
	-Wl,--no-undefined  \
	$(PRIVATE_ALL_STATIC_LIBRARIES) \
	$(call normalize-libraries,$(PRIVATE_ALL_SHARED_LIBRARIES)) \
	-o $@ \
	$(PRIVATE_GLOBAL_LDFLAGS) \
	$(PRIVATE_LD_DIRS) \
	$(PRIVATE_LDFLAGS) \
	$(PRIVATE_LDLIBS)
endef

define _concat-if-arg2-not-empty
$(if $(2),$(1) $(2))
endef

define split-long-arguments
$(1) $(wordlist 1,500,$(2))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 501,1000,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 1001,1500,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 1501,2000,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 2001,2500,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 2501,3000,$(2)))
$(call _concat-if-arg2-not-empty,$(1),$(wordlist 3001,99999,$(2)))
endef

# $(1): the full path of the source static library.
define _extract-and-include-single-target-whole-static-lib
ldir=$(PRIVATE_INTERMEDIATES_DIR)/WHOLE/$(basename $(notdir $(1)))_objs;\
    rm -rf $$ldir; \
    mkdir -p $$ldir; \
    cp $(1) $$ldir; \
    lib_to_include=$$ldir/$(notdir $(1)); \
    filelist=; \
    subdir=0; \
    for f in `$(PRIVATE_AR) t $(1)`; do \
        if [ -e $$ldir/$$f ]; then \
            mkdir $$ldir/$$subdir; \
            ext=$$subdir/; \
            subdir=$$((subdir+1)); \
            $(PRIVATE_AR) m $$lib_to_include $$f; \
        else \
            ext=; \
        fi; \
        $(PRIVATE_AR) p $$lib_to_include $$f > $$ldir/$$ext$$f; \
        filelist="$$filelist $$ldir/$$ext$$f"; \
    done ; \
    $(PRIVATE_AR) $(PRIVATE_GLOBAL_ARFLAGS) $@ $$filelist

endef

define extract-and-include-whole-static-libs-first
$(if $(strip $(1)),cp $(1) $@)
endef

define extract-and-include-target-whole-static-libs
$(call extract-and-include-whole-static-libs-first, $(firstword $(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES)))
$(foreach lib,$(wordlist 2,999,$(PRIVATE_ALL_WHOLE_STATIC_LIBRARIES)), \
    $(call _extract-and-include-single-target-whole-static-lib, $(lib)))
endef

define transform-o-to-static-lib
@mkdir -p $(dir $@)
@rm -f $@
$(extract-and-include-target-whole-static-libs)
$(call split-long-arguments,$(PRIVATE_AR) \
    $(PRIVATE_GLOBAL_ARFLAGS) $@,$(PRIVATE_ALL_OBJECTS))
endef


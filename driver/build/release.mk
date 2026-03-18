
TOOLCHAIN_PCTRACE_INSTALLED_MODULES := \
$(call get-host-installed-file,$(TOOLCHAIN_PCTRACE_HOST_MODULES))

TOOLCHAIN_PROFILING_INSTALLED_MODULES := \
$(call get-host-installed-file,$(TOOLCHAIN_PROFILING_HOST_MODULES))

THIRD_PARTY_INSTALLED_MODULES := \
$(call get-host-installed-file,$(THIRD_PARTY_HOST_MODULES))

HIAIENGINE_INSTALLED_MODULES := \
$(call get-host-installed-file,$(HIAIENGINE_HOST_MODULES))

DRIVER_HOST_INSTALLED_MODULES := \
$(call get-host-installed-file,$(DRIVER_HOST_MODULES))

DRIVER_DEVICE_INSTALLED_MODULES := \
$(call get-device-installed-file,$(DRIVER_DEVICE_MODULES))

UPGRADE_TOOL_INSTALLED_MODULES := \
$(call get-host-installed-file,$(UPGRADE_TOOL_HOST_MODULES))

TOOLCHAIN_DEBUG_INSTALLED_MODULES := \
$(call get-host-installed-file,$(TOOLCHAIN_DEBUG_HOST_MODULES))

TOOLCHAIN_IDE_DAEMON_INSTALLED_MODULES := \
$(call get-host-installed-file,$(TOOLCHAIN_IDE_DAEMON_HOST_MODULES))

TOOLCHAIN_LOG_INSTALLED_MODULES := \
$(call get-host-installed-file,$(TOOLCHAIN_LOG_HOST_MODULES))

MMPA_INSTALLED_MODULES := \
$(call get-host-installed-file,$(MMPA_HOST_MODULES))

NON_HOST_RELEASE_MODULES := SAFECITY_HOST_MODULES TOOLCHAIN_IDE_DAEMON_HOST_MODULES

INSTALLED_ALL_HOST_RELEASED_MODULES :=
$(foreach m,$(filter-out $(NON_HOST_RELEASE_MODULES), $(CUSTOM_HOST_MODULES_VARIABLES)),\
  $(eval INSTALLED_ALL_HOST_RELEASED_MODULES += $(addprefix $(HOST_OUT_ROOT)/,$($(m)))) \
)

.PHONY: release_modules
release_modules: $(INSTALLED_ALL_HOST_RELEASED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"

.PHONY: toolchain_pctrace
toolchain_pctrace: $(TOOLCHAIN_PCTRACE_INSTALLED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"

.PHONY: toolchain_profiling
toolchain_profiling: $(TOOLCHAIN_PROFILING_INSTALLED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"

.PHONY: third_party
third_party: $(THIRD_PARTY_INSTALLED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"

.PHONY: driver_host
driver_host: $(DRIVER_HOST_INSTALLED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"

.PHONY: driver_device
driver_device: $(DRIVER_DEVICE_INSTALLED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"

.PHONY: upgrade_tool
upgrade_tool: $(UPGRADE_TOOL_INSTALLED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"

.PHONY:toolchain_debug
toolchain_debug: $(TOOLCHAIN_DEBUG_INSTALLED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"

.PHONY:toolchain_log
toolchain_log: $(TOOLCHAIN_LOG_INSTALLED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"

.PHONY:mmpa
mmpa: $(MMPA_INSTALLED_MODULES)
	$(call copy-imgs-to-spc-out,$(RELEASE_DIR),$^)
	@echo -e "\n\033[32m### build $@ modules successfully ###\033[0m"


.PHONY: kernel

kernel: | ensure-output-dirs
	printf 'start make kernel\n'
	printf 'kernel release suffix: %s\n' "$(KERNEL_RELEASE_SUFFIX)"
	if [ -n "$(strip $(CCACHE_BIN))" ]; then printf 'ccache enabled: %s\n' "$(CCACHE_BIN)"; else printf 'ccache disabled\n'; fi
	$(MAKE) --no-print-directory clean-kernel-outputs
	$(MAKE) --no-print-directory sync-kernel-source-inputs
	$(MAKE) --no-print-directory clean-kernel-build-artifacts
	$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) "$(KERNEL_DEFCONFIG)"
	$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) olddefconfig
	if [ "$(SKIP_MENUCONFIG)" = "1" ]; then
		printf 'skip menuconfig because SKIP_MENUCONFIG=1\n'
	else
		$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) menuconfig
	fi
	$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) Image -j"$(JOBS)"
	$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) modules -j"$(JOBS)"
	$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) INSTALL_MOD_PATH="$(OUTPUT_DIR)/modules" modules_install
	cp -f "$(KERNEL_WORKSPACE)/arch/arm64/boot/Image" "$(OUTPUT_DIR)/Image.raw"
	raw_image_magic="$$(od -An -tx1 -N4 -j56 "$(OUTPUT_DIR)/Image.raw" | tr -d ' \n')"
	if [ "$$raw_image_magic" != "41524d64" ]; then \
		printf 'error: %s/Image.raw is not a standard ARM64 Linux Image (magic: %s)\n' "$(OUTPUT_DIR)" "$$raw_image_magic" >&2; \
		exit 1; \
	fi
	printf 'generate %s/modules success\n' "$(OUTPUT_DIR)"
	printf 'generate standard ARM64 Linux Image %s/Image.raw success\n' "$(OUTPUT_DIR)"

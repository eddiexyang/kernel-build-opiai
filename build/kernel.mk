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
	cp -f "$(KERNEL_WORKSPACE)/arch/arm64/boot/Image" "$(OUTPUT_DIR)/Image"
	python3 "$(SIGNING_DIR)/esbc_header/esbc_header.py" \
		-raw_img "$(OUTPUT_DIR)/Image" \
		-out_img "$(OUTPUT_DIR)/Image" \
		-version "$(HEADER_VERSION)" \
		-nvcnt 0 \
		-tag uimage \
		-platform hi1910Brc
	python3 "$(SIGNING_DIR)/image_pack/image_pack.py" \
		-raw_img "$(OUTPUT_DIR)/Image" \
		-out_img "$(OUTPUT_DIR)/Image" \
		-platform hi1910Brc \
		-version "$(HEADER_VERSION)"
	printf 'sign %s/Image success\n' "$(OUTPUT_DIR)"
	image_size="$$(stat -c '%s' "$(OUTPUT_DIR)/Image")"
	if [ "$$image_size" -gt 31457280 ]; then \
		printf 'error: Image size %s bytes exceeds 31457280 (30 MiB) limit\n' "$$image_size" >&2; \
		exit 1; \
	fi
	printf 'Image size: %s bytes (limit 31457280)\n' "$$image_size"
	printf 'generate %s/modules success\n' "$(OUTPUT_DIR)"
	printf 'generate %s/Image success\n' "$(OUTPUT_DIR)"

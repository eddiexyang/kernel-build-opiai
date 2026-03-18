.PHONY: kernel-patch kernel

kernel-patch: $(KERNEL_WORKSPACE_MARKER)
	printf 'patched kernel workspace is [%s]\n' "$(KERNEL_WORKSPACE)"

kernel: | ensure-output-dirs
	printf 'start make kernel\n'
	printf 'kernel release suffix: %s\n' "$(KERNEL_RELEASE_SUFFIX)"
	$(MAKE) --no-print-directory clean-kernel-outputs
	$(MAKE) --no-print-directory kernel-patch
	$(MAKE) --no-print-directory clean-kernel-build-artifacts
	$(MAKE) -C "$(KERNEL_WORKSPACE)" ARCH="$(ARCH_TYPE)" CROSS_COMPILE="$(CROSS_COMPILE_PREFIX)" LOCALVERSION="$(KERNEL_LOCALVERSION)" "$(KERNEL_DEFCONFIG)"
	if [ "$(SKIP_MENUCONFIG)" = "1" ]; then
		printf 'skip menuconfig because SKIP_MENUCONFIG=1\n'
	else
		$(MAKE) -C "$(KERNEL_WORKSPACE)" ARCH="$(ARCH_TYPE)" CROSS_COMPILE="$(CROSS_COMPILE_PREFIX)" LOCALVERSION="$(KERNEL_LOCALVERSION)" menuconfig
	fi
	$(MAKE) -C "$(KERNEL_WORKSPACE)" ARCH="$(ARCH_TYPE)" CROSS_COMPILE="$(CROSS_COMPILE_PREFIX)" LOCALVERSION="$(KERNEL_LOCALVERSION)" Image -j"$(JOBS)"
	$(MAKE) -C "$(KERNEL_WORKSPACE)" ARCH="$(ARCH_TYPE)" CROSS_COMPILE="$(CROSS_COMPILE_PREFIX)" LOCALVERSION="$(KERNEL_LOCALVERSION)" modules -j"$(JOBS)"
	$(MAKE) -C "$(KERNEL_WORKSPACE)" ARCH="$(ARCH_TYPE)" CROSS_COMPILE="$(CROSS_COMPILE_PREFIX)" LOCALVERSION="$(KERNEL_LOCALVERSION)" INSTALL_MOD_PATH="$(OUTPUT_DIR)/modules" modules_install
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
	printf 'generate %s/modules success\n' "$(OUTPUT_DIR)"
	printf 'generate %s/Image success\n' "$(OUTPUT_DIR)"

.PHONY: driver

driver: | ensure-output-dirs
	printf 'start make driver\n'
	printf 'kernel release suffix: %s\n' "$(KERNEL_RELEASE_SUFFIX)"
	if [ -n "$(strip $(CCACHE_BIN))" ]; then printf 'ccache enabled: %s\n' "$(CCACHE_BIN)"; else printf 'ccache disabled\n'; fi
	$(MAKE) --no-print-directory clean-driver-outputs
	$(MAKE) --no-print-directory sync-kernel-source-inputs
	$(MAKE) --no-print-directory clean-kernel-build-artifacts
	trap 'status=$$?; $(MAKE) --no-print-directory cleanup-driver-dependencies >/dev/null 2>&1 || true; exit $$status' EXIT
	$(MAKE) --no-print-directory prepare-driver-dependencies
	cd "$(DRIVER_SOURCE_DIR)"
	$(MAKE) driver_device \
		PRODUCT="$(DRIVER_PRODUCT)" \
		PRODUCT_SIDE=device \
		CCACHE="$(CCACHE)" \
		CROSS_COMPILE="$(CROSS_COMPILE_PREFIX)" \
		LOCALVERSION="$(KERNEL_LOCALVERSION)" \
		KERNEL_DIR="$(DRIVER_SOURCE_DIR)/kernel/linux-source" \
		KERNEL_DEFCONFIG="$(DRIVER_SOURCE_DIR)/kernel/linux-source/arch/arm64/configs/$(KERNEL_DEFCONFIG)" \
		build_device=true \
		-j1
	$(MAKE) driver_host \
		PRODUCT="$(DRIVER_PRODUCT)" \
		PRODUCT_SIDE=host \
		CCACHE="$(CCACHE)" \
		CROSS_COMPILE="$(CROSS_COMPILE_PREFIX)" \
		LOCALVERSION="$(KERNEL_LOCALVERSION)" \
		KERNEL_DIR="$(DRIVER_SOURCE_DIR)/kernel/linux-source" \
		KERNEL_DEFCONFIG="$(DRIVER_SOURCE_DIR)/kernel/linux-source/arch/arm64/configs/$(KERNEL_DEFCONFIG)" \
		-j1
	mkdir -p "$(OUTPUT_DIR)/driver_modules"
	cp -rf "$(DRIVER_SOURCE_DIR)/out/device/." "$(OUTPUT_DIR)/driver_modules/"
	rm -rf "$(OUTPUT_DIR)/driver_modules_host"
	mkdir -p "$(OUTPUT_DIR)/driver_modules_host"
	cp -rf "$(DRIVER_SOURCE_DIR)/out/host/." "$(OUTPUT_DIR)/driver_modules_host/"
	find "$(OUTPUT_DIR)/driver_modules" -maxdepth 1 -type f -name '*.ko' -printf '%f\n' \
		| sed 's/\.ko$$//' | sort -u > "$(OUTPUT_DIR)/ascend310b-driver-modules.conf"
	trap - EXIT
	$(MAKE) --no-print-directory cleanup-driver-dependencies >/dev/null 2>&1 || true
	printf 'generate %s/driver_modules success\n' "$(OUTPUT_DIR)"
	printf 'generate %s/driver_modules_host success\n' "$(OUTPUT_DIR)"
	printf 'generate %s/ascend310b-driver-modules.conf success\n' "$(OUTPUT_DIR)"
	printf 'make driver success\n'

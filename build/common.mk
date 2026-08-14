.PHONY: ensure-output-dirs sync-kernel-source-inputs clean-kernel-outputs clean-kernel-workspace \
	clean-kernel-build-artifacts clean-driver-outputs clean-all-outputs \
	prepare-driver-dependencies cleanup-driver-dependencies

ensure-output-dirs:
	mkdir -p "$(OUTPUT_DIR)" "$(WORKSPACE_DIR)"

sync-kernel-source-inputs: | ensure-output-dirs
	printf 'prepare in-place kernel source tree: %s\n' "$(KERNEL_WORKSPACE)"
	test -f "$(KERNEL_WORKSPACE)/arch/arm64/configs/$(KERNEL_DEFCONFIG)" || { \
		printf 'error: missing required path: %s\n' "$(KERNEL_WORKSPACE)/arch/arm64/configs/$(KERNEL_DEFCONFIG)" >&2; \
		exit 1; \
	}
	test -f "$(KERNEL_WORKSPACE)/certs/ELF_Common_RSA4096_CN_20191009_Huawei.pem" || { \
		printf 'error: missing required path: %s\n' "$(KERNEL_WORKSPACE)/certs/ELF_Common_RSA4096_CN_20191009_Huawei.pem" >&2; \
		exit 1; \
	}
	printf 'build directly from %s\n' "$(KERNEL_WORKSPACE)"

clean-kernel-outputs:
	rm -rf "$(OUTPUT_DIR)/modules" "$(OUTPUT_DIR)/Image" "$(OUTPUT_DIR)/Image.raw"

clean-kernel-workspace:
	printf 'kernel source is built in-place; nothing to remove under workspace\n'

clean-kernel-build-artifacts:
	if [ ! -d "$(KERNEL_WORKSPACE)" ]; then
		exit 0
	fi
	$(MAKE) --no-print-directory sync-kernel-source-inputs
	printf 'preserve in-place kernel build cache under %s\n' "$(KERNEL_WORKSPACE)"

prepare-driver-dependencies:
	printf 'wire driver tree to shared assets\n'
	rm -rf "$(DRIVER_SOURCE_DIR)/kernel"
	mkdir -p "$(DRIVER_SOURCE_DIR)/kernel"
	test -d "$(KERNEL_WORKSPACE)" || { printf 'error: missing required path: %s\n' "$(KERNEL_WORKSPACE)" >&2; exit 1; }
	ln -s "$(KERNEL_WORKSPACE)" "$(DRIVER_SOURCE_DIR)/kernel/linux-source"
	mkdir -p "$(DRIVER_SOURCE_DIR)/config/feature_config"
	grep -v '^#' "$(CONFIG_FEATURE_FILE)" | sed 's/^CONFIG/#define CONFIG/g' > "$(DRIVER_SOURCE_DIR)/config/feature_config/feature.h"
	grep -v '^#' "$(CONFIG_FEATURE_FILE)" | \
		sed -r 's/^(.*)=(.*)/CONFIG_DEFINES += -D\1=\2\n\1 := \2/;1iCONFIG_DEFINES :=' > "$(DRIVER_SOURCE_DIR)/config/feature_config/feature.mk"

cleanup-driver-dependencies:
	rm -rf "$(DRIVER_SOURCE_DIR)/kernel"

clean-driver-outputs:
	rm -rf "$(OUTPUT_DIR)/driver_modules" "$(OUTPUT_DIR)/driver_modules_host"
	$(MAKE) --no-print-directory cleanup-driver-dependencies
	$(MAKE) -C "$(DRIVER_SOURCE_DIR)" clean 2>/dev/null || true

clean-all-outputs:
	rm -rf "$(OUTPUT_DIR)" "$(WORKSPACE_DIR)"
	mkdir -p "$(OUTPUT_DIR)" "$(WORKSPACE_DIR)"
	$(MAKE) --no-print-directory cleanup-driver-dependencies
	$(MAKE) -C "$(DRIVER_SOURCE_DIR)" clean >/dev/null 2>&1 || true

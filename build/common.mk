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
	$(MAKE) --no-print-directory prepare-driver-dependencies
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
	pcie_rc_link="$(KERNEL_WORKSPACE)/drivers/pci/ascend_vendor"; \
	test -d "$(KERNEL_WORKSPACE)/drivers/pci" || { printf 'error: missing PCI source directory: %s\n' "$(KERNEL_WORKSPACE)/drivers/pci" >&2; exit 1; }; \
	if [ -L "$$pcie_rc_link" ]; then \
		test "$$(readlink "$$pcie_rc_link")" = "$(DRIVER_SOURCE_DIR)" || { printf 'error: unexpected PCIe RC link: %s\n' "$$pcie_rc_link" >&2; exit 1; }; \
	elif [ -e "$$pcie_rc_link" ]; then \
		printf 'error: refusing to replace existing PCIe RC path: %s\n' "$$pcie_rc_link" >&2; exit 1; \
	else \
		ln -s "$(DRIVER_SOURCE_DIR)" "$$pcie_rc_link"; \
	fi; \
	grep -Fq 'kernel-build-opiai: built-in Ascend PCIe RC' "$(KERNEL_WORKSPACE)/drivers/pci/Makefile" || \
		printf '\n# kernel-build-opiai: built-in Ascend PCIe RC\nobj-$$(CONFIG_PCI) += ascend_vendor/\n' >> "$(KERNEL_WORKSPACE)/drivers/pci/Makefile"

cleanup-driver-dependencies:
	rm -rf "$(DRIVER_SOURCE_DIR)/kernel"
	if [ -L "$(KERNEL_WORKSPACE)/drivers/pci/ascend_vendor" ]; then rm -f "$(KERNEL_WORKSPACE)/drivers/pci/ascend_vendor"; fi
	if grep -Fq 'kernel-build-opiai: built-in Ascend PCIe RC' "$(KERNEL_WORKSPACE)/drivers/pci/Makefile"; then \
		awk '/kernel-build-opiai: built-in Ascend PCIe RC/{skip=1; next} skip && index($$0, "ascend_vendor/"){skip=0; next} !skip{print}' \
			"$(KERNEL_WORKSPACE)/drivers/pci/Makefile" > "$(KERNEL_WORKSPACE)/drivers/pci/Makefile.kernel-build-opiai.tmp"; \
		mv "$(KERNEL_WORKSPACE)/drivers/pci/Makefile.kernel-build-opiai.tmp" "$(KERNEL_WORKSPACE)/drivers/pci/Makefile"; \
	fi

clean-driver-outputs:
	rm -rf "$(OUTPUT_DIR)/driver_modules" "$(OUTPUT_DIR)/driver_modules_host"
	$(MAKE) --no-print-directory cleanup-driver-dependencies
	$(MAKE) -C "$(DRIVER_SOURCE_DIR)" clean 2>/dev/null || true

clean-all-outputs:
	rm -rf "$(OUTPUT_DIR)" "$(WORKSPACE_DIR)"
	mkdir -p "$(OUTPUT_DIR)" "$(WORKSPACE_DIR)"
	$(MAKE) --no-print-directory cleanup-driver-dependencies
	$(MAKE) -C "$(DRIVER_SOURCE_DIR)" clean >/dev/null 2>&1 || true

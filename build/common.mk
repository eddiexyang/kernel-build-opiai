.PHONY: ensure-output-dirs clean-kernel-outputs clean-kernel-workspace clean-kernel-build-artifacts \
	clean-driver-outputs clean-all-outputs prepare-driver-dependencies cleanup-driver-dependencies

ensure-output-dirs:
	mkdir -p "$(OUTPUT_DIR)" "$(WORKSPACE_DIR)"

$(KERNEL_WORKSPACE_MARKER): | ensure-output-dirs
	printf 'prepare shared kernel workspace: %s\n' "$(KERNEL_WORKSPACE)"
	rm -rf "$(KERNEL_WORKSPACE)"
	mkdir -p "$(KERNEL_WORKSPACE)"
	cp -a "$(LINUX_SOURCE_DIR)/." "$(KERNEL_WORKSPACE)/"
	mkdir -p "$(KERNEL_WORKSPACE)/arch/arm64/configs"
	cp -f "$(KERNEL_DEFCONFIG_DIR)/$(KERNEL_DEFCONFIG)" \
		"$(KERNEL_WORKSPACE)/arch/arm64/configs/$(KERNEL_DEFCONFIG)"
	rm -rf "$(WORKSPACE_DIR)/kernel/spmi_hisi"
	mkdir -p "$(WORKSPACE_DIR)/kernel"
	cp -a "$(KERNEL_SPMI_DIR)" "$(WORKSPACE_DIR)/kernel/spmi_hisi"
	rm -rf "$(KERNEL_WORKSPACE)/spmi_hisi"
	ln -s "../spmi_hisi" "$(KERNEL_WORKSPACE)/spmi_hisi"
	rm -rf "$(WORKSPACE_DIR)/libc_sec" "$(WORKSPACE_DIR)/kernel/libc_sec"
	cp -a "$(LIBC_SEC_DIR)" "$(WORKSPACE_DIR)/libc_sec"
	ln -s "../libc_sec" "$(WORKSPACE_DIR)/kernel/libc_sec"
	cp -f "$(KERNEL_TOOLS_DIR)/publickey/ELF_Common_RSA4096_CN_20191009_Huawei.der" \
		"$(KERNEL_WORKSPACE)/certs/ELF_Common_RSA4096_CN_20191009_Huawei.der"
	cd "$(KERNEL_WORKSPACE)"
	if [ -n "$(strip $(KERNEL_PATCH_SERIES))" ]; then
		printf 'apply kernel patch series: %s\n' "$(KERNEL_PATCH_SERIES)"
		"$(KERNEL_TOOLS_DIR)/apply-patches" "$(KERNEL_PATCH_SERIES)" "$(ROOT_DIR)"
	else
		printf 'skip kernel patch series for current source baseline\n'
	fi
	if [ -n "$(strip $(KERNEL_PRODUCT_PATCH_SERIES))" ]; then
		printf 'apply product patch series: %s\n' "$(KERNEL_PRODUCT_PATCH_SERIES)"
		"$(KERNEL_TOOLS_DIR)/apply-patches" "$(KERNEL_PRODUCT_PATCH_SERIES)" "$(ROOT_DIR)"
	else
		printf 'skip product patch series for current source baseline\n'
	fi
	if [ "$(KERNEL_APPLY_EMMC_PATCH_SWAP)" = "1" ]; then
		printf 'switch eMMC patch from mini to Ascend310B\n'
		patch -E -Rp1 --batch --no-backup-if-mismatch -i "$(KERNEL_EMMC_MINI_PATCH)"
		patch -E -p1 --batch --no-backup-if-mismatch -i "$(KERNEL_EMMC_310B_PATCH)"
	else
		printf 'skip legacy eMMC patch swap\n'
	fi
	touch "$@"

clean-kernel-outputs:
	rm -rf "$(OUTPUT_DIR)/modules" "$(OUTPUT_DIR)/Image"

clean-kernel-workspace:
	rm -rf "$(WORKSPACE_DIR)/kernel"

clean-kernel-build-artifacts:
	if [ ! -d "$(KERNEL_WORKSPACE)" ]; then
		exit 0
	fi
	$(MAKE) -C "$(KERNEL_WORKSPACE)" ARCH="$(ARCH_TYPE)" CROSS_COMPILE="$(CROSS_COMPILE_PREFIX)" mrproper >/dev/null 2>&1 || true
	mkdir -p "$(KERNEL_WORKSPACE)/arch/arm64/configs"
	cp -f "$(KERNEL_DEFCONFIG_DIR)/$(KERNEL_DEFCONFIG)" \
		"$(KERNEL_WORKSPACE)/arch/arm64/configs/$(KERNEL_DEFCONFIG)"
	cp -f "$(KERNEL_TOOLS_DIR)/publickey/ELF_Common_RSA4096_CN_20191009_Huawei.der" \
		"$(KERNEL_WORKSPACE)/certs/ELF_Common_RSA4096_CN_20191009_Huawei.der"

prepare-driver-dependencies:
	printf 'wire driver tree to shared assets\n'
	rm -rf "$(DRIVER_SOURCE_DIR)/abl" "$(DRIVER_SOURCE_DIR)/libc_sec" "$(DRIVER_SOURCE_DIR)/kernel"
	mkdir -p "$(DRIVER_SOURCE_DIR)/inc" "$(DRIVER_SOURCE_DIR)/kernel"
	if [ -L "$(DRIVER_SOURCE_DIR)/inc/toolchain" ]; then
		rm -rf "$(DRIVER_SOURCE_DIR)/inc/toolchain"
	fi
	test -e "$(DRIVER_SOURCE_DIR)/inc/toolchain/bbox/bbox_proxy.h" || { printf 'error: missing required path: %s\n' "$(DRIVER_SOURCE_DIR)/inc/toolchain/bbox/bbox_proxy.h" >&2; exit 1; }
	mkdir -p "$(ABL_DIR)/bbox/inc"
	rm -rf "$(ABL_DIR)/bbox/inc/bbox"
	ln -s "../../../driver/inc/toolchain/bbox" "$(ABL_DIR)/bbox/inc/bbox"
	ln -s "$(ABL_DIR)" "$(DRIVER_SOURCE_DIR)/abl"
	ln -s "$(LIBC_SEC_DIR)" "$(DRIVER_SOURCE_DIR)/libc_sec"
	cp -al "$(KERNEL_WORKSPACE)" "$(DRIVER_SOURCE_DIR)/kernel/linux-source"
	cp -al "$(KERNEL_SPMI_DIR)" "$(DRIVER_SOURCE_DIR)/kernel/spmi_hisi"
	mkdir -p "$(DRIVER_SOURCE_DIR)/config/feature_config"
	grep -v '^#' "$(CONFIG_FEATURE_FILE)" | sed 's/^CONFIG/#define CONFIG/g' > "$(DRIVER_SOURCE_DIR)/config/feature_config/feature.h"
	grep -v '^#' "$(CONFIG_FEATURE_FILE)" | \
		sed -r 's/^(.*)=(.*)/CONFIG_DEFINES += -D\1=\2\n\1 := \2/;1iCONFIG_DEFINES :=' > "$(DRIVER_SOURCE_DIR)/config/feature_config/feature.mk"
	grep -v '^#' "$(CONFIG_FEATURE_FILE)" | \
		sed -r 's/^(.*)=(.*)/list(APPEND CONFIG_DEFINES \1=\2)\nset(\1 \2)/;1iset(CONFIG_DEFINES)' > "$(DRIVER_SOURCE_DIR)/config/feature_config/feature.cmake"

cleanup-driver-dependencies:
	rm -rf "$(DRIVER_SOURCE_DIR)/abl" "$(DRIVER_SOURCE_DIR)/libc_sec" "$(DRIVER_SOURCE_DIR)/kernel"
	if [ -L "$(DRIVER_SOURCE_DIR)/inc/toolchain" ]; then
		rm -rf "$(DRIVER_SOURCE_DIR)/inc/toolchain"
	fi

clean-driver-outputs:
	rm -rf "$(OUTPUT_DIR)/driver_modules"
	$(MAKE) --no-print-directory cleanup-driver-dependencies
	$(MAKE) -C "$(DRIVER_SOURCE_DIR)" clean >/dev/null 2>&1 || true

clean-all-outputs:
	rm -rf "$(OUTPUT_DIR)" "$(WORKSPACE_DIR)"
	mkdir -p "$(OUTPUT_DIR)" "$(WORKSPACE_DIR)"
	$(MAKE) --no-print-directory cleanup-driver-dependencies
	$(MAKE) -C "$(DRIVER_SOURCE_DIR)" clean >/dev/null 2>&1 || true

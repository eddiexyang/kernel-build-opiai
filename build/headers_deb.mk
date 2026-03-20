.PHONY: headers-deb

headers-deb: | ensure-output-dirs
	kernel_release=
	package_name=
	package_arch=
	package_version=
	deb_workspace=
	pkg_root=
	debian_dir=
	headers_dir=
	modules_dir=
	deb_path=
	printf 'start generate headers deb\n'
	printf 'kernel release suffix: %s\n' "$(KERNEL_RELEASE_SUFFIX)"
	if [ -n "$(strip $(CCACHE_BIN))" ]; then printf 'ccache enabled: %s\n' "$(CCACHE_BIN)"; else printf 'ccache disabled\n'; fi
	$(MAKE) --no-print-directory sync-kernel-source-inputs
	$(MAKE) --no-print-directory clean-kernel-build-artifacts
	$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) "$(KERNEL_DEFCONFIG)"
	$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) olddefconfig
	$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) modules_prepare
	$(MAKE) -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) modules -j"$(JOBS)"
	kernel_release="$$( $(MAKE) -s -C "$(KERNEL_WORKSPACE)" $(KERNEL_MAKE_VARS) kernelrelease )"
	[ -n "$$kernel_release" ] || { printf 'error: failed to determine kernel release\n' >&2; exit 1; }
	package_name="$${HEADERS_DEB_PACKAGE_NAME:-linux-headers-$$kernel_release}"
	package_arch="$${HEADERS_DEB_PACKAGE_ARCH:-$(MODULES_DEB_PACKAGE_ARCH)}"
	package_version="$${HEADERS_DEB_PACKAGE_VERSION:-$(BUILD_TIMESTAMP_MINUTE)}"
	deb_workspace="$(WORKSPACE_DIR)/headers-deb"
	pkg_root="$$deb_workspace/pkgroot"
	debian_dir="$$pkg_root/DEBIAN"
	headers_dir="$$pkg_root/usr/src/$$package_name"
	modules_dir="$$pkg_root/lib/modules/$$kernel_release"
	deb_path="$(OUTPUT_DIR)/$${package_name}_$${package_version}_$${package_arch}.deb"
	rm -rf "$$deb_workspace"
	mkdir -p "$$headers_dir" "$$debian_dir" "$$modules_dir"
	for path in .config Makefile Kbuild Kconfig Module.symvers System.map include scripts arch/"$(ARCH_TYPE)"; do \
		if [ -e "$(KERNEL_WORKSPACE)/$$path" ]; then \
			mkdir -p "$$headers_dir/$$(dirname "$$path")"; \
			cp -a "$(KERNEL_WORKSPACE)/$$path" "$$headers_dir/$$path"; \
		fi; \
	done
	ln -s "../../../usr/src/$$package_name" "$$modules_dir/build"
	ln -s "../../../usr/src/$$package_name" "$$modules_dir/source"
	printf '%s\n' \
		"Package: $${package_name}" \
		"Version: $${package_version}" \
		'Section: kernel' \
		'Priority: optional' \
		"Architecture: $${package_arch}" \
		"Maintainer: $${HEADERS_DEB_MAINTAINER:-Ascend310B Build System <noreply@local>}" \
		"Description: Linux kernel headers for $${kernel_release}" \
		> "$$debian_dir/control"
	rm -f "$$deb_path"
	fakeroot dpkg-deb --build "$$pkg_root" "$$deb_path" >/dev/null
	printf 'generate %s success\n' "$$deb_path"

.PHONY: modules-deb

modules-deb: | ensure-output-dirs
	depmod_bin=
	kernel_release=
	package_name=
	package_arch=
	package_version=
	deb_workspace=
	pkg_root=
	debian_dir=
	driver_target_dir=
	autoload_conf_src=
	autoload_conf_name=
	autoload_conf_dir=
	deb_path=
	test -e "$(OUTPUT_DIR)/modules/lib/modules" || { printf 'error: missing required path: %s\n' "$(OUTPUT_DIR)/modules/lib/modules" >&2; exit 1; }
	test -e "$(OUTPUT_DIR)/driver_modules" || { printf 'error: missing required path: %s\n' "$(OUTPUT_DIR)/driver_modules" >&2; exit 1; }
	depmod_bin="$$(command -v depmod 2>/dev/null || true)"
	if [ -z "$$depmod_bin" ]; then
		for candidate in /sbin/depmod /usr/sbin/depmod; do
			if [ -x "$$candidate" ]; then
				depmod_bin="$$candidate"
				break
			fi
		done
	fi
	[ -n "$$depmod_bin" ] || { printf 'error: depmod is required to build modules-deb\n' >&2; exit 1; }
	mapfile -t releases < <(find "$(OUTPUT_DIR)/modules/lib/modules" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort)
	[ "$${#releases[@]}" -eq 1 ] || { printf 'error: expected exactly one kernel release under %s, found %s\n' "$(OUTPUT_DIR)/modules/lib/modules" "$${#releases[@]}" >&2; exit 1; }
	kernel_release="$${releases[0]}"
	package_name="$${MODULES_DEB_PACKAGE_NAME:-linux-modules-$$kernel_release}"
	package_arch="$${MODULES_DEB_PACKAGE_ARCH:-$(MODULES_DEB_PACKAGE_ARCH)}"
	package_version="$${MODULES_DEB_PACKAGE_VERSION:-$(BUILD_TIMESTAMP_MINUTE)}"
	deb_workspace="$(WORKSPACE_DIR)/modules-deb"
	pkg_root="$$deb_workspace/pkgroot"
	debian_dir="$$pkg_root/DEBIAN"
	driver_target_dir="$$pkg_root/lib/modules/$$kernel_release/extra/ascend310b"
	autoload_conf_src="$(OUTPUT_DIR)/ascend310b-driver-modules.conf"
	autoload_conf_name="ascend310b-driver-modules-$$kernel_release.conf"
	autoload_conf_dir="$$pkg_root/etc/modules-load.d"
	deb_path="$(OUTPUT_DIR)/$${package_name}_$${package_version}_$${package_arch}.deb"
	printf 'start generate modules deb\n'
	rm -rf "$$deb_workspace"
	mkdir -p "$$pkg_root"
	cp -a "$(OUTPUT_DIR)/modules/." "$$pkg_root/"
	rm -rf "$$pkg_root/lib/modules/$$kernel_release/build" "$$pkg_root/lib/modules/$$kernel_release/source"
	mkdir -p "$$driver_target_dir"
	cp -a "$(OUTPUT_DIR)/driver_modules/." "$$driver_target_dir/"
	if [ ! -f "$$autoload_conf_src" ]; then
		find "$(OUTPUT_DIR)/driver_modules" -maxdepth 1 -type f -name '*.ko' -printf '%f\n' \
			| sed 's/\.ko$$//' | sort -u > "$$autoload_conf_src"
	fi
	mkdir -p "$$autoload_conf_dir"
	cp -f "$$autoload_conf_src" "$$autoload_conf_dir/$$autoload_conf_name"
	find "$$pkg_root/lib/modules/$$kernel_release" -maxdepth 1 -type f \
		\( -name 'modules.dep*' -o -name 'modules.alias*' -o -name 'modules.symbols*' \
		   -o -name 'modules.softdep' -o -name 'modules.weakdep' -o -name 'modules.devname' \) \
		-delete
	"$$depmod_bin" -b "$$pkg_root" -a "$$kernel_release"
	mkdir -p "$$debian_dir"
	printf '%s\n' \
		"Package: $${package_name}" \
		"Version: $${package_version}" \
		'Section: kernel' \
		'Priority: optional' \
		"Architecture: $${package_arch}" \
		"Maintainer: $${MODULES_DEB_MAINTAINER:-Ascend310B Build System <noreply@local>}" \
		"Description: Ascend310B kernel and driver modules for $${kernel_release}" \
		> "$$debian_dir/control"
	printf '%s\n' \
		'#!/bin/sh' \
		'set -e' \
		"if command -v depmod >/dev/null 2>&1; then depmod -a $${kernel_release} || true; fi" \
		'if command -v systemctl >/dev/null 2>&1 && [ -d /run/systemd/system ]; then systemctl restart systemd-modules-load.service || true; fi' \
		> "$$debian_dir/postinst"
	printf '%s\n' \
		'#!/bin/sh' \
		'set -e' \
		"if command -v depmod >/dev/null 2>&1; then depmod -a $${kernel_release} || true; fi" \
		> "$$debian_dir/postrm"
	chmod 0755 "$$debian_dir/postinst" "$$debian_dir/postrm"
	rm -f "$$deb_path"
	fakeroot dpkg-deb --build "$$pkg_root" "$$deb_path" >/dev/null
	printf 'generate %s success\n' "$$deb_path"

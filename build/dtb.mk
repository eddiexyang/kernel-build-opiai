.PHONY: dtb

dtb: | ensure-output-dirs
	printf 'start generate dt.img\n'
	rm -rf "$(DTB_WORKSPACE)"
	mkdir -p "$(DTB_WORKSPACE)"
	cd "$(DTB_WORKSPACE)"
	cmake "$(DTB_SOURCE_DIR)/dtbtool" -DTOP_DIR="$(ROOT_DIR)" -DPRODUCT="$(DTB_PRODUCT)"
	$(MAKE) device_dtb
	cp -f "$(DTB_WORKSPACE)/dt.img" "$(OUTPUT_DIR)"
	python3 "$(SIGNING_DIR)/esbc_header/esbc_header.py" \
		-raw_img "$(OUTPUT_DIR)/dt.img" \
		-out_img "$(OUTPUT_DIR)/dt.img" \
		-version "$(HEADER_VERSION)" \
		-nvcnt 0 \
		-tag dtimg \
		-platform hi1910Brc
	python3 "$(SIGNING_DIR)/image_pack/image_pack.py" \
		-raw_img "$(OUTPUT_DIR)/dt.img" \
		-out_img "$(OUTPUT_DIR)/dt.img" \
		-platform hi1910Brc \
		-version "$(HEADER_VERSION)"
	printf 'sign %s/dt.img success\n' "$(OUTPUT_DIR)"
	printf 'generate %s/dt.img success\n' "$(OUTPUT_DIR)"

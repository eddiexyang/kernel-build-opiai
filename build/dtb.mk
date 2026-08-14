.PHONY: dtb

dtb: | ensure-output-dirs
	printf 'start generate standalone device trees\n'
	dtc -I dts -O dtb \
		"$(DTB_SOURCE_DIR)/dts/hi1910b/hi1910BL/hi1910B-orangepiaipro20t.dts" \
		-o "$(OUTPUT_DIR)/hi1910B-orangepiaipro20t.dtb"
	dtc -I dts -O dtb \
		"$(DTB_SOURCE_DIR)/dts/hi1910b/hi1910BL/hi1910B-orangepiaipro8t.dts" \
		-o "$(OUTPUT_DIR)/hi1910B-orangepiaipro8t.dtb"
	printf 'generate %s/hi1910B-orangepiaipro20t.dtb success\n' "$(OUTPUT_DIR)"
	printf 'generate %s/hi1910B-orangepiaipro8t.dtb success\n' "$(OUTPUT_DIR)"

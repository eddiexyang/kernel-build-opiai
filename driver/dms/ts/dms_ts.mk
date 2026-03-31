ifeq ($(TOP_DIR),)
	ifeq ($(DAVINCI_HIAI_DKMS),y) #for DKMS
	endif
else #for CMake & ctrl cpu open
	ifeq ($(PRODUCT_SIDE), host)
	else
		drv_devmng-y += ts/dms_ts_info.o
		ifneq ($(filter $(PRODUCT), ascend910 ascend310 ascend310rc),)
			drv_devmng-y += ts/dms_ts_scan.o
		endif
	endif
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/ts
	EXTRA_CFLAGS += -DCFG_FEATURE_TS
endif
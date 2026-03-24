ifeq ($(TOP_DIR),)
	ifeq ($(DAVINCI_HIAI_DKMS),y) #for DKMS
	endif
else #for CMake
	ifeq ($(PRODUCT_SIDE), host)
	else
		ifneq ($(filter $(PRODUCT), ascend910B ascend920esl ascend920emu),)
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/hccs
			EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/drv_devmng/drv_devmng_device/ascend910
			drv_devmng-y += hccs/dms_hccs_feature.o
			EXTRA_CFLAGS += -DCFG_FEATURE_GET_PCS_BITMAP_BY_BOARD_TYPE
		endif
	endif
endif

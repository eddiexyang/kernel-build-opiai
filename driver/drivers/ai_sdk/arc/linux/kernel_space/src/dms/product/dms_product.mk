ifeq ($(TOP_DIR),)
	ifeq ($(DAVINCI_HIAI_DKMS),y) #for DKMS
		drv_devmng_host-y += product/dms_product.o
		drv_devmng_host-y += product/dms_product_host.o
		EXTRA_CFLAGS += -DCFG_FEATURE_DMS_PRODUCT_HOST
		ifeq ($(TARGET_PRODUCT),cloud)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
		endif
		EXTRA_CFLAGS += -I$(HIAI_DKMS_DIR)/dms/product
	endif
else #for CMake
	ifeq ($(PRODUCT_SIDE), host)
		drv_devmng_host-y += product/dms_product.o
		drv_devmng_host-y += product/dms_product_host.o
		EXTRA_CFLAGS += -DCFG_FEATURE_DMS_PRODUCT_HOST
	else
		drv_devmng-y += product/dms_product.o
	endif
	EXTRA_CFLAGS += -I$(DRIVER_KERNEL_DIR)/src/dms/product
endif

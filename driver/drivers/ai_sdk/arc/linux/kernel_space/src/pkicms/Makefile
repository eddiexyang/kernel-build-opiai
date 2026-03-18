ifeq ($(TOP_DIR),)
KERNEL_DIR :=$(srctree)
	EXTRA_CFLAGS += -I$(PWD)/inc/driver
	EXTRA_CFLAGS += -I$(PWD)/libc_sec/include
	EXTRA_CFLAGS += -I$(PWD)/os/itrustee/tzdriver/libhwsecurec
	EXTRA_CFLAGS += -I$(PWD)/inc/hiss
	EXTRA_CFLAGS += -I$(PWD)/drivers/dev_inc
	EXTRA_CFLAGS += -DLINUX_KERNEL_BUILD
	ifeq ($(TARGET_PRODUCT),cloud)
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/asn1
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/cms
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/cmscbb_common
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/include
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/pki
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/src
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/x509
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
    EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
	else ifeq ($(TARGET_PRODUCT),mdc)
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/asn1
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/cms
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/cmscbb_common
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/include
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/pki
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/src
	EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/x509
	EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
	else ifeq ($(TARGET_PRODUCT),mini)
	ifneq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
		EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/asn1
		EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/cms
		EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/cmscbb_common
		EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/include
		EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/pki
		EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/src
		EXTRA_CFLAGS += -I$(PWD)/drivers/cmscbb/x509
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINI
	else
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
        EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
	endif
	endif

	ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef -Wstack-usage=2048 $(WDATE_TIME)
	ccflags-y += -Wno-unused-parameter -Wno-sign-compare
obj-m += drv_pkicms.o

	drv_pkicms-objs := pkicms_module.o pkicms/cmscbb.o pkicms/pkicms_common.o pkicms/soc_verify.o
	ifeq ($(TARGET_PRODUCT),cloud)
		drv_pkicms-objs += cmscbb/cms_verify.o cmscbb/cmscbb_asn1_decode.o cmscbb/cmscbb_asn1_def.o cmscbb/cmscbb_templ.o cmscbb/cmscbb_crl_compare.o cmscbb/cmscbb_pem_util.o cmscbb/cmscbb_asn1_utils.o cmscbb/cmscbb_base64.o cmscbb/cmscbb_buf.o
		drv_pkicms-objs += cmscbb/cmscbb_cms_parse.o cmscbb/cmscbb_list.o cmscbb/cmscbb_pki.o cmscbb/cmscbb_plt_proxy.o cmscbb/cmscbb_x509.o
		drv_pkicms-objs += pkicms/pkicms.o pkicms/pkicms_api.o
	else ifeq ($(TARGET_PRODUCT),mdc)
		drv_pkicms-objs += pkicms/pkicms.o pkicms/pkicms_api.o
		drv_pkicms-objs += pkicms/drv_cpu_type.o
	else ifeq ($(TARGET_PRODUCT),mini)
		ifneq ($(TARGET_CHIP_ID), $(ASCEND_FLORENCE_DC_V10))
			drv_pkicms-objs += cmscbb/cms_verify.o cmscbb/cmscbb_asn1_decode.o cmscbb/cmscbb_asn1_def.o cmscbb/cmscbb_templ.o cmscbb/cmscbb_crl_compare.o cmscbb/cmscbb_pem_util.o cmscbb/cmscbb_asn1_utils.o cmscbb/cmscbb_base64.o cmscbb/cmscbb_buf.o
			drv_pkicms-objs += cmscbb/cmscbb_cms_parse.o cmscbb/cmscbb_list.o cmscbb/cmscbb_pki.o cmscbb/cmscbb_plt_proxy.o cmscbb/cmscbb_x509.o
			drv_pkicms-objs += pkicms/pkicms.o pkicms/pkicms_api.o
		endif
	endif
else
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/driver
	EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
	EXTRA_CFLAGS += -I$(TOP_DIR)/os/itrustee/tzdriver/libhwsecurec
	EXTRA_CFLAGS += -I$(TOP_DIR)/inc/hiss
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/firmware/hiss/inc/hiss
	EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc
	EXTRA_CFLAGS += -DLINUX_KERNEL_BUILD

	ifeq ($(PRODUCT),ascend910)
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/asn1
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/cms
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/cmscbb_common
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/include
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/pki
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/src
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/x509
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
		EXTRA_CFLAGS += -DCFG_FEATURE_PKCS_SIGN
		EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
		EXTRA_CFLAGS += -DCMSCBB_SUPPORT_RSAPSS=0
		EXTRA_CFLAGS += -DCFG_FEATURE_RSA_2048
		EXTRA_CFLAGS += -DCFG_FEATURE_RSA_4096
	else ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1),)
		ifneq ($(filter $(PRODUCT),  ascend610Lite ),)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
		endif
		ifneq ($(filter $(PRODUCT),  ascend610Liteesl ),)
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_LITE_ESL
		endif
		ifneq ($(filter $(PRODUCT),  as31xm1 ),)
			EXTRA_CFLAGS += -DCFG_FEATURE_NOT_SUPPORT_SOC_VERIFY
			EXTRA_CFLAGS += -DCFG_FEATURE_GET_ENABLE_FLAG
			EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V11
			EXTRA_CFLAGS += -DCFG_FEATURE_SIGN_VERSION_1
		endif
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/asn1
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/cms
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/cmscbb_common
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/include
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/pki
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/src
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/x509
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/common
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MDC_V51
		EXTRA_CFLAGS += -DCMSCBB_SUPPORT_RSAPSS=1
		EXTRA_CFLAGS += -DCFG_FEATURE_PKCS_SIGN
		EXTRA_CFLAGS += -DCFG_FEATURE_RSA_2048
		EXTRA_CFLAGS += -DCFG_FEATURE_RSA_4096
	else ifneq ($(filter $(PRODUCT),  ascend310p helper310p ascend310B ascend310Besl ascend310Bemu ascend310Brc ascend310Brcesl ascend310Brcemu as31xm1),)
		ifeq ($(PRODUCT),ascend310Besl)
			EXTRA_CFLAGS += -DCFG_ENV_ESL
		endif
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_MINIV2
		EXTRA_CFLAGS += -DCFG_FEATURE_PKCS_SIGN
		EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
		EXTRA_CFLAGS += -DCMSCBB_SUPPORT_RSAPSS=0
		EXTRA_CFLAGS += -DCFG_FEATURE_RSA_2048
		EXTRA_CFLAGS += -DCFG_FEATURE_RSA_4096
	else ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu),)
		ifeq ($(PRODUCT),ascend920esl)
			EXTRA_CFLAGS += -DCFG_ENV_ESL
		endif
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD_V2
		EXTRA_CFLAGS += -DCFG_FEATURE_PSS_SIGN
		EXTRA_CFLAGS += -DCMSCBB_SUPPORT_RSAPSS=0
		EXTRA_CFLAGS += -DCFG_FEATURE_SIGN_VERSION_1
		EXTRA_CFLAGS += -DCFG_FEATURE_RSA_4096
	else
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/asn1
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/cms
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/cmscbb_common
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/include
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/pki
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/src
		EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/cmscbb/x509
		EXTRA_CFLAGS += -DCFG_SOC_PLATFORM_CLOUD
		EXTRA_CFLAGS += -DCMSCBB_SUPPORT_RSAPSS=0
		EXTRA_CFLAGS += -DCFG_FEATURE_PKCS_SIGN
		EXTRA_CFLAGS += -DCFG_FEATURE_RSA_2048
		EXTRA_CFLAGS += -DCFG_FEATURE_RSA_4096
	endif

	ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef -Wstack-usage=2048 $(WDATE_TIME)
	ccflags-y += -Wno-unused-parameter -Wno-sign-compare
obj-m += drv_pkicms.o

	drv_pkicms-objs := pkicms_module.o pkicms/cmscbb.o pkicms/pkicms_common.o pkicms/soc_verify.o
	ifeq ($(PRODUCT),ascend910)
		drv_pkicms-objs += cmscbb/cms_verify.o cmscbb/cmscbb_asn1_decode.o cmscbb/cmscbb_asn1_def.o cmscbb/cmscbb_templ.o cmscbb/cmscbb_crl_compare.o cmscbb/cmscbb_pem_util.o cmscbb/cmscbb_asn1_utils.o cmscbb/cmscbb_base64.o cmscbb/cmscbb_buf.o
		drv_pkicms-objs += cmscbb/cmscbb_cms_parse.o cmscbb/cmscbb_list.o cmscbb/cmscbb_pki.o cmscbb/cmscbb_plt_proxy.o cmscbb/cmscbb_x509.o
		drv_pkicms-objs += pkicms/pkicms.o pkicms/pkicms_api.o
	else ifneq ($(filter $(PRODUCT),  ascend610 bs9sx1a ascend610Lite ascend610Liteesl as31xm1),)
		drv_pkicms-objs += pkicms/pkicms.o pkicms/pkicms_api.o
		drv_pkicms-objs += pkicms/drv_cpu_type.o
	else ifneq ($(filter $(PRODUCT),  ascend910B ascend920esl ascend920emu ascend310p helper310p ascend310B ascend310Besl ascend310Bemu ascend310Brc as31xm1 ascend310Brcesl ascend310Brcemu),)
	else
		drv_pkicms-objs += cmscbb/cms_verify.o cmscbb/cmscbb_asn1_decode.o cmscbb/cmscbb_asn1_def.o cmscbb/cmscbb_templ.o cmscbb/cmscbb_crl_compare.o cmscbb/cmscbb_pem_util.o cmscbb/cmscbb_asn1_utils.o cmscbb/cmscbb_base64.o cmscbb/cmscbb_buf.o
		drv_pkicms-objs += cmscbb/cmscbb_cms_parse.o cmscbb/cmscbb_list.o cmscbb/cmscbb_pki.o cmscbb/cmscbb_plt_proxy.o cmscbb/cmscbb_x509.o
		drv_pkicms-objs += pkicms/pkicms.o pkicms/pkicms_api.o
	endif
endif

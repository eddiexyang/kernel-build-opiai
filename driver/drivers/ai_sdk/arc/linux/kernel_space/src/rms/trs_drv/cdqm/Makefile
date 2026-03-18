obj-y += device/
obj-y += host/
obj-y += comm/

ifeq ($(TARGET_PRODUCT),cloud)
	ccflags-y += -Wall -Werror -Wtrampolines $(WDATE_TIME) -Wfloat-equal -Wvla -Wundef -funsigned-char  -Wformat=2 -Wstack-usage=2048 -Wcast-align
	ccflags-y += -fno-common -Wstrict-prototypes -Wframe-larger-than=2048 -Wcast-qual
	ccflags-y += -Wextra -Wno-cast-qual -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-empty-body
endif


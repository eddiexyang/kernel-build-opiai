ccflags-y += -Wall -Werror -funsigned-char -Wextra -Wformat=2 -Wfloat-equal -Wcast-align -Wvla -Wundef $(WDATE_TIME)
ccflags-y += -Wstack-usage=2048 -Wno-unused-parameter -Wno-sign-compare
ifneq ($(NOT_SUPPORT_SP), y)
    EXTRA_CFLAGS += -fstack-protector-all
endif

obj-m += drv_sys_ctrl_uniform.o
drv_sys_ctrl_uniform-objs := drv_sys_ctrl.o

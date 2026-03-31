# Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
# Description: make file of sysmonitor
# Author: tangyizhou
# Create: 2018-12-15

ifeq ($(TOP_DIR),) 
KERNEL_DIR :=$(srctree)
obj-m += sysmonitor.o
sysmonitor-objs := sysmonitor_main.o signo_catch.o
EXTRA_CFLAGS += -Wall -Werror
else
obj-m += sysmonitor.o
sysmonitor-objs := sysmonitor_main.o signo_catch.o
EXTRA_CFLAGS += -Wall -Werror
EXTRA_CFLAGS += -I$(TOP_DIR)/abl/libc_sec/include
endif

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) clean

.PHONY: modules modules_install clean

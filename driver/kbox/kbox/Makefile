KERNEL_DIR := $(srctree)
EXTRA_CFLAGS += -I$(TOP_DIR)/drivers/kernel/kbox/kbox -I$(KERNEL_DIR)
EXTRA_CFLAGS += -fsigned-char -pipe
obj-$(CONFIG_KBOX_CORE) += kbox.o
kbox-objs := kbox_common_interface.o kbox_die.o kbox_emerge.o kbox_find_sym_addr.o kbox_main.o kbox_oom.o kbox_panic.o kbox_printk_buf.o kbox_reboot.o kbox_reboot_timer.o kbox_rlock.o

ifeq ($(ARCH),x86_64)
kbox-objs += kbox_x86_64.o
endif
ifeq ($(ARCH),arm64)
kbox-objs += kbox_arm64.o
endif

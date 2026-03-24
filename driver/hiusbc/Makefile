ccflags-y += -Wall -Werror -Wno-implicit-fallthrough
EXTRA_CFLAGS += -I$(src)/include

obj-m += drv_hiusbc.o
drv_hiusbc-objs += \
	hiusbc_core.o \
	hiusbc_debugfs_common.o \
	hiusbc_debugfs.o \
	hiusbc_drd.o \
	hiusbc_setup.o \
	hiusbc_transfer.o \
	hiusbc_event.o \
	hiusbc_gadget.o \
	hiusbc_host.o \
	hiusbc_asic_config.o

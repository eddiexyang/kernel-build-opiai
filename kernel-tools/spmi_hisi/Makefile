#
# Makefile for kernel SPMI framework.
#

KCOV_INSTRUMENT := n

ccflags-y += -I$(src)/include

ifeq ($(CONFIG_HISI_SPMI),y)
ccflags-y += -DCONFIG_HISI_SPMI
endif
ifeq ($(CONFIG_HISI_SPMI),m)
ccflags-y += -DCONFIG_HISI_SPMI_MODULE
endif

ifeq ($(CONFIG_HISI_SPMI_DEBUG_FS),y)
ccflags-y += -DCONFIG_HISI_SPMI_DEBUG_FS
endif
ifeq ($(CONFIG_HISI_SPMI_DEBUG_FS),m)
ccflags-y += -DCONFIG_HISI_SPMI_DEBUG_FS_MODULE
endif

ifeq ($(CONFIG_HISI_SPMI_PMIC_VOLTAGE),y)
ccflags-y += -DCONFIG_HISI_SPMI_PMIC_VOLTAGE
endif
ifeq ($(CONFIG_HISI_SPMI_PMIC_VOLTAGE),m)
ccflags-y += -DCONFIG_HISI_SPMI_PMIC_VOLTAGE_MODULE
endif

obj-$(CONFIG_HISI_SPMI)	+= hisi-spmi-core.o

obj-$(CONFIG_HISI_SPMI)	+= hisi-spmi-controller.o

obj-$(CONFIG_HISI_SPMI)	+= of_hisi_spmi.o

obj-$(CONFIG_HISI_SPMI)	+= hisi_pmic_spmi.o

obj-$(CONFIG_HISI_SPMI)	+= hisi_pmu_tsensor.o

obj-$(CONFIG_HISI_SPMI_DEBUG_FS)	+= hisi-spmi-dbgfs.o

obj-$(CONFIG_HISI_SPMI_PMIC_VOLTAGE)	+= hisi_pmic_volt_inquire.o

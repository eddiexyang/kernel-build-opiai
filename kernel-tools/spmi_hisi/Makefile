#
# Makefile for kernel SPMI framework.
#

KCOV_INSTRUMENT := n

obj-$(CONFIG_HISI_SPMI)	+= hisi-spmi-core.o

obj-$(CONFIG_HISI_SPMI)	+= hisi-spmi-controller.o

obj-$(CONFIG_HISI_SPMI)	+= of_hisi_spmi.o

obj-$(CONFIG_HISI_SPMI)	+= hisi_pmic_spmi.o

obj-$(CONFIG_HISI_SPMI)	+= hisi_pmu_tsensor.o

obj-$(CONFIG_HISI_SPMI_DEBUG_FS)	+= hisi-spmi-dbgfs.o

obj-$(CONFIG_HISI_SPMI_PMIC_VOLTAGE)	+= hisi_pmic_volt_inquire.o

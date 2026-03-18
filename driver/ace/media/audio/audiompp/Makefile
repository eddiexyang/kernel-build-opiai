KERNEL_DIR := $(srctree)
OSTYPE := linux
# all source file in this module
EXTRA_CFLAGS += -I${TOP_DIR}/inc \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/inc \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/include \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/sensor \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/event \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/sensor/config \
                -I$(TOP_DIR)/drivers/ai_sdk/arc/linux/kernel_space/src/dms/config \
                -I${TOP_DIR}/inc/driver \
                -I$(TOP_DIR)/abl/libc_sec/include \
                -I${TOP_DIR}/ace/media/mediabase/src/base/mkp/include \
                -I${TOP_DIR}/ace/media/mediabase/src/base/arch/arch11/include \
                -I${TOP_DIR}/ace/media/mediabase/src/src/base/arch/include \
                -I${TOP_DIR}/ace/media/mediabase/src/sys/mkp/include \
                -I${TOP_DIR}/ace/media/mediabase/src/sys/arch/arch11/include \
                -I${TOP_DIR}/ace/media/mediabase/src/sys/arch/include \
                -I${TOP_DIR}/ace/media/mediabase/src/base/utils/memory/algo \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/include \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/h8_include/base/arch/archv200/include \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/inc/external/mediabase \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/h8_include/include \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/h8_include/sys/include \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/h8_include/osal/include \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/h8_include/base/include \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/osal \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/sys/ext_inc \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/sys/adapt \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/base/adapt \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/base/ext_inc \
                -I${TOP_DIR}/ace/media/mediabase/inc/external/mediabase \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/include/adapt \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/base/ext_inc \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/include/adapt \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/include/h8/inner/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/include/h8/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/include/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/ext_inc/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/arch/include \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/mkp/ai \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/mkp/ao/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/mkp/aenc/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/mkp/adec/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/mkp/comm/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/acodec/v900/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/acodec/v900/adapt/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/init/v200/linux/include/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/arch/v200/ \
                -I$(TOP_DIR)/ace/media/audio/audiompp/audio/arch/v200/include/ \
                -I$(TOP_DIR)/ace/media/mediabase/inc/mediabase/h8_include/include/ \
                -I${TOP_DIR}/ace/media/mediabase/inc/mediabase/audio/

EXTRA_CFLAGS += -Wno-error=date-time
EXTRA_CFLAGS += -Wshadow
EXTRA_CFLAGS += -Wformat=2
EXTRA_CFLAGS += -Wextra
EXTRA_CFLAGS += -Wcast-qual
EXTRA_CFLAGS += -Wconversion
EXTRA_CFLAGS += -Wdelete-non-virtual-dtor
EXTRA_CFLAGS += -Wdate-time
EXTRA_CFLAGS += -Wvla
EXTRA_CFLAGS += -Wfloat-equal
EXTRA_CFLAGS += -Wunused
EXTRA_CFLAGS += -Wnon-virtual-dtor
EXTRA_CFLAGS += -Wcast-align
EXTRA_CFLAGS += -Wtrampolines
EXTRA_CFLAGS += -Wswitch-default
EXTRA_CFLAGS += -Wstrict-prototypes
EXTRA_CFLAGS += -Wundef
EXTRA_CFLAGS += -Woverloaded-virtual
EXTRA_CFLAGS += -Wall
EXTRA_CFLAGS += -funsigned-char
EXTRA_CFLAGS += -fstrong-eval-order
EXTRA_CFLAGS += -freg-struct-return
EXTRA_CFLAGS += -fvisibility=hidden
EXTRA_CFLAGS += -pipe
EXTRA_CFLAGS += -DBUILD_MODULE_AUDIO
EXTRA_CFLAGS += -fstack-protector-all

obj-m += drv_ai.o \
         drv_ao.o \
         drv_aio.o \
         drv_adec.o \
         drv_aenc.o \
         drv_acodec.o

# drv_ai
drv_ai-objs += audio/mkp/ai/ai.o\
        audio/mkp/ai/ai_trans.o\
        audio/mkp/comm/audio_fb.o\
        audio/mkp/comm/aio_pub.o \
        audio/arch/common/ai_drv.o \
        audio/mkp/ai/ai_proc_h7.o \
        audio/arch/common/ai_drv_proc_h7.o \
        audio/init/v200/linux/src/ai_init.o \
        audio/arch/v200/hi19xx_i2s.o

drv_ao-objs += audio/mkp/ao/ao.o\
        audio/mkp/ao/ao_trans.o\
        audio/mkp/comm/aio_pub.o\
        audio/arch/common/ao_drv.o \
        audio/mkp/ao/ao_proc_h7.o \
        audio/arch/common/ao_drv_proc_h7.o \
        audio/init/v200/linux/src/ao_init.o \
        audio/arch/v200/hi19xx_i2s.o

drv_aio-objs += audio/arch/common/aiao_hal_comm.o \
        audio/arch/v200/hal/audio/aiao_hal.o \
        audio/init/v200/linux/src/aio_init.o \
        audio/arch/v200/asp_misc.o \
        audio/arch/v200/pid_protect.o \
        audio/arch/v200/asp_dma.o \
        audio/arch/v200/hi19xx_i2s.o \
        audio/arch/common/audio_fault.o \
        audio/arch/v200/asp_dma_hal.o

drv_acodec-objs += audio/init/v200/linux/src/acodec_init.o \
                   audio/mkp/acodec/V900/acodec.o

drv_adec-objs += audio/mkp/adec/adec.o \
        audio/mkp/adec/adec_proc_h8.o \
        audio/mkp/comm/audio_codec_pub.o \
        audio/init/v200/linux/src/adec_init.o

drv_aenc-objs += audio/init/v200/linux/src/aenc_init.o \
         audio/mkp/comm/audio_fb.o \
         audio/mkp/comm/audio_codec_pub.o \
         audio/arch/common/aenc_drv.o \
         audio/mkp/aenc/aenc.o \
         audio/mkp/aenc/aenc_proc_h8.o

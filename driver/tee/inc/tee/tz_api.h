/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
* Description: tz api file
* Author: huawei
* Create: 2019/06/18
*/
#ifndef TZ_API_H
#define TZ_API_H

#include <linux/types.h>

#define IMAGE_TEE                 (0x00000000)
#define IMAGE_LPM3                (0x00000001)
#define IMAGE_KERNEL              (0x00000100)
#define IMAGE_DTS                 (0x00000101)
#define IMAGE_INITRD              (0x00000200)

#define TEEC_SUCCESS              0x00000000
#define TEEC_INVALID_PARAMETERS   0xFFFFFFFF
#define TEEC_ERROR_BAD_PARAMETERS 0xFFFF0006

#define SUBKEY_ID_NUM_MASK (0x1F)
#define ROOT_PUBKEY_LEN (1024)
#define RIM_CRL_REQ_SIZE (544)

#define IMG_VER_MAGIC 0x5a5aa5a5

typedef struct {
    u32 flag;
    u32 NvCnt;
} img_ver_tail;

typedef enum {
    PG_MODULE_TYPE_CPU,
    PG_MODULE_TYPE_AIC,
    PG_MODULE_TYPE_AIV,
    PG_MODULE_TYPE_HBM,
    PG_MODULE_TYPE_MAX = PG_MODULE_TYPE_HBM,
    PG_MODULE_TYPE_INVALID,
} module_type;

typedef enum {
    PG_DATA_TYPE_FREQ,
    PG_DATA_TYPE_TOTAL_NUM,
    PG_DATA_TYPE_CORE_MAP,
    PG_DATA_TYPE_NUM_LEVEL,
    PG_DATA_TYPE_FREQ_LEVEL,
    PG_DATA_TYPE_MAX = PG_DATA_TYPE_FREQ_LEVEL,
    PG_DATA_TYPE_INVALID,
} data_type;

typedef struct {
    module_type module;
    data_type data;
} pg_cmd_data;

void tz_sec_mutex_init(void);
void tz_sec_mutex_uninit(void);
u32 verify_init_session(int dev_id);
void verify_finalize_session(int dev_id);
u32 ini_cms_verify_kernel(int dev_id, const u8 *ini_buf, u32 ini_len, u32 img_id,
                          const u8 *cms_cert_buf, u32 cms_cert_len,
                          const u8 *crl_buf, u32 crl_len);
u32 img_hash_check_kernel(int dev_id, const u8 *img_buf, const u32 img_len, const u32 img_id);
int sec_update_verification(int dev_id, const u8 *image_head_base,
    u32 image_head_offset, const u8 *image_position, u32 size, u32 pss_cfg);

int sec_rim_data(int dev_id, const u8 *rot_data, u32 rot_len,
    const u8 *rim_data, u32 rim_len);
int sec_update_cnt(int dev_id);
int sec_read_pg_info(int dev_id, pg_cmd_data cmd, u64 *out_arg);
int soc_get_nvcnt(uint32_t dev_id, uint32_t *buff, uint32_t size);

#endif

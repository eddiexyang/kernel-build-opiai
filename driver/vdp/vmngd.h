/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */
#ifndef DVPP_COMM_VMGD_H
#define DVPP_COMM_VMGD_H

#include "hi_type.h"

struct vmngd_client_instance;

hi_s32 load_vmng_func(hi_void);
hi_void unload_vmng_func(hi_void);

typedef struct {
    hi_s32 (*dvpp_vdec_calc_init)(struct vmngd_client_instance *instance);
    hi_s32 (*dvpp_vpc_calc_init)(struct vmngd_client_instance *instance);
    hi_s32 (*dvpp_pngd_calc_init)(struct vmngd_client_instance *instance);
    hi_s32 (*dvpp_venc_calc_init)(struct vmngd_client_instance *instance);
    hi_s32 (*dvpp_vdec_calc_uninit)(struct vmngd_client_instance *instance);
    hi_s32 (*dvpp_vpc_calc_uninit)(struct vmngd_client_instance *instance);
    hi_s32 (*dvpp_pngd_calc_uninit)(struct vmngd_client_instance *instance);
    hi_s32 (*dvpp_venc_calc_uninit)(struct vmngd_client_instance *instance);
    hi_s32 (*dvpp_vdec_calc_update_vf_info)(hi_u32 device_id, hi_u32 vf_id);
    hi_s32 (*dvpp_vpc_calc_update_vf_info)(hi_u32 device_id, hi_u32 vf_id);
    hi_s32 (*dvpp_venc_calc_update_vf_info)(hi_u32 device_id, hi_u32 vf_id);
} dvpp_vf_calc;

extern dvpp_vf_calc* sys_get_dvpp_vf_calc(hi_void);

hi_s32 dvpp_calc_get_vf_id(hi_u32 logic_dev_id, hi_u32 *ext_phy_dev_id, hi_u32 *vf_id, hi_bool is_host_mode);
hi_s32 dvpp_calc_dev_id_to_pf_vf_id(hi_u32 ext_phy_dev_id, hi_u32 ext_vf_id, hi_u32 *pf_id, hi_u32 *vf_id);
hi_s32 dvpp_calc_dev_id_to_pf_vf_id_ex(hi_u32 ext_phy_dev_id, hi_u32 ext_vf_id, hi_u32 *pf_id, hi_u32 *vf_id);
hi_u32 dvpp_calc_get_drv_required_dev_id(hi_u32 ext_phy_dev_id, hi_u32 dev_id);
hi_u32 dvpp_calc_get_mem_alloc_required_dev_id(hi_u32 ext_phy_dev_id, hi_u32 dev_id);
#endif // #ifndef DVPP_COMM_VMGD_H
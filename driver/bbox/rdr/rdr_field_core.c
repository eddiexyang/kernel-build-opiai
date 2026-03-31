/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2022-08-13
 */

#include "rdr_field_core.h"

#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/securec.h>
#include "bbox_product.h"
#include "bootparam/bbox_boot_param.h"
#include "communication/bbox_channel_status.h"
#include "common/bbox_common.h"
#include "rdr_common.h"
#include "rdr_core.h"

STATIC struct rdr_struct_s *g_pbb = NULL;
STATIC struct rdr_struct_s *g_tmp_pbb = NULL;

/*
 * @brief       : get pbb
 * @return      : static reserved memory
 */
struct rdr_struct_s *rdr_field_get_pbb(void)
{
    return g_pbb;
}

/*
 * @brief       : get tmp pbb
 * @return      : dynamic request memory
 */
struct rdr_struct_s *rdr_field_get_tmppbb(void)
{
    return g_tmp_pbb;
}

/*
 * @brief       : clear tmppbb
 * @return      : NA
 */
void rdr_field_clear_tmppbb(void)
{
    if (g_tmp_pbb != NULL) {
        vfree(g_tmp_pbb);
        g_tmp_pbb = NULL;
    }
}

/*
 * @brief       : clear pbb
 * @return      : NA
 */
static inline void rdr_field_clear_pbb(void)
{
    if (g_pbb != NULL) {
        rdr_unmap(g_pbb);
        g_pbb = NULL;
    }
}

/*
 * @brief       : clear tmppbb & pbb
 * @return      : NA
 */
static inline void rdr_field_clear_data(void)
{
    rdr_field_clear_tmppbb();
    rdr_field_clear_pbb();
}

/*
 * @brief       : get pbb size
 * @return      : pbb size
 */
u64 rdr_field_get_pbb_size(void)
{
    u64 size = 0;
    s32 ret = bbox_config_get_rdr(NULL, &size);
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return 0, "get rdr phymem size failed.\n");
    return size;
}

/*
 * @brief       : get area info
 * @param [in]  : u8 coreid                             core id
 * @param [in]  : struct bbox_module_result *retinfo    return info
 * @return      : != 0 fail; = 0 success
 */
s32 rdr_field_get_areainfo(u8 coreid, struct bbox_module_result *retinfo)
{
    s32 i;

    BB_CHECK_PTR(retinfo, return BBOX_FAILURE, "invalid parameter, retinfo is NULL.\n");
    for (i = 0; i < (s32)BBOX_AREA_MAXIMUM; i++) {
        if (coreid == g_pbb->area_info[i].coreid) {
            retinfo->log_len = g_pbb->area_info[i].length;
            retinfo->log_addr = g_pbb->area_info[i].offset;
            return BBOX_SUCCESS;
        }
    }

    retinfo->log_len = 0;
    retinfo->log_addr = 0;
    return BBOX_FAILURE;
}

/*
 * @brief       : save base info args
 * @param [in]  : u32 excepid               exception id
 * @param [in]  : struct bbox_time *tm      timestamp
 * @param [in]  : u32 devid                 device id
 * @param [in]  : u32 arg                   arg
 * @return      : NA
 */
void rdr_field_save_args(u32 excepid, const struct bbox_time *tm, u32 devid, u32 arg)
{
    g_pbb->base_info.excepid = excepid;
    g_pbb->base_info.devid = devid;
    g_pbb->base_info.arg = arg;
    if (tm != NULL) {
        g_pbb->base_info.tm.tv_sec = tm->tv_sec;
        g_pbb->base_info.tm.tv_nsec = tm->tv_nsec;
        bbox_get_date(tm, g_pbb->base_info.date, DATATIME_MAXLEN);
    } else {
        g_pbb->base_info.tm.tv_sec = 0;
        g_pbb->base_info.tm.tv_nsec = 0;
        g_pbb->base_info.date[0] = '\0';
    }
}

/*
 * @brief       : save exception data of base info
 * @param [in]  : struct bbox_exception_info *e
 * @return      : != 0 fail; = 0 success
 */
void rdr_field_save_edata(const struct bbox_exception_info *e)
{
    s32 ret;

    BB_CHECK_PTR(e, return, "invalid param, e is NULL.\n");
    g_pbb->base_info.e_core = e->e_from_core;
    g_pbb->base_info.e_type = e->e_exce_type;
    ret = memcpy_s(g_pbb->base_info.e_module, BBOX_MODULE_NAME_LEN, e->e_from_module, BBOX_MODULE_NAME_LEN);
    BB_CHECK_MEMCPY(ret, return);
    ret = memcpy_s(g_pbb->base_info.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, e->e_desc, BBOX_EXCEPTIONDESC_MAXLEN);
    BB_CHECK_MEMCPY(ret, return);
}

/*
 * @brief       : procexec done, write flag
 * @return      : NA
 */
void rdr_field_procexec_done(void)
{
    g_pbb->base_info.start_flag = RDR_PROC_EXEC_DONE;
}

/*
 * @brief       : communication notify callback function
 * @param [in]  : struct bbox_notifier_block *nb    notifier block
 * @param [in]  : unsigned long event               event
 * @param [in]  : void *args                        args
 * @return      : =0: success <0: failure
 */
STATIC s32 rdr_field_comm_notify(struct bbox_notifier_block *nb, unsigned long event, void *args)
{
    u32 device_id;
    const struct bbox_callback_info *info = NULL;

    BB_CHECK_PTR(args, return BBOX_FAILURE, "invalid param, args is NULL.\n");
    UNUSED(nb);
    UNUSED(event);

    info = (struct bbox_callback_info *)args;
    device_id = info->devid;
    if ((device_id >= bbox_get_device_num()) ||
        (info->new_status >= CHANNEL_STATUS_MAX)) {
        BB_PRINT_ERR("invalid argument, devid[%u], status[%d]", device_id, (s32)info->new_status);
        return BBOX_FAILURE;
    }
    g_pbb->base_info.comm_flag[device_id] = (u8)info->new_status;
    return BBOX_SUCCESS;
}

STATIC struct bbox_notifier_block g_rdr_field_comm_block = {
    .s_list = {NULL, NULL},
    .notifier_call = rdr_field_comm_notify
};

/*
 * @brief       : init base info data
 * @return      : NA
 */
STATIC void rdr_field_baseinfo_init(void)
{
    s32 i, ret;

    g_pbb->base_info.excepid = 0;
    g_pbb->base_info.devid = 0;
    g_pbb->base_info.arg = 0;
    g_pbb->base_info.e_core = 0;
    g_pbb->base_info.e_type = 0;
    g_pbb->base_info.tm.tv_sec = 0;
    g_pbb->base_info.tm.tv_nsec = 0;
    ret = memset_s(g_pbb->base_info.date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);
    BB_CHECK_MEMSET(ret, return);
    ret = memset_s(g_pbb->base_info.e_module, BBOX_MODULE_NAME_LEN, 0, BBOX_MODULE_NAME_LEN);
    BB_CHECK_MEMSET(ret, return);
    ret = memset_s(g_pbb->base_info.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, 0, BBOX_EXCEPTIONDESC_MAXLEN);
    BB_CHECK_MEMSET(ret, return);

    g_pbb->base_info.start_flag = 0;
    g_pbb->base_info.reboot_flag = 0;
    for (i = 0; i < (s32)DEVICE_MAX_NUM; i++) {
        g_pbb->base_info.comm_flag[i] = CHANNEL_STATUS_INIT;
    }

    g_pbb->core_base_info.excepid = 0;
    g_pbb->core_base_info.devid = 0;
    g_pbb->core_base_info.arg = 0;
    g_pbb->core_base_info.e_core = 0;
    g_pbb->core_base_info.e_type = 0;
    g_pbb->core_base_info.tm.tv_sec = 0;
    g_pbb->core_base_info.tm.tv_nsec = 0;
    ret = memset_s(g_pbb->core_base_info.date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);
    BB_CHECK_MEMSET(ret, return);
    ret = memset_s(g_pbb->core_base_info.e_module, BBOX_MODULE_NAME_LEN, 0, BBOX_MODULE_NAME_LEN);
    BB_CHECK_MEMSET(ret, return);
    ret = memset_s(g_pbb->core_base_info.e_desc, BBOX_EXCEPTIONDESC_MAXLEN, 0, BBOX_EXCEPTIONDESC_MAXLEN);
    BB_CHECK_MEMSET(ret, return);

    g_pbb->core_base_info.start_flag = 0;
    g_pbb->core_base_info.reboot_flag = 0;
    for (i = 0; i < (s32)DEVICE_MAX_NUM; i++) {
        g_pbb->core_base_info.comm_flag[i] = CHANNEL_STATUS_INIT;
    }
    (void)bbox_register_channel_notifier(&g_rdr_field_comm_block);
    return;
}

/*
 * @brief       : reinit base info data
 * @return      : NA
 */
void rdr_field_baseinfo_reinit(void)
{
    g_pbb->base_info.excepid = 0;
    g_pbb->base_info.devid = 0;
    g_pbb->base_info.arg = 0;
    g_pbb->base_info.e_core = 0;
    g_pbb->base_info.e_type = 0;
    g_pbb->base_info.start_flag = RDR_PROC_EXEC_START;
    g_pbb->base_info.tm.tv_sec = 0;
    g_pbb->base_info.tm.tv_nsec = 0;
    g_pbb->base_info.date[0] = '\0';
    return;
}

static bool rdr_field_need_clear(u8 coreid)
{
    s32 i;
    const u8 list[] = {BBOX_BIOS, BBOX_LPM, BBOX_LPFW, BBOX_TF, BBOX_TEEOS,
                       BBOX_HSM, BBOX_AOS_DP, BBOX_AOS_SD, BBOX_AOS_LINUX, BBOX_AOS_CORE,
                       BBOX_MICROWATT, BBOX_IMU};

    for (i = 0; i < (s32)(sizeof(list) / sizeof(list[0])); i++) {
        if (coreid == list[i]) {
            return false;
        }
    }
    return true;
}

/*
 * @brief       : init area info data
 * @param [in]  : struct rdr_area_mem_info *info        area member info
 * @return      : NA
 */
STATIC void rdr_field_areainfo_init(const struct rdr_area_mem_info *info)
{
    s32 i, j;

    for (i = 0; i < (s32)info->num; i++) {
        g_pbb->area_info[i].coreid = info->area_mem_id[i];
        g_pbb->area_info[i].offset = info->area_mem_addr[i];
        g_pbb->area_info[i].length = info->area_mem_size[i];
        if (g_pbb->area_info[i].length == 0) {
            continue;
        }

        /* Clear the reserved memory and BIOS. (ATF, TEE, LPM, HSM, etc.) The OS is started before or in parallel.
         * The reserved memory already contains data and is not cleared.
         */
        for (j = 0; j < (s32)bbox_get_device_num(); j++) {
            s32 ret;
            void *vaddr = NULL;
            if (rdr_field_need_clear(info->area_mem_id[i]) == false) {
                continue;
            }
            vaddr = rdr_map((u32)j, (phys_addr_t)g_pbb->area_info[i].offset, g_pbb->area_info[i].length);
            if (vaddr == NULL) {
                BB_PRINT_ERR("map device [%d] area info[%d][0x%hhx] failed.\n", j, i, g_pbb->area_info[i].coreid);
                continue;
            }

            ret = memset_s(vaddr, g_pbb->area_info[i].length, 0, g_pbb->area_info[i].length);
            if (ret != EOK) {
                BB_PRINT_ERR("memset_s failed, index: %d.\n", i);
            }
            rdr_unmap(vaddr);
            vaddr = NULL;
        }
    }
}

/*
 * @brief       : init log info data
 * @return      : NA
 */
STATIC void rdr_field_loginfo_init(void)
{
    s32 ret;
    size_t size = sizeof(struct rdr_log_info_s);
    BB_CHECK_PTR(g_pbb, return, "invalid param, g_pbb is NULL.\n");
    ret = memset_s(&g_pbb->log_info, size, 0, size);
    BB_CHECK_MEMSET(ret, return);
    return;
}

/*
 * @brief       : init top head data
 * @return      : NA
 */
STATIC void rdr_field_tophead_init(u32 num)
{
    s32 ret;
    g_pbb->top_head.magic = FILE_MAGIC;
    g_pbb->top_head.version = RDR_VERSION;
    g_pbb->top_head.area_number = num;
    ret = memcpy_s(g_pbb->top_head.product_name, RDR_PRODUCT_RELATION_LEN, BBOX_PRODUCT, strlen(BBOX_PRODUCT));
    BB_CHECK_MEMCPY(ret, return);
    return;
}

/*
 * @brief       : rdr area config init
 * @param [in]  : struct rdr_area_mem_info *inf     mem info
 * @return      : != 0 fail; = 0 success
 */
STATIC s32 rdr_field_area_init(struct rdr_area_mem_info *info)
{
    s32 i;
    u64 sum_size = 0;
    const struct bbox_config_data *config = bbox_get_config();

    BB_CHECK_PTR(info, return BBOX_FAILURE, "invalid input param, info is NULL.\n");
    BB_CHECK_PTR(config, return BBOX_FAILURE, "invalid input param, config is NULL.\n");

    info->end_addr = config->bbox_data.addr + config->bbox_data.length;
    info->num = BBOX_MIN(config->reserved_area.area_num, BBOX_AREA_MAXIMUM);
    for (i = 0; i < (s32)info->num; i++) {
        info->area_mem_id[i] = rdr_get_exception_coreid(config->reserved_area.area_name[i]);
        info->area_mem_size[i] = config->reserved_area.area_size[i];
        sum_size += (u64)info->area_mem_size[i];
    }

    BB_CHECK_EXP_ACT((sum_size + RDR_BASEINFO_SIZE) > config->bbox_data.length,
        return BBOX_FAILURE, "init rdr area size failed.\n");

    for (i = (s32)info->num - 1; i > 0; i--) {
        if (i == ((s32)info->num - 1)) {
            info->area_mem_addr[i] = info->end_addr - (u64)(info->area_mem_size[i]);
        } else {
            info->area_mem_addr[i] = info->area_mem_addr[i + 1] - (u64)(info->area_mem_size[i]);
        }
    }
    info->area_mem_addr[0] = config->bbox_data.addr + RDR_BASEINFO_SIZE;
    info->area_mem_size[0] = (u32)((info->area_mem_addr[1] - RDR_BASEINFO_SIZE) - config->bbox_data.addr);
    return BBOX_SUCCESS;
}

/*
 * @brief       : rdr field init
 * @return      : != 0 fail; = 0 success
 */
s32 rdr_field_init(void)
{
    s32 ret;
    u64 addr = 0;
    u64 size = 0;
    struct rdr_area_mem_info info = {0};

    ret = bbox_config_get_rdr(&addr, &size);
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "get rdr phymem addr&size failed.\n");

    g_pbb = (struct rdr_struct_s *)rdr_map(0, addr, (size_t)size);
    BB_CHECK_PTR(g_pbb, return BBOX_FAILURE, "map pbb failed.\n");

    g_tmp_pbb = (struct rdr_struct_s *)bbox_vmalloc((size_t)size);
    if (g_tmp_pbb == NULL) {
        BB_PRINT_ERR("vmalloc tmp pbb failed.\n");
        rdr_field_clear_data();
        return BBOX_FAILURE;
    }

#ifndef CFG_SOC_PLATFORM_ESL_FPGA
#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
    ret = bbox_copy_exception_msg(g_tmp_pbb, (const char *)g_pbb, (u32)size);
#else
    ret = memcpy_s(g_tmp_pbb, (size_t)size, g_pbb, (size_t)size);
#endif
    if (ret != EOK) {
        BB_PRINT_ERR("memcpy data to tmppbb failed.\n");
        rdr_field_clear_data();
        return BBOX_FAILURE;
    }

    ret = memset_s(g_pbb, (size_t)size, 0, RDR_BASEINFO_SIZE);
    BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);
#endif

    ret = rdr_field_area_init(&info);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("init rdr area failed.\n");
        rdr_field_clear_data();
        return BBOX_FAILURE;
    }

    rdr_field_tophead_init(info.num);
    rdr_field_baseinfo_init();
    rdr_field_areainfo_init(&info);
    rdr_field_loginfo_init();
    return BBOX_SUCCESS;
}

/*
 * @brief       : rdr field exit
 * @return      : NA
 */
void rdr_field_exit(void)
{
    (void)bbox_unregister_channel_notifier(&g_rdr_field_comm_block);
    rdr_field_clear_data();
}


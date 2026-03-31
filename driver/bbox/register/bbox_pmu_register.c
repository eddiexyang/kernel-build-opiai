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

#include "bbox_pmu_register.h"
#include <linux/io.h>
#include <linux/hisi-spmi.h>
#include <linux/securec.h>
#include <linux/securectype.h>
#include "bbox_platform.h"
#include "bbox_register.h"
#include "common/bbox_print.h"
#include "common/bbox_common.h"
#include "config/bbox_config.h"

STATIC struct spmi_controller *g_spmi_ctrl = NULL;

STATIC struct bbox_pmu_info g_pmu_regs[] = {
    {SMPI_SLAVEID_MAIN, MAIN_PMU_DUMP_REG_START, MAIN_PMU_DUMP_REG_NUM},
    {SMPI_SLAVEID_SUBA, SUBA_PMU_DUMP_REG_START, SUBA_PMU_DUMP_REG_NUM},
    {SMPI_SLAVEID_SUBB, SUBB_PMU_DUMP_REG_START, SUBB_PMU_DUMP_REG_NUM},
};

/*
 * @brief       : set spmi controller
 * @return      : != 0 failure; ==0 success
 */
STATIC s32 bbox_set_spmi_ctrl(void)
{
    u32 channel;
    struct spmi_controller *ctrl = NULL;

    if (g_spmi_ctrl == NULL) {
        channel = bbox_config_get_spmi_channel();
        ctrl = spmi_busnum_to_ctrl(channel);
        if (ctrl == NULL) {
            BB_PRINT_ERR("missing SPMI controller.\n");
            return BBOX_FAILURE;
        }
        g_spmi_ctrl = ctrl;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : get spmi controller
 * @return      : spmi ctrl pointer
 */
STATIC struct spmi_controller *bbox_get_spmi_ctrl(void)
{
    return g_spmi_ctrl;
}

/*
 * @brief       : record the exception type in the reset reason register
 * @param [in]  : u8 e_type     exception type
 * @return      : NA
 */
void bbox_record_exce_type(u8 e_type)
{
    BB_CHECK_PTR(g_spmi_ctrl, return, "invalid param, spmi ctrl is NULL.\n");
    (void)spmi_ext_register_writel(g_spmi_ctrl, 0, PMU_RESET_REASON_OFFSET, &e_type, 1);
}

/*
 * @brief       : record the start stage in the reset reason register
 * @param [in]  : u8 stage      start stage
 * @return      : NA
 */
void bbox_record_kernel_stage(u8 stage)
{
    BB_CHECK_PTR(g_spmi_ctrl, return, "invalid param, spmi ctrl is NULL.\n");
    (void)spmi_ext_register_writel(g_spmi_ctrl, 0, PMU_KERNEL_STAGE_OFFSET, &stage, 1);
}

/*
 * @brief       : read pmu regs, write to buffer
 * @param [in]  : u8 sid        sid
 * @param [in]  : u16 offset    reg offset
 * @param [out] : u8 *buf       buffer to put data
 * @param [in]  : u32 len       size of pmu registers
 * @return      : != 0 failure; ==0 success
 */
STATIC s32 bbox_read_pmu_regs(u8 sid, u16 offset, u8 *buf, u32 len)
{
    u32 i;

    BB_CHECK_PTR(buf, return BBOX_FAILURE, "invalid param, buf is NULL.\n");
    BB_CHECK_PTR(g_spmi_ctrl, return BBOX_FAILURE, "invalid param, spmi ctrl is NULL.\n");
    BB_CHECK_EXP_ACT(len == 0, return BBOX_FAILURE, "invalid param, len is zero.\n");

    // read only 1 register at a time
    for (i = 0; i < len; i++) {
        int ret = spmi_ext_register_readl(g_spmi_ctrl, sid, (offset + i), &buf[i], 1);
        if (ret != 0) {
            BB_PRINT_ERR("fail to read register with %d! pmu[%d][0x%x], total len[0x%x].\n",
                         ret, sid, (offset + i), len);
            continue;
        }
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : get pmu registers data
 * @param [in]  : char *buffer      buffer
 * @param [in]  : u32 length        buffer size
 * @return      : <0 failure; =0 success
 */
s32 bbox_get_pmu_info(char *buffer, u32 length)
{
    u8 data = 0;
    s32 i;
    u32 j;
    s32 ret;
    u32 num = 0;
    u32 offset = 0;
    struct spmi_controller *ctrl = NULL;

    // set spmi ctrl handle if need
    ret = bbox_set_spmi_ctrl();
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE, "rdr set spmi ctrl failed.\n");

    // get spmi ctrl handle
    ctrl = bbox_get_spmi_ctrl();
    BB_CHECK_PTR(ctrl, return BBOX_FAILURE, "rdr get spmi ctrl failed.\n");

    // read one reg at a time
    for (i = 0; i < (s32)(sizeof(g_pmu_regs) / sizeof(struct bbox_pmu_info)) && num < length; i++) {
        ret = sprintf_s(buffer + num, length - num, "pmu[%2d][0x%hx@0x%x]:\t",
                        g_pmu_regs[i].type, g_pmu_regs[i].offset, g_pmu_regs[i].size);
        BB_CHECK_SPRINTF(ret, return ret);
        num += (u32)ret;

        for (j = 0; j < g_pmu_regs[i].size && num < length; j++) {
            offset = g_pmu_regs[i].offset + j;
            ret = spmi_ext_register_readl(ctrl, g_pmu_regs[i].type, offset, &data, PMU_EACH_READ_ONE_NUM);
            if (ret != BBOX_SUCCESS) {
                BB_PRINT_ERR("failed to read register with ret = %d! pmu[%d][0x%x], len[0x%x].\n",
                             ret, g_pmu_regs[i].type, offset, g_pmu_regs[i].size);
                break;
            }
            ret = sprintf_s(buffer + num, length - num, "%02hhx", data);
            BB_CHECK_SPRINTF(ret, return ret);
            num += (u32)ret;

            // read clean the reg, write 1 to clean, but the stat of startup need retain
            if (offset == MAIN_PMU_HEX_22B) {
                data = MAIN_PMU_HEX_22B_MASK;
            } else if (offset == MAIN_PMU_HEX_22C) {
                data = MAIN_PMU_HEX_22C_MASK;
            } else {
                data = COMM_PMU_MASK;
            }
            (void)spmi_ext_register_writel(ctrl, g_pmu_regs[i].type, offset, &data, PMU_EACH_READ_ONE_NUM);
        }

        if (num < length) {
            ret = sprintf_s(buffer + num, length - num, "\n");
            BB_CHECK_SPRINTF(ret, return ret);
            num += (u32)ret;
        }
    }

    return (s32)num;
}

enum reg_type_list bbox_register_type(const char *name, u32 nlen)
{
    BB_CHECK_PTR(name, return REG_TYPE_UNKNOWN, "invalid param, name is NULL.\n");

    if (strncmp(PMU_REG_NAME, name, nlen) == 0) {
        return REG_TYPE_PMU;
    } else {
        return REG_TYPE_SCTRL;
    }
}

void *bbox_register_ioremap(const char *name, u32 nlen, u64 addr, u32 size)
{
    enum reg_type_list type;
    void *map_addr = NULL;

    type = bbox_register_type(name, nlen);
    if (type == REG_TYPE_PMU) {
        map_addr = (void *)(uintptr_t)addr;
    } else if (type == REG_TYPE_SCTRL) {
        map_addr = ioremap(addr, size);
    }

    return map_addr;
}

void bbox_register_iounmap(const char *name, u32 nlen, void *map_addr)
{
    if ((bbox_register_type(name, nlen) == REG_TYPE_SCTRL) && (map_addr != NULL)) {
        iounmap(map_addr);
    }
}

/*
 * @brief       : dump registers to DDR
 * @return      : NA
 */
void bbox_register_dump(enum reg_type_list reg_type, u8 *dst, u32 dstsz, const u8 *src, u32 srcsz)
{
    if (reg_type == REG_TYPE_PMU) {
        u8 buffer[PMU_REG_MAX_SIZE] = {0};
        u32 size = BBOX_MIN(srcsz, PMU_REG_MAX_SIZE);
        s32 ret = bbox_read_pmu_regs(SMPI_SLAVEID_MAIN, (u16)(uintptr_t)src, buffer, size);
        if (ret == BBOX_SUCCESS) {
            ret = memcpy_s(dst, dstsz, buffer, size);
            BB_CHECK_MEMCPY(ret, return);
        }
    } else {
        s32 ret = memcpy_s(dst, dstsz, src, srcsz);
        BB_CHECK_MEMCPY(ret, return);
    }
}

/*
 * @brief       : register module init
 * @return      : NA
 */
s32 bbox_register_init(void)
{
    return bbox_set_spmi_ctrl();
}

/*
 * @brief       : register module exit
 * @return      : NA
 */
void bbox_register_exit(void)
{
    g_spmi_ctrl = NULL;
}


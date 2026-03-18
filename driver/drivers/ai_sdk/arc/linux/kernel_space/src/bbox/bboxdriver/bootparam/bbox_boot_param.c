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

#include "bbox_boot_param.h"

#include <linux/securec.h>
#include <linux/init.h>
#include <linux/topology.h>
#include <linux/fs.h>
#include <asm/setup.h>
#include "bbox_platform.h"
#include "bbox_product.h"
#include "common/bbox_print.h"
#include "common/bbox_sys_api.h"
#include "device/bbox_pub.h"
#include "device/bbox_types.h"
#include "rdr/rdr_common.h"
#include "config/bbox_config.h"


STATIC struct bbox_boot_param_mgr g_bbox_boot_param;

/*
 * @brief       : Find a non-boolean option (i.e. option=argument).
 * @param [in]  : option    option string to look for
 * @param [out] : buffer    memory buffer to return the option argument
 * @param [in]  : bufsize   size of the supplied memory buffer
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_parse_cmdline_option(const char *option, char *buffer, u32 bufsize)
{
    u32 i;
    s32 ret;
    const char *p_str = NULL;
    const char *q_str = NULL;

    BB_CHECK_PTR(option, return BBOX_FAILURE, "invalid param, option is NULL.\n");
    BB_CHECK_PTR(buffer, return BBOX_FAILURE, "invalid param, buffer is NULL.\n");
    BB_CHECK_PTR(g_bbox_boot_param.command_line, return BBOX_FAILURE, "invalid param, saved_command_line is NULL.\n");

    p_str = strstr(g_bbox_boot_param.command_line, option);
    if (p_str == NULL) {
        return BBOX_FAILURE;
    }

    q_str = &p_str[strlen(option)];
    for (i = 0U; i < bufsize; i++) {
        if ((q_str[i] == ' ') || (q_str[i] == '\0')) {
            break;
        }
    }

    if (i == bufsize) {
        BB_PRINT_ERR("Cmdline option[%s] argument out of length limit: %u.\n", option, bufsize);
        return BBOX_FAILURE;
    }

    if (i == 0U) {
        BB_PRINT_ERR("Cmdline cannot find option argument: %s\n", option);
        return BBOX_FAILURE;
    }

    ret = strncpy_s(buffer, bufsize, q_str, i);
    BB_CHECK_STRNCPY(ret, return BBOX_FAILURE);
    return BBOX_SUCCESS;
}

/*
 * @brief       : parse reboot reason cmdline
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_parse_reboot_reason(void)
{
    s32 i, ret;
    char *p_str = NULL;
    char *q_str = NULL;
    u32 size = 0;
    const char *option = "reboot_reason=";
    char reboot_reason[BBOX_REBOOT_REASON_LEN] = "undef";
    char reboot_reason_cmdline[BBOX_REBOOT_REASON_STR_LEN] = {0};

    ret = bbox_parse_cmdline_option(option, reboot_reason_cmdline, BBOX_REBOOT_REASON_STR_LEN);
    if (ret != BBOX_SUCCESS) {
        return BBOX_SUCCESS;
    }

    p_str = (char *)reboot_reason_cmdline;
    for (i = 0; i < (s32)DEVICE_MAX_NUM; i++) {
        ret = memset_s(reboot_reason, BBOX_REBOOT_REASON_LEN, 0, BBOX_REBOOT_REASON_LEN);
        BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

        q_str = strchr(p_str, ',');
        if (q_str == NULL) {
            if (*p_str != '\0') {
                ret = memcpy_s(reboot_reason, BBOX_REBOOT_REASON_LEN, p_str, strlen(p_str));
                BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
            }
        } else {
            size = (u32)(q_str - p_str);
            ret = memcpy_s(reboot_reason, BBOX_REBOOT_REASON_LEN, p_str, size);
            BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
            p_str = &q_str[1];
        }

        reboot_reason[BBOX_REBOOT_REASON_LEN - 1U] = '\0';
        g_bbox_boot_param.reboot_etype[i] = rdr_get_etype((const char *)reboot_reason);
        BB_PRINT_INFO("device-%d reboot reason [%s][0x%hhx]\n", i, reboot_reason, g_bbox_boot_param.reboot_etype[i]);

        if (q_str == NULL) {
            break;
        }
    }

    if ((g_bbox_boot_param.reboot_etype[0] >= (u8)BBOX_REBOOT_REASON_LABEL2) &&
        (g_bbox_boot_param.reboot_etype[0] < (u8)BBOX_REBOOT_REASON_LABEL3)) {
        for (i = 1; i < (s32)DEVICE_MAX_NUM; i++) {
            g_bbox_boot_param.reboot_etype[i] = g_bbox_boot_param.reboot_etype[0];
        }
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : parse sub reason cmdline
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_parse_sub_reason(void)
{
    s32 i, ret;
    char *p_str = NULL;
    char *q_str = NULL;
    u32  size = 0;
    const char *option_lp = "lp_reason=";
    char sub_reason[BBOX_SUB_REASON_LEN] = {0};
    char sub_reason_cmdline[BBOX_SUB_REASON_STR_LEN] = {0};

    ret = bbox_parse_cmdline_option(option_lp, sub_reason_cmdline, BBOX_SUB_REASON_STR_LEN);
    if (ret != BBOX_SUCCESS) {
        const char *option_lpm3 = "lpm3_reason=";
        ret = bbox_parse_cmdline_option(option_lpm3, sub_reason_cmdline, BBOX_SUB_REASON_STR_LEN);
        if (ret != BBOX_SUCCESS) {
            return BBOX_SUCCESS;
        }
    }

    p_str = (char *)sub_reason_cmdline;
    for (i = 0; i < (s32)DEVICE_MAX_NUM; i++) {
        u32 val = 0;
        ret = memset_s(sub_reason, BBOX_SUB_REASON_LEN, 0, BBOX_SUB_REASON_LEN);
        BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

        q_str = strchr(p_str, ',');
        if (q_str == NULL) {
            if (*p_str != '\0') {
                ret = memcpy_s(sub_reason, BBOX_SUB_REASON_LEN, p_str, strlen(p_str));
                BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
            }
        } else {
            size = (u32)(q_str - p_str);
            ret = memcpy_s(sub_reason, BBOX_SUB_REASON_LEN, p_str, size);
            BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
            p_str = &q_str[1];
        }
        sub_reason[BBOX_SUB_REASON_LEN - 1U] = '\0';
        ret = bbox_strtouint_h((char *)sub_reason, &val);
        if ((ret != BBOX_SUCCESS) || (val > U8_MAX)) {
            g_bbox_boot_param.sub_etype[i] = 0;
            BB_PRINT_ERR("Invalid value, device-%d sub reason: %s\n", i, sub_reason);
            return BBOX_FAILURE;
        } else {
            g_bbox_boot_param.sub_etype[i] = (u8)val;
            BB_PRINT_INFO("device-%d sub reason [0x%hhx]\n", i, g_bbox_boot_param.sub_etype[i]);
        }
        if (q_str == NULL) {
            break;
        }
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : parse devmem addr cmdline
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_parse_devmem_addr(void)
{
    s32 ret;
    const char *option = "dump_data_addr=";
    char dump_addr_cmdline[BBOX_DEVMEM_ADDR_LEN] = {0};

    ret = bbox_parse_cmdline_option(option, dump_addr_cmdline, BBOX_DEVMEM_ADDR_LEN);
    if (ret != BBOX_SUCCESS) {
        return BBOX_SUCCESS;
    }

    dump_addr_cmdline[BBOX_DEVMEM_ADDR_LEN - 1U] = '\0';
    ret = bbox_strtol_h((char *)dump_addr_cmdline, &g_bbox_boot_param.devmem_addr);
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE,
        "invalid value, dump addr cmdline is %s.\n", dump_addr_cmdline);

    return BBOX_SUCCESS;
}

/*
 * @brief       : parse devmem len cmdline
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_parse_devmem_len(void)
{
    s32 ret;
    const char *option = "dump_data_len=";
    char dump_len_cmdline[BBOX_DEVMEM_SIZE_LEN] = {0};

    ret = bbox_parse_cmdline_option(option, dump_len_cmdline, BBOX_DEVMEM_SIZE_LEN);
    if (ret != BBOX_SUCCESS) {
        return BBOX_SUCCESS;
    }

    dump_len_cmdline[BBOX_DEVMEM_SIZE_LEN - 1U] = '\0';
    ret = bbox_strtouint_h((char *)dump_len_cmdline, &g_bbox_boot_param.devmem_len);
    BB_CHECK_EXP_ACT(ret != BBOX_SUCCESS, return BBOX_FAILURE,
        "invalid value, dump len cmdline is %s.\n", dump_len_cmdline);

    return BBOX_SUCCESS;
}

#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
/*
 * @brief       : parse aosmode cmdline
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_parse_aosmode_flag(void)
{
    s32 ret;
    const char *option = "aosmode=";
    char dump_len_cmdline[BBOX_AOSMODE_SIZE_LEN] = {0};

    ret = bbox_parse_cmdline_option(option, dump_len_cmdline, BBOX_AOSMODE_SIZE_LEN);
    if (ret != BBOX_SUCCESS) {
        g_bbox_boot_param.aos_flag = AOS_CORE_DISABLE;
        return BBOX_SUCCESS;
    }

    dump_len_cmdline[BBOX_AOSMODE_SIZE_LEN - 1U] = '\0';
    if (strncmp(CMDLINE_AOSENABLE, dump_len_cmdline, CMDLINE_AOSENABLE_LEN) == 0) {
        g_bbox_boot_param.aos_flag = AOS_CORE_ENABLE;
    } else {
        g_bbox_boot_param.aos_flag = AOS_CORE_DISABLE;
    }

    return BBOX_SUCCESS;
}
#endif

/*
 * @brief       : parse dpclk cmdline
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_parse_dpclk(void)
{
    s32 ret;
    u32 dpclk_config = 0;
    const char *option = "dpclk=";
    char dpclk_cmdline[BBOX_DPCLK_SIZE] = {0};

    ret = bbox_parse_cmdline_option(option, dpclk_cmdline, BBOX_DPCLK_SIZE);
    if (ret != BBOX_SUCCESS) {
        g_bbox_boot_param.dpclk = BBOX_CLOCK_VIRTTIME;
        return BBOX_SUCCESS;
    }

    dpclk_cmdline[BBOX_DPCLK_SIZE - 1U] = '\0';
#ifndef BBOX_UT
    ret = kstrtou32((char *)dpclk_cmdline, 0, &dpclk_config);
    if (ret != 0) {
        BB_PRINT_INFO("Invalid dpclk cmdline, use default.(val=%s)\n", dpclk_cmdline);
        dpclk_config = BBOX_DPCLK_DEFAULT;
    }

    if (dpclk_config == BBOX_DPCLK_REAL) {
        g_bbox_boot_param.dpclk = BBOX_CLOCK_REALTIME;
    } else {
        g_bbox_boot_param.dpclk = BBOX_CLOCK_VIRTTIME;
    }
#endif

    BB_PRINT_INFO("Dpclkconfig. (value=%u)\n", g_bbox_boot_param.dpclk);

    return BBOX_SUCCESS;
}

/*
 * @brief       : get os cmdline
 * @return      : cmdline
 */
STATIC char* bbox_get_cmdline(void)
{
    static char cmdline[COMMAND_LINE_SIZE] = {0};
    struct file *fp = NULL;
    loff_t pos = 0;

    fp = filp_open(CMDLINE_FILE, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        BB_PRINT_ERR("open %s fail!\n", CMDLINE_FILE);
        return NULL;
    }
    kernel_read(fp, cmdline, COMMAND_LINE_SIZE - 1, &pos);
    filp_close(fp, NULL);
    return cmdline;
}

/*
 * @brief       : parse devmem len cmdline
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_boot_command_line(void)
{
    g_bbox_boot_param.command_line = bbox_get_cmdline();
    return BBOX_SUCCESS;
}

/*
 * @brief       : boot option init
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_init_cmdline_option(void)
{
    s32 ret;

    // boot cmdline
    ret = bbox_boot_command_line();
    if (ret != BBOX_SUCCESS) {
        return BBOX_FAILURE;
    }

    // boot option: reboot reason
    ret = bbox_parse_reboot_reason();
    if (ret != BBOX_SUCCESS) {
        return BBOX_FAILURE;
    }

    // boot option: sub reason
    ret = bbox_parse_sub_reason();
    if (ret != BBOX_SUCCESS) {
        return BBOX_FAILURE;
    }

    // boot option: devmem addr
    ret = bbox_parse_devmem_addr();
    if (ret != BBOX_SUCCESS) {
        return BBOX_FAILURE;
    }

    // boot option: devmem len
    ret = bbox_parse_devmem_len();
    if (ret != BBOX_SUCCESS) {
        return BBOX_FAILURE;
    }

#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
    // boot option: aosmode
    ret = bbox_parse_aosmode_flag();
    if (ret != BBOX_SUCCESS) {
        return BBOX_FAILURE;
    }
#endif

    // boot option: dpclk
    ret = bbox_parse_dpclk();
    if (ret != BBOX_SUCCESS) {
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : boot option init
 * @return      : <0: failure; =0: success
 */
STATIC s32 bbox_init_ddr_option(void)
{
    s32 i, ret;
    u64 paddr = 0;
    u64 size = 0;

    ret = bbox_config_get_boot_param(&paddr, &size);
    if (ret != BBOX_SUCCESS) {
        return BBOX_FAILURE;
    }

    if (paddr == 0) {
        BB_PRINT_INFO("no find ddr option config.\n");
        return BBOX_SUCCESS;
    }

    for (i = 0; i < (s32)g_bbox_boot_param.device_num; i++) {
        u64 dev_addr = bbox_get_device_addr((u32)i, paddr);
        struct bbox_boot_param *head = (struct bbox_boot_param *)bbox_ioremap(dev_addr, size);
        if (head == NULL) {
            BB_PRINT_ERR("[device-%d] map boot param ddr addr failed.\n", i);
            continue;
        }

        if ((head->magic != BBOX_BOOT_PARAM_MAGIC) ||
            (head->magic_end != BBOX_BOOT_PARAM_MAGIC_END)) {
            BB_PRINT_INFO("[device-%d] no find ddr option data.\n", i);
            bbox_iounmap(head);
            head = NULL;
            continue;
        }

        g_bbox_boot_param.reboot_etype[i] = head->reboot_reason;
        g_bbox_boot_param.sub_etype[i] = head->sub_reason;
        BB_PRINT_INFO("[device-%d] reboot_reason: 0x%hhx, sub_reason: 0x%hhx\n",
                      i, head->reboot_reason, head->sub_reason);
        if (i == 0) {
            g_bbox_boot_param.devmem_addr = head->devmem_addr;
            g_bbox_boot_param.devmem_len = (u32)head->devmem_len;
#ifndef BBOX_UT
            BB_PRINT_INFO("[device-%d] devmem_addr: %pK, devmem_len: 0x%llx.\n",
                          i, (void *)(uintptr_t)head->devmem_addr, head->devmem_len);
#endif
        }
        bbox_iounmap(head);
        head = NULL;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : parse device num
 * @return      : device num
 */
STATIC void bbox_init_device_num(void)
{
    s32 node;
    u32 device_num = 0;
    for_each_node_with_cpus(node) {
        device_num++;
    }
    BB_PRINT_INFO("device num: %u.\n", device_num);
    g_bbox_boot_param.device_num = BBOX_MIN(device_num, DEVICE_MAX_NUM);
}

STATIC bool bbox_boot_check_ddr(void)
{
    return bbox_check_feature(FEATURE_BOOTPARAM_DDR);
}

/*
 * @brief       : boot param init
 * @return      : <0: failure; =0: success
 */
s32 bbox_boot_param_init(void)
{
    s32 ret;
    size_t size;

    size = sizeof(g_bbox_boot_param);
    ret = memset_s(&g_bbox_boot_param, size, 0, size);
    BB_CHECK_MEMSET(ret, return BBOX_FAILURE);

    // device num
    bbox_init_device_num();

    // boot option
    if (bbox_boot_check_ddr()) {
        ret = bbox_init_ddr_option();
    } else {
        ret = bbox_init_cmdline_option();
    }

    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("boot option init failed.\n");
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : get device num
 * @return      : device num
 */
u32 bbox_get_device_num(void)
{
    return g_bbox_boot_param.device_num;
}

/*
 * @brief       : get reboot type by device id
 * @param [in]  : u32 devid                     device id
 * @return      : reboot type
 */
u8 bbox_get_reboot_type(u32 devid)
{
    if (devid < DEVICE_MAX_NUM) {
        return g_bbox_boot_param.reboot_etype[devid];
    }
    return DEVICE_COLDBOOT;
}

/*
 * @brief       : get sub type by device id
 * @param [in]  : u32 devid                     device id
 * @return      : sub type
 */
u8 bbox_get_sub_etype(u32 devid)
{
    if (devid < DEVICE_MAX_NUM) {
        return g_bbox_boot_param.sub_etype[devid];
    }
    return 0;
}

/*
 * @brief       : get devmem addr
 * @return      : devmem addr
 */
u64 bbox_get_devmem_addr(void)
{
    return g_bbox_boot_param.devmem_addr;
}

/*
 * @brief       : get devmem len
 * @return      : devmem len
 */
u32 bbox_get_devmem_len(void)
{
    return g_bbox_boot_param.devmem_len;
}

#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
/*
 * @brief       : get aosmode flag
 * @return      : aosmode flag
 */
u32 bbox_get_aosmode_flag(void)
{
    return g_bbox_boot_param.aos_flag;
}
#endif

/*
 * @brief       : get dpclk
 * @return      : clockId config
 */
bool bbox_use_real_dpclk(void)
{
    return (g_bbox_boot_param.dpclk == BBOX_CLOCK_REALTIME);
}

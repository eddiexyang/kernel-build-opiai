/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/kallsyms.h>
#include <linux/suspend.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include <linux/io.h>

#include "kernel_version_adapt.h"
#ifndef DEVMNG_UT
#include "devdrv_manager_llc.h"

#ifdef CFG_SOC_PLATFORM_CLOUD
#include "i2c_cpld.h"
#elif !defined(CFG_SOC_PLATFORM_MINIV2)
#include <hisi_pmic.h>
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "can_drv_api.h"
#include "ufs_hisi_api.h"
#include "devdrv_os_power.h"
#include "drv_notify.h"
#include "dms_chip_info.h"
#endif

#include "devdrv_manager_common.h"
#include "devdrv_manager.h"
#include "devmng_dms_adapt.h"
#include "devdrv_ipc.h"
#include "devdrv_info.h"
#include "devdrv_manager_container.h"
#include "ascend_kernel_hal.h"
#include "icm_interface.h"
#include "dms_ipc_interface.h"
#include "tsmng_interface.h"

#ifndef CFG_SOC_PLATFORM_MINIV2
#else
#include "devdrv_manager_container.h"
#include "devdrv_ipc_msg.h"
#include "devdrv_user_common.h"
#include <linux/suspend.h>
#include "drv_whitelist.h"
#include "HiDvppQueryInfo.h"
#endif

#ifdef CFG_BUILD_DEBUG
#include "devdrv_info_debug.h"
#endif

#ifdef CFG_FEATURE_HISS
#include "hsm_info.h"
#endif

#define ECC_DATA_LEN       8
#define OFFSET_EIGHT       8
#define OFFSET_SIXTEEN     16
#define OFFSET_TWENTY_FOUR 24

#define UINT_TYPE_MAX 0xFFFFFFFF
#define DRV_KERNEL_ERROR_RESUME 150

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define DSMI_DMP_MBIST_FILE        "/var/silerika_bs9sx1a_bist.bin"
#define DSMI_DMP_MBIST_REGION_ADDR (0x2E3A1000ULL)
#define DSMI_DMP_MBIST_LOG_REGION  (0x19080U)
#define DSMI_DMP_MBIST_REGION_LEN  (0x200000U - DSMI_DMP_MBIST_LOG_REGION)

#ifdef AOS_LLVM_BUILD
#define DEVDRV_SEND_IPC_MBOX_ID HISI_RPROC_SI_Q_TX_RPID17
#define DEVDRV_SEND_TAISHAN_SUBID TAISHAN1_SUBSYS_AOS
#else
#define DEVDRV_SEND_IPC_MBOX_ID HISI_RPROC_SI_Q_TX_RPID15
#define DEVDRV_SEND_TAISHAN_SUBID TAISHAN1_SUBSYS
#endif
#endif

int bbox_get_device_errorcode(u32 dev_id, u32 *e_code, u32 e_capacity);
int bbox_get_device_ecode_info(u32 ecode, u8 *desc, u32 length);
__attribute__((weak)) void devdrv_manager_get_bist_mode(struct devdrv_ipc_imu *ipc_msg);
__attribute__((weak)) void devdrv_manager_bist_prepare(void);

/* save the temperature data from IMU: chip, hbm and N Die */
int temp_data[MAX_CHIP_NUM][TEMP_FROM_IMU_NUM] = {0};
struct timeval ipc_send_timestamp[MAX_CHIP_NUM] = {0};

#ifdef CFG_SOC_PLATFORM_MDC_V51
static DEFINE_MUTEX(g_suspend_lock);
#endif

int devdrv_manager_get_chip_type(int *chip_type)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int ret;

    if (chip_type == NULL) {
        devdrv_drv_err("chip_type is null pointer, please check.\n");
        return -EINVAL;
    }

    ret = centre_notify_get_val(KEY_CHIP_TYPE_INDEX, chip_type);
    if (ret != 0) {
        devdrv_drv_err("get chip_type failed, ret(%d).\n", ret);
        return -ENOSYS;
    }
#ifdef CFG_SOC_MDC_V51_LITE
    if (*chip_type != CHIP_TYPE_ASCEND_V51_LITE) {
        devdrv_drv_err("get chip_type failed, chip_type = %d.\n", *chip_type);
        return -EINVAL;
    }
#else
    if ((*chip_type != CHIP_TYPE_ASCEND_V1) && (*chip_type != CHIP_TYPE_ASCEND_V2)) {
        devdrv_drv_err("get chip_type failed, chip_type = %d.\n", *chip_type);
        return -EINVAL;
    }
#endif
    return 0;
#else
    *chip_type = CHIP_TYPE_NOT_ASCEND;
    return 0;
#endif
}
EXPORT_SYMBOL(devdrv_manager_get_chip_type);

STATIC int devdrv_get_pcie_id(unsigned long arg)
{
    struct dmanage_pcie_id_info pcie_id_info = {0};
    unsigned int dev_id;
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&pcie_id_info, (void *)((uintptr_t)arg), sizeof(struct dmanage_pcie_id_info));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    ret = devdrv_manager_trans_and_check_id(pcie_id_info.davinci_id, &phys_id, &vfid, TRANS_PHYID_TO_PFID);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
            pcie_id_info.davinci_id, ret);
        return ret;
    }

    dev_id = phys_id;
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("invalid device id, id = %u.\n", dev_id);
        return -EINVAL;
    }

    ret = devdrv_get_pcie_id_info(dev_id, &pcie_id_info);
    if (ret) {
        devdrv_drv_err("devdrv_get_pcie_id failed.\n");
        return ret;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &pcie_id_info, sizeof(struct dmanage_pcie_id_info));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }
    return 0;
}

static int g_last_ko_insert_flag = DEVDRV_KO_INSERT_WAIT;
void set_last_ko_insert_flag(void)
{
    g_last_ko_insert_flag = DEVDRV_KO_INSERT_DONE;
}

void clear_last_ko_insert_flag(void)
{
    g_last_ko_insert_flag = DEVDRV_KO_INSERT_WAIT;
}

int is_last_ko_insert(void)
{
    return g_last_ko_insert_flag ? DEVDRV_KO_INSERT_DONE : DEVDRV_KO_INSERT_WAIT;
}

int devdrv_get_max_health_code(unsigned int dev_id, unsigned int *max_health_value,
    struct devdrv_error_code_para *error_code_para)
{
    int i = 0;
    u32 max = 0;
    u32 error_level = 0;
    u32 *err_code = NULL;

    if ((dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (max_health_value == NULL) ||
        (error_code_para == NULL)) {
        devdrv_drv_err("invalid device id, id = %u.\n", dev_id);
        return -EINVAL;
    }
    err_code = error_code_para->error_code;

#if defined(CFG_SOC_PLATFORM_CLOUD) && !defined(CFG_SOC_PLATFORM_CLOUD_V2)
    if (is_last_ko_insert() != DEVDRV_KO_INSERT_DONE) {
        devdrv_drv_warn("not all ko insmod succeed.\n");
    }
#endif

    error_code_para->error_code_count = bbox_get_device_errorcode(dev_id, err_code, DMANAGE_ERROR_ARRAY_NUM);
    if (error_code_para->error_code_count < 0) {
        devdrv_drv_err("bbox_get_device_errorcode failed, ret = %d.\n", error_code_para->error_code_count);
        return -EINVAL;
    }

    if (error_code_para->error_code_count > DMANAGE_ERROR_ARRAY_NUM) {
        devdrv_drv_err("error_code_count(%d) is illegal.\n", error_code_para->error_code_count);
        return -EINVAL;
    }

    for (i = 0; i < error_code_para->error_code_count; i++) {
        error_level = ((error_code_para->error_code[i] >> ERROR_BIT_POSITION) & (TOTAL_ERROR_MASK));
        if (max < error_level) {
            max = error_level;
        }
    }

    max = (max > ERROR_MAX_VALUE) ? ERROR_MAX_VALUE : max;

    *max_health_value = ((max > 0) ? (max - 1) : 0);

    return 0;
}

STATIC int devdrv_get_health_code(unsigned long arg)
{
    struct devdrv_error_code_para para = { 0, { 0 }, 0 };
    struct ioctl_arg user_arg = {0};
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    ret = devdrv_manager_trans_and_check_id(user_arg.dev_id, &phys_id, &vfid, ALSO_DOES_SUPPORT_VF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
            user_arg.dev_id, ret);
        return ret;
    }

    ret = devdrv_get_max_health_code(phys_id, &user_arg.data1, &para);
    if (ret) {
        devdrv_drv_err("Failed to invoke devdrv_get_max_health_code.\n");
        return ret;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }
    return 0;
}

void devdrv_fresh_error_code_to_shm(struct work_struct *work)
{
    struct devdrv_error_code_para para = { 0, { 0 }, 0 };
    unsigned int health_code = 0;
    struct devdrv_info *dev_info = NULL;
    int ret, i;

    dev_info = container_of(work, struct devdrv_info, work);
    if ((dev_info == NULL) || (dev_info->shm_status == NULL)) {
        devdrv_drv_err("dev_info=NULL or dev_info->shm_status=NULL.\n");
        return;
    }

    ret = devdrv_get_max_health_code(dev_info->dev_id, &health_code, &para);
    if (ret) {
        devdrv_drv_err("[devid=%u] get health code fail, ret=%d.\n", dev_info->dev_id, ret);
        return;
    }

    dev_info->shm_status->health_status = (u16)health_code;
    dev_info->shm_status->error_cnt = para.error_code_count;
    for (i = 0; i < DEVMNG_SHM_INFO_ERROR_CODE_LEN; i++) {
        dev_info->shm_status->error_code[i] = para.error_code[i];
    }
}

int devdrv_fresh_event_code_to_shm(u32 devid, u32 *health_code, u32 health_len,
    struct shm_event_code *event_code, u32 event_len)
{
#ifndef CFG_SOC_PLATFORM_MDC_V51
    struct devdrv_info *dev_info = NULL;
    int event_cnt = 0;
    int i;

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_info == NULL) {
        devdrv_drv_warn("Dev_info is NULL. (device id=%u)\n", devid);
        return -EINVAL;
    } else if (dev_info->shm_status == NULL) {
        devdrv_drv_warn("Shm_status is NULL. (device id=%u)\n", devid);
        return -EINVAL;
    }

    for (i = 0; i < VMNG_VDEV_MAX_PER_PDEV; i++) {
        dev_info->shm_status->dms_health_status[i] = (u16)health_code[i];
    }
    for (i = 0; i < DEVMNG_SHM_INFO_EVENT_CODE_LEN; i++) {
        if (event_code[i].event_code == 0) {
            break;
        }
        dev_info->shm_status->event_code[i].event_code = event_code[i].event_code;
        dev_info->shm_status->event_code[i].fid = event_code[i].fid;
        event_cnt++;
    }
    dev_info->shm_status->event_cnt = event_cnt;
#endif
    return 0;
}

int devdrv_fresh_random_to_shm(u32 devid, char *random_number, u32 random_len)
{
    struct devdrv_info *dev_info = NULL;
    int ret;

    if (random_number == NULL || random_len < DEVMNG_SHM_INFO_RANDOM_SIZE) {
        devdrv_drv_err("random number is NULL or len error. (random_len=%u)\n", random_len);
        return -EINVAL;
    }

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if ((dev_info == NULL) || (dev_info->shm_status == NULL)) {
        devdrv_drv_err("dev_info or dev_info->shm_status is NULL. (devid=%u)\n", devid);
        return -EINVAL;
    }

    ret = memcpy_s(dev_info->shm_status->random_number, DEVMNG_SHM_INFO_RANDOM_SIZE,
        random_number, random_len);
    if (ret) {
        devdrv_drv_err("Memcpy random to shm failed. (devid=%u)\n", devid);
        return ret;
    }

    return 0;
}

STATIC int devdrv_get_error_code(unsigned long arg)
{
    struct devdrv_error_code_para user_arg = { 0, {0}, 0 };
    u32 *err_code = user_arg.error_code;
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct devdrv_error_code_para));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    ret = devdrv_manager_trans_and_check_id(user_arg.dev_id, &phys_id, &vfid, ALSO_DOES_SUPPORT_VF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
            user_arg.dev_id, ret);
        return ret;
    }

    user_arg.error_code_count = bbox_get_device_errorcode(phys_id, err_code, DMANAGE_ERROR_ARRAY_NUM);
    if (user_arg.error_code_count < 0) {
        devdrv_drv_err("bbox_get_device_errorcode failed, ret = %d.\n", user_arg.error_code_count);
        return -EINVAL;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &user_arg, sizeof(struct devdrv_error_code_para));
    if (ret != 0) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }

    return 0;
}

#if (defined CFG_FEATURE_LP_ENABLE)
#ifdef CFG_SOC_PLATFORM_MDC_V51
#define MAX_NAME_LENGTH 32
#define SILS_IPC_MSGTYPE_HAERTBEAT 8
#define SILS_IPC_CMDTYPE_OFF 6
#define SILS_IPC_DATA 0xff
STATIC int check_ipc_msg_from_safetyisland(struct drv_safety_island_ipc_info *rsp_msg,
                                           struct drv_safety_island_ipc_info *req_msg,
                                           unsigned int valid_rsp_payload_len)
{
    if (rsp_msg->cmd_dest != req_msg->cmd_src) {
        devdrv_drv_err("response msg destination addr is %d not equal reqest msg source addr %d\n",
                       rsp_msg->cmd_dest, req_msg->cmd_src);
        return -EINVAL;
    }
    if (rsp_msg->cmd_src != req_msg->cmd_dest) {
        devdrv_drv_err("response msg source addr is %d not equal reqest msg destination addr %d\n",
                       rsp_msg->cmd_src, req_msg->cmd_dest);
        return -EINVAL;
    }
    if (rsp_msg->len != valid_rsp_payload_len) {
        devdrv_drv_err("response msg payload len(%d) is error, should equal %d\n",
                       rsp_msg->len, valid_rsp_payload_len);
        return -EINVAL;
    }
    if (rsp_msg->payload[ERROR_CODE_OFFSET_IN_PAYLOAD] != 0) {
        devdrv_drv_err("get ipc msg from safety island, err_code = %d\n",
                       rsp_msg->payload[ERROR_CODE_OFFSET_IN_PAYLOAD]);
        return -EINVAL;
    }
    return 0;
}

STATIC void print_ipc_msg_info(struct drv_safety_island_ipc_info *rsp_msg,
                               struct drv_safety_island_ipc_info *req_msg)
{
    devdrv_drv_info("print ipc rsponse msg from safetyisland\n");
    PRINT_IPC_MSG_FROM_SAFETYISLAND(rsp_msg);

    devdrv_drv_err("print ipc request msg from safetyisland\n");
    PRINT_IPC_MSG_FROM_SAFETYISLAND(req_msg);
}

/* control cpu HISI_RPROC_SI_Q_TX_RPID15(addr: TAISHAN1_SUBSYS)-> safetyisland(addr SAFETY_ISLAND) */
/* control cpu HISI_RPROC_SI_Q_RX_RPID1(addr: TAISHAN1_SUBSYS) <- safetyisland(addr SAFETY_ISLAND) */
STATIC int devdrv_get_msg_from_safetyisland(u32 dev_id, struct drv_safety_island_ipc_info *msg_req,
                                            struct drv_safety_island_ipc_info *msg_rsp)
{
    int ret;
    int i;
    int msg_len = sizeof(struct drv_safety_island_ipc_info) / sizeof(rproc_msg_t);

    if ((msg_req == NULL) || (msg_rsp == NULL)) {
        devdrv_drv_err("input point is null. (device id=%u; msg_req=%d; msg_rsp=%d)\n",
                       dev_id, msg_req != NULL, msg_rsp != NULL);
        return -EINVAL;
    }

    ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, DEVDRV_SEND_IPC_MBOX_ID),
        (rproc_msg_t *)msg_req, msg_len, (rproc_msg_t *)msg_rsp, msg_len);
    if (ret != 0) {
        devdrv_drv_err("icm_msg_send_sync failed, ret=%d.\n", ret);
        return ret;
    }
    for (i = 0; i < DRVFAULT_IPC_MSG_LENGTH; i++) {
        devdrv_drv_debug("%d(0x%x)\n", i, msg_rsp->payload[i]);
    }
    return ret;
}

void devdrv_manager_set_safetyisland_power(void)
{
    int ret;
    struct drv_safety_island_ipc_info ipc_info = {0};

    devdrv_drv_event("send power msg to safetyisland.\n");

    ipc_info.cmd_type = SILS_IPC_MSGTYPE_HAERTBEAT;
    ipc_info.cmd = SILS_IPC_CMDTYPE_OFF;
    ipc_info.cmd_dest = SAFETY_ISLAND;
    ipc_info.cmd_src = DEVDRV_SEND_TAISHAN_SUBID;
    ipc_info.seq++;
    ipc_info.data_type = IPC_SAFETY_ISLAND_REQUEST;
    ipc_info.len = IPC_GET_SAFETYISLAND_INFO_FUN_TYPE_LEN;
    ipc_info.payload[0] = SILS_IPC_DATA;
    ipc_info.crc = sils_crc16((unsigned char *)&ipc_info, ipc_info.len + SILS_IPC_HEAD_LEN);

    ret = icm_msg_send_async(ICM_FD_BUILD(0, DEVDRV_SEND_IPC_MBOX_ID), (rproc_msg_t *)&ipc_info,
        sizeof(struct drv_safety_island_ipc_info) / sizeof(rproc_msg_t));
    if (ret != 0) {
        devdrv_drv_warn("send power msg to safetyisland failed, ret = %d\n", ret);
    }
    /* stop taishan sils heart*/
    ret = devdrv_manager_suspend_proc(0, DEVDRV_SUSPEND);
    if (ret != 0) {
        devdrv_drv_err("close taishan sils heart failed, ret = %d\n", ret);
    }

    return;
}
#endif

STATIC int devdrv_get_msg_from_lp(u32 dev_id, struct devdrv_ipc_imu *ipc, struct devdrv_ipc_imu *msg_ret,
    unsigned int msg_len)
{
    int ret = 0;

    if ((dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) || (ipc == NULL) || (msg_ret == NULL)) {
        devdrv_drv_err("invalid device id. (id=%u; ipc=%d; msg_ret=%d)\n", dev_id, ipc != NULL, msg_ret != NULL);
        return -EINVAL;
    }

    ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, HISI_RPROC_LP_Q_TX_RPID4_ACPU0),
        (rproc_msg_t *)ipc, sizeof(struct devdrv_ipc_imu) / sizeof(rproc_msg_t), (rproc_msg_t *)msg_ret, IPCDRV_RPROC_MSG_LENGTH);
    if (ret) {
        devdrv_drv_err("icm_msg_send_sync failed, ret=%d, ipc.cmd_type0 = %u, ipc.cmd_type1 = %u\n",
            ret, ipc->cmd_type0, ipc->cmd_type1);
        return ret;
    }

    if ((msg_ret->cmd_type0 != ipc->cmd_type0) || (msg_ret->cmd_type1 != ipc->cmd_type1)) {
// Query information from the LP module again when the cmd_types are inconsistent.
#ifdef CFG_SOC_PLATFORM_MDC_V51
        ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, HISI_RPROC_LP_Q_TX_RPID4_ACPU0),
            (rproc_msg_t *)ipc, sizeof(struct devdrv_ipc_imu) / sizeof(rproc_msg_t),
            (rproc_msg_t *)msg_ret, IPCDRV_RPROC_MSG_LENGTH);
        if (ret != 0) {
            devdrv_drv_err("devdrv_ipc_msg_send failed, ret=%d, ipc.cmd_type0 = %u, ipc.cmd_type1 = %u\n",
                ret, ipc->cmd_type0, ipc->cmd_type1);
            return ret;
        }
        if ((msg_ret->cmd_type0 == ipc->cmd_type0) && (msg_ret->cmd_type1 == ipc->cmd_type1)) {
            return ret;
        }
#endif
        devdrv_drv_err("get invalid ipc message. "
                       "ack.cmd_type0 = %u, ipc.cmd_type0 = %u, "
                       "ack.cmd_type1 = %u, ipc.cmd_type1 = %u.\n",
                       msg_ret->cmd_type0, ipc->cmd_type0, msg_ret->cmd_type1, ipc->cmd_type1);
        devdrv_drv_err("ack.cmd_type1(%u), ack.cmd_type0(%u), ack.target_id(%u), ack.source_id(%d), "
                       "ack.cmd_para0(%u), ack.cmd_para1(%u), ack.cmd_para2(%u), ack.cmd_para3(%u), "
                       "ack.cmd_data0(%u), ack.cmd_data1(%u), ack.cmd_data2(%u), ack.cmd_data3(%u), "
                       "ack.data1(%u), ack.data2(%u), ack.data3(%u), ack.data4(%u), ack.data5(%u).\n",
                       msg_ret->cmd_type1, msg_ret->cmd_type0, msg_ret->target_id, msg_ret->source_id,
                       msg_ret->cmd_para0, msg_ret->cmd_para1, msg_ret->cmd_para2, msg_ret->cmd_para3,
                       msg_ret->cmd_data0, msg_ret->cmd_data1, msg_ret->cmd_data2, msg_ret->cmd_data3,
                       msg_ret->data1, msg_ret->data2, msg_ret->data3, msg_ret->data4, msg_ret->data5);
        return -EINVAL;
    }

    return ret;
}

static int devdrv_manager_check_lp_msg_validity(struct devdrv_ipc_imu *lp_msg)
{
    u32 lp_error_code;

    /*
    LP_ERRCODE_QUERY_SUCCESS         0
    LP_ERRCODE_QUERY_FAIL            1
    LP_ERRCODE_QUERY_TIMEOUT         2
    LP_ERRCODE_QUERY_PARA_ERR        3
    LP_ERRCODE_QUERY_NONSUPPORT      4
    32-bit error code. The first 16 bits indicate the voltage error code and the last 16 bits indicate the current error code.
    */
    lp_error_code = *(u32 *)(&(lp_msg->cmd_para0));
    if (lp_error_code != 0) {
        devdrv_drv_err("get low_power module message failed. ack.cmd_type0 = %u, ack.cmd_type1 = %u"
            "lp_err_code = 0x%x\n", lp_msg->cmd_type0, lp_msg->cmd_type1, lp_error_code);
        return -EINVAL;
    }

    return 0;
}
#endif

#if (defined CFG_FEATURE_LP_ENABLE) || (defined CFG_SOC_PLATFORM_MDC_V11)
typedef s32 (*check_whitelist_ops)(const char **list_name, u32 name_num);
#define DMS_OPS_CHECK_WHITELIST "whitelist_process_handler"

int devdrv_manager_check_call_process(void)
{
    int ret;
    const char *wl_process_name = CHECK_PROCESS_DMP;
    check_whitelist_ops check_whitelist_func = NULL;

    /* check process name and related bin/so excutable section sha256 */
    check_whitelist_func = (check_whitelist_ops)(uintptr_t)__symbol_get(DMS_OPS_CHECK_WHITELIST);
    if (check_whitelist_func == NULL) {
        devdrv_drv_err("Lookup symbol: %s failed.\n", DMS_OPS_CHECK_WHITELIST);
        return -EINVAL;
    }
    ret = check_whitelist_func(&wl_process_name, 1);
    __symbol_put(DMS_OPS_CHECK_WHITELIST);
    if (ret) {
        devdrv_drv_err("Permission denied! ret = %d.\n", ret);
        return ret;
    }
    return 0;
}
#endif

#ifdef CFG_FEATURE_IMU_ENABLE
STATIC int devdrv_get_ddr_capacity(unsigned long arg)
{
    struct ioctl_arg user_arg = {0};
    int ret;

    /* wait get interface from os */
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -EINVAL;
    }
    return 0;
}

int devdrv_get_ecc_statistics(unsigned long arg)
{
    struct ioctl_arg user_arg = {0};
    u8 buf[ECC_DATA_LEN] = {0};
    u32 buf_len = ECC_DATA_LEN;
    u32 dev_type;
    u32 dev_id;
    int ret;

    ret = memcpy_s(&user_arg, sizeof(struct ioctl_arg), (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("Memcpy failed.\n");
        return ret;
    }

    dev_id = user_arg.dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("invalid device id, id = %u.\n", dev_id);
        return -EINVAL;
    }

    dev_type = user_arg.type;
    if (dev_type == DEVICE_TYPE_DDR) {
        ret = lpm_get_info_from_ipc(dev_id, LPM_ECC_STATISTICS, LPM_DDR_ID, buf, &buf_len);
        if (ret != 0) {
            devdrv_drv_err("Failed to obtain the ecc through the ipc."
                "(dev_id=%u; dev_type=%u; ret=%d)\n", dev_id, dev_type, ret);
            return ret;
        }
    } else if (dev_type == DEVICE_TYPE_HBM) {
        ret = lpm_get_info_from_ipc(dev_id, LPM_ECC_STATISTICS, LPM_HBM_ID, buf, &buf_len);
        if (ret != 0) {
            devdrv_drv_err("Failed to obtain the ecc through the ipc."
                "(dev_id=%u; dev_type=%u; ret=%d)\n", dev_id, dev_type, ret);
            return ret;
        }
    } else {
        devdrv_drv_err("invalid device type.\n");
        return -EINVAL;
    }
    /* ecc single bit */
    user_arg.data1 = *(u32 *)buf;
    /* ecc double bit */
    user_arg.data3 = *(u32 *)&buf[4];

    ret = memcpy_s((void *)((uintptr_t)arg), sizeof(struct ioctl_arg), &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("Memcpy failed.\n");
        return ret;
    }
    return 0;
}
EXPORT_SYMBOL(devdrv_get_ecc_statistics);

STATIC int devdrv_config_ecc_enable(unsigned long arg)
{
    struct ioctl_arg user_arg = {0};
    u32 dev_id;
    int ret;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    dev_id = user_arg.dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("invalid device id, id = %u.\n", dev_id);
        return -EINVAL;
    }

    return 0;
}

#elif defined(CFG_FEATURE_LP_ENABLE)
STATIC int devdrv_ipc_type_decode(u32 type, struct devdrv_ipc_imu *ipc_msg)
{
    ipc_msg->source_id = 0;
    ipc_msg->target_id = OBJ_IMU;
    switch (type)
    {
        case CCPU_FREQ:
            ipc_msg->cmd_type0 = CMD_INQUIRY;
            ipc_msg->cmd_type1 = SUB_CMD_QUERY_CTRLCPUFREQ;
            break;
        case DDR_FREQ:
            ipc_msg->cmd_type0 = CMD_INQUIRY;
            ipc_msg->cmd_type1 = SUB_CMD_QUERY_DDR_FREQUENCY;
            break;
        case AICORE0_FREQ:
            ipc_msg->cmd_type0 = CMD_INQUIRY;
            ipc_msg->cmd_type1 = SUB_CMD_QUERY_AICFREQ;
            break;
        case AICORE1_FREQ:
            ipc_msg->cmd_type0 = CMD_INQUIRY;
            ipc_msg->cmd_type1 = SUB_CMD_QUERY_AICFREQ;
            break;
        case VECTOR_FREQ:
            ipc_msg->cmd_type0 = CMD_INQUIRY;
            ipc_msg->cmd_type1 = SUB_CMD_QUERY_AIVFREQ;
            break;
        case DDR_INIT:
            ipc_msg->cmd_type0 = CMD_INQUIRY;
            ipc_msg->cmd_type1 = SUB_CMD_INIT_DDR_STATISTIC;
            break;
       case DDR_STATISTIC:
           ipc_msg->cmd_type0 = CMD_INQUIRY;
           ipc_msg->cmd_type1 = SUB_CMD_QUERY_DDR_STATISTIC;
           break;
        default:
            devdrv_drv_err("not support this type (%d)\n", type);
            return -1;
    }

    return 0;
}

int devdrv_query_ddr_statistic_from_lp(unsigned int dev_id, unsigned int type, void *out_msg)
{
    int ret;
    struct devdrv_ipc_imu ipc_msg = {0};
    struct devdrv_ipc_imu *ack_msg = (struct devdrv_ipc_imu *)out_msg;
    ret = devdrv_ipc_type_decode(type, &ipc_msg);
    if (ret != 0) {
        devdrv_drv_err("dev(%u) devdrv_ipc_type_decode failed, type = %u, ret = %d.\n", dev_id, type, ret);
        return ret;
    }
    ret = devdrv_get_msg_from_lp(dev_id, &ipc_msg, ack_msg, sizeof(struct devdrv_ipc_imu));
    if (ret != 0) {
        devdrv_drv_err("dev(%u) devdrv_get_msg_from_lp failed, ret = %d.\n", dev_id, ret);
        return ret;
    }
    ret = devdrv_manager_check_lp_msg_validity(ack_msg);
    if (ret != 0) {
        devdrv_drv_err("The message obtained from the lp is invalid. please check lp_err_code.\n");
        return ret;
    }

    return ret;
}
EXPORT_SYMBOL(devdrv_query_ddr_statistic_from_lp);

STATIC int devdrv_get_freq_from_lp(unsigned int dev_id, unsigned int type, unsigned int *freq)
{
    int ret;
    struct devdrv_ipc_imu ipc_msg = {0};
    struct devdrv_ipc_imu ack_msg = {0};

    ret = devdrv_ipc_type_decode(type, &ipc_msg);
    if (ret) {
        devdrv_drv_err("dev(%u) devdrv_ipc_type_decode failed, type = %u, ret = %d.\n", dev_id, type, ret);
        return ret;
    }

    ret = devdrv_get_msg_from_lp(dev_id, &ipc_msg, &ack_msg, sizeof(unsigned int));
    if (ret) {
        devdrv_drv_err("dev(%u) devdrv_get_msg_from_lp failed, ret = %d.\n", dev_id, ret);
        return ret;
    }

    ret = devdrv_manager_check_lp_msg_validity(&ack_msg);
    if (ret != 0) {
        devdrv_drv_err("The message obtained from the lp is invalid. please check lp_err_code.\n");
        return ret;
    }

    *freq = (type != AICORE1_FREQ ? *(u32 *)&ack_msg.cmd_data0 : ack_msg.data1);
    return ret;
}

// export for drv_memory
int devdrv_get_freq_from_lp_memory(unsigned int dev_id, unsigned int type, unsigned int *freq)
{
    if ((dev_id >= MAX_CHIP_NUM) || (freq == NULL)) {
        devdrv_drv_err("Invalid parameter. (dev_id=%d; freq=%pK)\n", dev_id, freq);
        return -EINVAL;
    }

    return devdrv_get_freq_from_lp(dev_id, type, freq);
}
EXPORT_SYMBOL(devdrv_get_freq_from_lp_memory);

STATIC int devdrv_get_ddr_capacity(unsigned long arg)
{
    return 0;
}

int devdrv_get_ecc_statistics(unsigned long arg)
{
    int ret;
    struct ioctl_arg user_arg = {0};
    struct devdrv_ipc_imu ipc_msg = {0};
    struct devdrv_ipc_imu ack_msg = {0};

    if (arg == 0) {
        devdrv_drv_err("Invalid parameter.\n");
        return -EINVAL;
    }
    ret = memcpy_s(&user_arg, sizeof(struct ioctl_arg), (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("Memcpy_s failed, ret = %d.\n", ret);
        return ret;
    }

    ipc_msg.source_id = 0;
    ipc_msg.target_id = OBJ_IMU;
    ipc_msg.cmd_type0 = CMD_INQUIRY;
    ipc_msg.cmd_type1 = SUB_CMD_QUERY_DDR_ECC;

    ret = devdrv_get_msg_from_lp(user_arg.dev_id, &ipc_msg, &ack_msg, sizeof(unsigned int));
    if (ret) {
        devdrv_drv_err("devdrv_get_msg_from_lp failed, ret = %d.\n", ret);
        return ret;
    }

    ret = devdrv_manager_check_lp_msg_validity(&ack_msg);
    if (ret != 0) {
        devdrv_drv_err("The message obtained from the lp is invalid. please check lp_err_code.\n");
        return ret;
    }

    user_arg.data1 = *(u32 *) &ack_msg.cmd_data0;
    user_arg.data3 = ack_msg.data1;
    ret = memcpy_s((void *)((uintptr_t)arg), sizeof(struct ioctl_arg), &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("Memcpy failed, ret = %d.\n", ret);
    }

    return ret;

}
EXPORT_SYMBOL(devdrv_get_ecc_statistics);

STATIC int devdrv_config_ecc_enable(unsigned long arg)
{
    return 0;
}

#else
STATIC int devdrv_get_ddr_capacity(unsigned long arg)
{
    struct dmanage_temp_share_mem *share_mem_base = NULL;
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    struct ioctl_arg user_arg = {0};
    u32 capacity;
    u8 high;
    u8 low;
    int ret;

    dev_info = devdrv_get_default_devdrv_info();
    if (dev_info == NULL) {
        devdrv_drv_err("default dev_info is null, unable to get temperature.\n");
        return -EINVAL;
    }

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    share_mem_base = (struct dmanage_temp_share_mem *)pdata->ts_pdata[0].tsensor_shm_vaddr;

    high = share_mem_base->ddr_capacity_h;
    low = share_mem_base->ddr_capacity_l;
    capacity = (high << OFFSET_EIGHT) | low;
    user_arg.data1 = capacity;

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }

    return 0;
}

STATIC int devdrv_config_ecc_enable(unsigned long arg)
{
    int ret;
    struct ioctl_arg user_arg = {0};

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }
    return 0;
}
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51

int devdrv_manager_equipment_set_safety_island_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;

#ifdef CFG_BUILD_DEBUG
    ret = devdrv_manager_equipment_set_safetyisland_info(filep, cmd, arg);
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_equipment_set_safetyisland_info failed, ret=%d\n", ret);
    }
#else
    ret = -EINVAL;
    devdrv_drv_err("This interface is applicable only to equipment test scenarios\n");
#endif

    return ret;
}

int devdrv_manager_equipment_get_safety_island_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;

#ifdef CFG_BUILD_DEBUG
    ret = devdrv_manager_equipment_get_safetyisland_info(filep, cmd, arg);
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_equipment_get_safetyisland_info failed, ret=%d\n", ret);
    }
#else
    ret = -EINVAL;
    devdrv_drv_err("This interface is applicable only to equipment test scenarios\n");
#endif

    return ret;
}

#endif

STATIC int devdrv_get_llc_perf_para(unsigned long arg)
{
    int ret;
    llc_perf_para_t perf_para_rd = {0};
    llc_perf_para_t perf_para_wr = {0};
    llc_perf_para_t perf_para_total = {0};
    u32 device_id, phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;
    u64 throughput_total;

    ret = copy_from_user_safe(&perf_para_total, (void *)((uintptr_t)arg), sizeof(llc_perf_para_t));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    device_id = perf_para_total.dev_id;
    ret = devdrv_manager_trans_and_check_id(device_id, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", device_id, ret);
        return ret;
    }

    if (phys_id >= devdrv_manager_get_devnum()) {
        devdrv_drv_err("invalid device id, id = %u.\n", phys_id);
        return -EINVAL;
    }

    ret = devdrv_llc_get_perf_para(phys_id, LLC_PROF_TYPE_RD_HIT, &perf_para_rd);
    if (ret != 0) {
        devdrv_drv_err("devdrv_llc_get_perf_para read failed.\n");
        return ret;
    }

    ret = devdrv_llc_get_perf_para(phys_id, LLC_PROF_TYPE_WR_HIT, &perf_para_wr);
    if (ret != 0) {
        devdrv_drv_err("devdrv_llc_get_perf_para write failed.\n");
        return ret;
    }

    perf_para_total.rd_hit_rate = perf_para_rd.rd_hit_rate;
    perf_para_total.wr_hit_rate = perf_para_wr.wr_hit_rate;
    /* Divided by 2 to get the average */
    throughput_total = ((u64)perf_para_rd.throughput + (u64)perf_para_wr.throughput) / 2;
    /* llc throughput max value is 128GB/s for 1 L3D, 512GB/s for 1 device */
    if (throughput_total <= UINT_TYPE_MAX) {
        perf_para_total.throughput = throughput_total;
    } else {
        /* if value is beyond 4G*1024 Byte/s, it is err value */
        devdrv_drv_err("throughput is beyond range of uint.\n");
        return -EINVAL;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &perf_para_total, sizeof(llc_perf_para_t));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }

    return 0;
}

int devdrv_manager_common_status_to_user(const void *status, unsigned int status_size,
    unsigned long user_arg)
{
    int ret;
    common_status_info_t common_status = {0};
    ret = copy_from_user_safe(&common_status, (void *)((uintptr_t)user_arg), sizeof(common_status_info_t));
    if (ret) {
        devdrv_drv_err("copy common_status failed, ret = %d\n", ret);
        return ret;
    }

    if (common_status.commoninfo_len < status_size) {
        devdrv_drv_err("struct length Not compatible, input lenght = %d, need length %d\n",
            common_status.commoninfo_len, status_size);
        return -EINVAL;
    }

    ret = copy_to_user_safe(common_status.commoninfo, status, status_size);
    if (ret) {
        devdrv_drv_err("copy commoninfo failed, ret = %d\n", ret);
        return ret;
    }

    common_status.commoninfo_len = status_size;
    ret = copy_to_user_safe((void *)(uintptr_t)user_arg, &common_status, sizeof(common_status_info_t));
    if (ret) {
        devdrv_drv_err("copy input_arg failed, ret = %d\n", ret);
        return ret;
    }

    return ret;
}

#ifdef CFG_FEATURE_HISS

typedef int (*dms_get_hiss_status_ops)(uint32_t dev_id, hsm_status_t *hsm_status);
typedef int (*dms_get_flash_info)(uint32_t dev_id, uint32_t flash_idx, sec_flash_info_t *flash_info);
typedef int (*dms_get_flash_cnt)(uint32_t dev_id, uint32_t *flash_cnt);
#define DMS_OPS_GET_HISS_STATUS "hsm_get_hiss_status"
#define DMS_OPS_GET_FLASH_INFO "sec_flash_get_info"
#define DMS_OPS_GET_FLASH_COUNT "sec_flash_get_count"

int devdrv_manager_get_hiss_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    hsm_status_t hiss_status = {.hsm_status_code = HSM_NOT_INITIALIZED, .hsm_status_info = 0};
    common_status_info_t common_status = {0};
    dms_get_hiss_status_ops get_hisi_status_func = NULL;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&common_status, (void *)((uintptr_t)arg), sizeof(common_status_info_t));
    if (ret != 0) {
        devdrv_drv_err("copy common_status failed, ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_trans_and_check_id(common_status.dev_id, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", common_status.dev_id, ret);
        return ret;
    }

    get_hisi_status_func = (dms_get_hiss_status_ops)(uintptr_t)__symbol_get(DMS_OPS_GET_HISS_STATUS);
    if (get_hisi_status_func == NULL) {
        devdrv_drv_err("Lookup symbol: %s failed.\n", DMS_OPS_GET_HISS_STATUS);
        return -EINVAL;
    }
    ret = get_hisi_status_func(phys_id, &hiss_status);
    __symbol_put(DMS_OPS_GET_HISS_STATUS);
    if (ret != 0) {
        devdrv_drv_err("invorker hsm interface : hsm_get_hiss_status failed, ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_common_status_to_user(&hiss_status,
        sizeof(hsm_status_t), arg);
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_common_status_to_user failed, ret = %d\n", ret);
        return ret;
    }

    return ret;
}

int devdrv_manager_get_flash_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    sec_flash_info_t sec_flash_info = {0};
    common_status_info_t common_status = {0};
    dms_get_flash_info get_flash_info_func = NULL;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&common_status, (void *)((uintptr_t)arg), sizeof(common_status_info_t));
    if (ret) {
        devdrv_drv_err("copy common_status failed, ret = %d\n", ret);
        return ret;
    }

    if (common_status.commoninfo_len != sizeof(sec_flash_info_t)) {
        devdrv_drv_err("invalid paramenter length, input length = %d, valid lenght = %ld\n",
            common_status.commoninfo_len, sizeof(sec_flash_info_t));
        return -EINVAL;
    }

    ret = copy_from_user_safe(&sec_flash_info, common_status.commoninfo, common_status.commoninfo_len);
    if (ret) {
        devdrv_drv_err("copy commoninfo failed, ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_trans_and_check_id(common_status.dev_id, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", common_status.dev_id, ret);
        return ret;
    }

    get_flash_info_func = (dms_get_flash_info)(uintptr_t)__symbol_get(DMS_OPS_GET_FLASH_INFO);
    if (get_flash_info_func == NULL) {
        devdrv_drv_err("Lookup symbol: %s failed.\n", DMS_OPS_GET_FLASH_INFO);
        return -EINVAL;
    }

    ret = get_flash_info_func(phys_id, sec_flash_info.flash_id, &sec_flash_info);
    __symbol_put(DMS_OPS_GET_FLASH_INFO);
    if (ret != 0) {
        devdrv_drv_err("sec_flash_get_info failed, ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_common_status_to_user(&sec_flash_info, sizeof(sec_flash_info_t), arg);
    if (ret) {
        devdrv_drv_err("devdrv_manager_common_status_to_user failed, ret = %d\n", ret);
        return ret;
    }
    return ret;
}

int devdrv_get_flash_count(unsigned long arg)
{
    int ret;
    unsigned int flash_count = 0;
    struct ioctl_arg user_arg = {0};
    dms_get_flash_cnt get_flash_cnt_func = NULL;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret != 0) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    ret = devdrv_manager_trans_and_check_id(user_arg.dev_id, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n",
            user_arg.dev_id, ret);
        return ret;
    }

    get_flash_cnt_func = (dms_get_flash_cnt)(uintptr_t)__symbol_get(DMS_OPS_GET_FLASH_COUNT);
    if (get_flash_cnt_func == NULL) {
        devdrv_drv_err("Lookup symbol: %s failed.\n", DMS_OPS_GET_FLASH_COUNT);
        return -EINVAL;
    }

    ret = get_flash_cnt_func(phys_id, &flash_count);
    __symbol_put(DMS_OPS_GET_FLASH_COUNT);
    if (ret != 0) {
        devdrv_drv_err("sec_flash_get_count failed.\n");
        return ret;
    }

    user_arg.data1 = flash_count;
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &user_arg, sizeof(struct ioctl_arg));
    if (ret != 0) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }

    return ret;
}
#endif

#if defined(CFG_FEATURE_LP_ENABLE)
STATIC int devdrv_manager_status_check_device_id(common_status_info_t *common_status,
                                                 unsigned long user_arg)
{
    int ret;
    ret = copy_from_user_safe(common_status, (void *)((uintptr_t)user_arg), sizeof(common_status_info_t));
    if (ret != 0) {
        devdrv_drv_err("copy common_status failed, ret = %d\n", ret);
        return ret;
    }
    if (common_status->dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) {
        devdrv_drv_err("Invalid devid(%u)\n", common_status->dev_id);
        return -EINVAL;
    }
    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
__attribute__((weak)) void devdrv_manager_get_bist_mode(struct devdrv_ipc_imu *ipc_msg)
{
    struct dms_bist_mode bist_mode;
    BIST_VECTOR_INFO *bist_vector_info = NULL;

    (void)dms_get_bist_mode(&bist_mode);
    dms_clear_bist_mode();      // bist_mode在使能BIST流程获取后会清空，下一次使能BIST前需要重新配置
    bist_vector_info = dms_get_bist_vector_info_addr();

    ipc_msg->cmd_para0 = (uint8_t)bist_mode.seconds;
    ipc_msg->cmd_para1 = (uint8_t)bist_mode.bist_flag;
    ipc_msg->cmd_para2 = (uint8_t)bist_mode.start_vector_id;
    ipc_msg->cmd_para3 = (uint8_t)bist_vector_info->lp_bist_vec_offset;
    ipc_msg->cmd_data0 = (uint8_t)bist_vector_info->vector_cnt;

    devdrv_drv_info("bist info: seconds=%u, bist_flag=%u, start_vector_id=%u, lp_bist_vec_offset=%u, vector_cnt=%u.\n",
        bist_mode.seconds,
        bist_mode.bist_flag,
        bist_mode.start_vector_id,
        bist_vector_info->lp_bist_vec_offset,
        bist_vector_info->vector_cnt);
    return;
}

STATIC int devdrv_manager_get_file_size(struct file *filp, unsigned int *file_size)
{
    if (filp->f_inode == NULL) {
        devdrv_drv_err("file inode is NULL.\n");
        return -EINVAL;
    }
    *file_size = filp->f_inode->i_size;
    return OK;
}

STATIC int devdrv_manager_read_file(struct file *filp, loff_t offset, char *buf, unsigned int size)
{
    loff_t offset_tmp = offset;
    ssize_t result;
    void *file_buf = vmalloc(size + 1);

    if (file_buf == NULL) {
        devdrv_drv_err("vmalloc for read file error.\n");
        return -EINVAL;
    }

    result = kernel_read(filp, file_buf, size, &offset_tmp);
    if (result != size) {
        devdrv_drv_err("kernel read file error, result=%ld.\n", result);
        vfree(file_buf);
        return -EINVAL;
    }

    result = memcpy_s((void *)buf, size, (void *)file_buf, size);
    if (result != 0) {
        devdrv_drv_err("memcpy file data error, result=%ld.\n", result);
        vfree(file_buf);
        return -EINVAL;
    }

    vfree(file_buf);
    return OK;
}

STATIC int devdrv_manager_copy_file_to_mem(const char *file_path, char *dest_addr, long length)
{
    int ret;
    unsigned int file_size = 0;
    struct file *filp = NULL;

    filp = filp_open(file_path, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(filp)) {
        devdrv_drv_err("unable to open file: %s (%ld)\n", file_path, PTR_ERR(filp));
        return -EINVAL;
    }

    ret = devdrv_manager_get_file_size(filp, &file_size);
    if (ret != OK) {
        devdrv_drv_err("get file size failed, ret=%d.\n", ret);
        filp_close(filp, NULL);
        return ret;
    }

    if (file_size > length) {
        filp_close(filp, NULL);
        devdrv_drv_err("file size[%u] is out of range %ld.\n", file_size, length);
        return -ERANGE;
    }

    ret = devdrv_manager_read_file(filp, 0, dest_addr, file_size);
    if (ret != OK) {
        filp_close(filp, NULL);
        devdrv_drv_err("read file to mem failed, ret: %d\n", ret);
        return ret;
    }

    filp_close(filp, NULL);

    devdrv_drv_info("read file to mem finished, file_len=0x%x.\n", file_size);
    return OK;
}

STATIC int devdrv_manager_copy_bist_image(void)
{
    int ret = OK;
    char *addr = NULL;

    addr = ioremap_wc((uint64_t)DSMI_DMP_MBIST_REGION_ADDR, DSMI_DMP_MBIST_REGION_LEN);
    if (addr == NULL) {
        devdrv_drv_err("ioremap for mbist file failed!\n");
        return -EINVAL;
    }

    ret = memset_s(addr, DSMI_DMP_MBIST_REGION_LEN, 0, DSMI_DMP_MBIST_REGION_LEN);
    if (ret != OK) {
        devdrv_drv_err("memset mbist memory failed! ret:%d.\n", ret);
    }

    ret = devdrv_manager_copy_file_to_mem(DSMI_DMP_MBIST_FILE, addr, DSMI_DMP_MBIST_REGION_LEN);
    if (ret != OK) {
        iounmap(addr);
        devdrv_drv_err("copy mbist file failed! ret:%d.\n", ret);
        return ret;
    }

    iounmap(addr);
    devdrv_drv_info("copy mbist file to ram success.\n");

    return ret;
}

__attribute__((weak)) void devdrv_manager_bist_prepare(void)
{
    (void)devdrv_manager_copy_bist_image();
    dms_clear_bist_rslt_magic();
    return;
}
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC void devdrv_manager_notify_lp_resume(unsigned int dev_id)
{
    int ret;
    struct devdrv_ipc_imu ipc_msg = {0};
    struct timespec stamp;

    ipc_msg.target_id = LPR52_TARGET_ID;
    ipc_msg.source_id = LPR52_SOURECE_ID;
    ipc_msg.cmd_type0 = CMD_NOTIFY;
    ipc_msg.cmd_type1 = SUB_CMD_NOTIFY_LP_RESUME;

    stamp = current_kernel_time();
    ipc_msg.data1 = ((unsigned long)stamp.tv_sec) & 0xFFFFFFFF;            // seconds low 32 bits
    ipc_msg.data2 = (((unsigned long)stamp.tv_sec) >> 32) & 0xFFFFFFFF;  // seconds high 32 bits
    ipc_msg.data3 = ((unsigned long)stamp.tv_nsec) & 0xFFFFFFFF;          // nanoseconds  low 32 bits
    ipc_msg.data4 = (((unsigned long)stamp.tv_nsec) >> 32) & 0xFFFFFFFF; // nanoseconds high 32 bits

    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, HISI_RPROC_LP_Q_TX_RPID4_ACPU2),
        (rproc_msg_t *)&ipc_msg, sizeof(ipc_msg) / sizeof(rproc_msg_len_t));
    if (ret != 0) {
        devdrv_drv_warn("icm_msg_send_async not success, ret = %d.\n", ret);
        return;
    }

    return;
}
#endif

STATIC void devdrv_manager_notify_lp_suspend(unsigned int dev_id, DSMI_POWER_STATE type)
{
    int ret;
    struct devdrv_ipc_imu ipc_msg = {0};
    struct timespec stamp;

    ipc_msg.target_id = LPR52_TARGET_ID;
    ipc_msg.source_id = LPR52_SOURECE_ID;
    ipc_msg.cmd_type0 = CMD_NOTIFY;

    if (type == POWER_STATE_BIST) {
        ipc_msg.cmd_type1 = SUB_CMD_NOTIFY_LP_BIST;
        devdrv_manager_bist_prepare();
        devdrv_manager_get_bist_mode(&ipc_msg);
    } else {
        ipc_msg.cmd_type1 = SUB_CMD_NOTIFY_LP_SUSPEND;
    }

    stamp = current_kernel_time();
    ipc_msg.data1 = ((unsigned long)stamp.tv_sec) & 0xFFFFFFFF;            // seconds low 32 bits
    ipc_msg.data2 = (((unsigned long)stamp.tv_sec) >> 32) & 0xFFFFFFFF;  // seconds high 32 bits
    ipc_msg.data3 = ((unsigned long)stamp.tv_nsec) & 0xFFFFFFFF;          // nanoseconds  low 32 bits
    ipc_msg.data4 = (((unsigned long)stamp.tv_nsec) >> 32) & 0xFFFFFFFF; // nanoseconds high 32 bits

    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, HISI_RPROC_LP_Q_TX_RPID4_ACPU2),
        (rproc_msg_t *)&ipc_msg, sizeof(ipc_msg) / sizeof(rproc_msg_len_t));
    if (ret != OK) {
        devdrv_drv_warn("icm_msg_send_async not success, ret = %d.\n", ret);
    }

    return;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
void devdrv_manager_print_suspend_stats(void)
{
    int i;
    devdrv_drv_event("suspend_stats:\n"
        "%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n"
        "%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n",
        "success", suspend_stats.success,
        "f", suspend_stats.fail,
        "f_freeze", suspend_stats.failed_freeze,
        "f_prepare", suspend_stats.failed_prepare,
        "f_suspend", suspend_stats.failed_suspend,
        "f_suspend_late", suspend_stats.failed_suspend_late,
        "f_suspend_noirq", suspend_stats.failed_suspend_noirq,
        "f_resume", suspend_stats.failed_resume,
        "f_resume_early", suspend_stats.failed_resume_early,
        "f_resume_noirq", suspend_stats.failed_resume_noirq);
    devdrv_drv_event("f_dev_info:\n");
    for (i = 0; i < REC_FAILED_NUM; i++) {
        devdrv_drv_event("%s\n", suspend_stats.failed_devs[i]);
    }
}
#endif

#define SUSPEND_DELAY_10_MS 10
typedef bool (*pm_get_wakeup_count_call)(unsigned int *count, bool block);
typedef bool (*pm_save_wakeup_count_call)(unsigned int count);
int devdrv_suspend_proc(unsigned int dev_id)
{
    int ret = 0;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    unsigned int initial_count = 0;
    pm_get_wakeup_count_call get_wakeup_count = NULL;
    pm_save_wakeup_count_call save_wakeup_count = NULL;

    get_wakeup_count = (pm_get_wakeup_count_call)(uintptr_t)__kallsyms_lookup_name("pm_get_wakeup_count");
    if (get_wakeup_count != NULL) {
        if (!get_wakeup_count(&initial_count, true)) {
            devdrv_drv_err("get_wakeup_count failed!\n");
            return -EIO;
        }
    }
    devdrv_drv_event("pm_get_wakeup_count succeed.\n");
    mutex_lock(&g_suspend_lock);
    save_wakeup_count = (pm_save_wakeup_count_call)(uintptr_t)__kallsyms_lookup_name("pm_save_wakeup_count");
    if (save_wakeup_count != NULL) {
        if (!save_wakeup_count(initial_count)) {
            mutex_unlock(&g_suspend_lock);
            devdrv_drv_err("save_wakeup_count failed!\n");
            return -EIO;
        }
    }
    devdrv_drv_event("invoke os interface pm_suspend start.\n");
    msleep(SUSPEND_DELAY_10_MS); // Delay of 10 ms to ensure that logs at the invocation boundary are saved to the disk.
    ret = pm_suspend(PM_SUSPEND_MEM);
    if (ret != 0) {
        devdrv_drv_err("invoke os interface pm_suspend failed, ret(%d).\n", ret);
    } else {
        devdrv_drv_event("invoke os interface pm_suspend end.\n");
    }

    devdrv_manager_notify_lp_resume(dev_id);
    mutex_unlock(&g_suspend_lock);
#ifdef CFG_SOC_PLATFORM_MDC_V51
    devdrv_manager_print_suspend_stats();
#endif
#endif
    return ret;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int devdrv_manager_set_power_state_bist_pre_check(void)
{
    int ret;
    int chip_type;
    BIST_MODE_STRU *bist_mode = dms_get_bist_mode_addr();

    ret = devdrv_manager_get_chip_type(&chip_type);
    if (ret != OK) {
        devdrv_drv_err("get chip type failed! ret = %d\n", ret);
        return ret;
    }

    if ((chip_type != CHIP_TYPE_ASCEND_V2) && (chip_type != CHIP_TYPE_ASCEND_V51_LITE)) {
        devdrv_drv_err("chip type error, can't support bist, type=%d.\n", chip_type);
        return -EOPNOTSUPP;
    }

    /* 判断是否配置BIST参数，若没有配置则返回失败，无法使能BIST流程 */
    if ((bist_mode->seconds == 0) && (bist_mode->bist_flag == 0) && (bist_mode->start_vector_id == 0)) {
        devdrv_drv_err("bist mode is invalid, please set bist mode before enable bist process.\n");
        return -EINVAL;
    }

    return OK;
}
#endif

STATIC int devdrv_check_ts_id(int ts_id)
{
    if (ts_id < 0) {
        devdrv_drv_err("ts id %d is invalid,ts id must big than 0\n", ts_id);
        return -EINVAL;
    }
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (ts_id >= DEVDRV_TS_MAX) {
        devdrv_drv_err("ts id %d is invalid,in dc ts id must less than %d\n", ts_id, DEVDRV_TS_MAX);
        return -EINVAL;
    }
#else
    if (ts_id != DEVDRV_TS_AICORE) {
        devdrv_drv_err("ts id %d is invalid,in mdc v51 ts id must euqal %d ts aicore\n", ts_id, DEVDRV_TS_AICORE);
        return -EINVAL;
    }
#endif
    return 0;
}

int devdrv_manager_get_ts_group_num(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    common_status_info_t common_status = {0};
    unsigned int ts_group_num = DEVDRV_TS_GROUP_NUM;
    unsigned int ts_id;

    ret = devdrv_manager_status_check_device_id(&common_status, arg);
    if (ret != 0) {
        devdrv_drv_err("check device id fail ret = %d\n", ret);
        return ret;
    }

    ts_id = common_status.reserver[GET_GROUP_TS_ID_RES_INDEX];
    ret = devdrv_check_ts_id(ts_id);
    if (ret != 0) {
        devdrv_drv_err("ts id %d is invalid\n", ts_id);
        return ret;
    }

    ret = devdrv_manager_common_status_to_user(&ts_group_num, sizeof(unsigned int), arg);
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_common_status_to_user failed, ret = %d\n", ret);
        return ret;
    }

    return ret;
}

int devdrv_get_ts_spec_group_info(unsigned int device_id, int ts_id, int group_id,
                                  struct ts_group_info *group_info)
{
    int ret;
    struct ts_ipcdrv_message req_msg = {0};
    struct ts_ipcdrv_message rsp_msg = {0};
    struct ipc_rsp_ts_group_info *rsp_group_info = NULL;

    req_msg.ipcdrv_payload[0] = group_id;

    ret = tsmng_group_getinfo_from_ts_sync(device_id, ts_id, &req_msg, &rsp_msg);
    if (ret != 0) {
        devdrv_drv_err("send get ts group info msg failed, ret = %d, device id = %d, ts id=%d\n",
                       ret, device_id, ts_id);
        return ret;
    }

    if (sizeof(struct ipc_rsp_ts_group_info) > sizeof(rsp_msg.ipcdrv_payload)) {
        devdrv_drv_err("sizeof(struct ipc_rsp_ts_group_info) %ld > sizeof(rsp_msg.ipcdrv_payload) %ld\n",
                       sizeof(struct ipc_rsp_ts_group_info), sizeof(rsp_msg.ipcdrv_payload));
        return -EINVAL;
    }
    rsp_group_info = (struct ipc_rsp_ts_group_info *)rsp_msg.ipcdrv_payload;
    group_info->group_id = rsp_group_info->group_id;
    group_info->state = rsp_group_info->state;
    group_info->extend_attribute = rsp_group_info->extend_attribute;
    group_info->aicore_number = rsp_group_info->aicore_number;
    group_info->aivector_number = rsp_group_info->aivector_number;
    group_info->sdma_number = rsp_group_info->sdma_number;
    group_info->aicpu_number = rsp_group_info->aicpu_number;
    group_info->active_sq_number = rsp_group_info->active_sq_number;
    group_info->aicore_mask = rsp_group_info->aicore_mask;

    return 0;
}

STATIC int devdrv_get_ts_group_info_check_para(struct get_ts_group_para *group_para,
                                               struct ts_group_info *group_info)
{
    int ret;
    int group_id;
    int group_count;

    if (group_para == NULL) {
        devdrv_drv_err("group_para is null\n");
        return -EINVAL;
    }

    if (group_info == NULL) {
        devdrv_drv_err("group_info is null\n");
        return -EINVAL;
    }
    ret = devdrv_check_ts_id(group_para->ts_id);
    if (ret != 0) {
        devdrv_drv_err("ts id %d is invalid\n", group_para->ts_id);
        return ret;
    }

    group_id = group_para->group_id;
    group_count = group_para->group_count;

    if (group_id == -1 && group_count < DEVDRV_TS_GROUP_NUM) {
        devdrv_drv_err("group_id == -1 but group_count %d != %d\n", group_count, DEVDRV_TS_GROUP_NUM);
        return -EINVAL;
    } else if (group_id != -1 && group_count < 1) {
        devdrv_drv_err("group_id != -1 but group_count %d != 1\n", group_count);
        return -EINVAL;
    } else if (group_id >= DEVDRV_TS_GROUP_NUM) {
        devdrv_drv_err("group_id %d must less than %d\n", group_id, DEVDRV_TS_GROUP_NUM);
        return -EINVAL;
    } else {
        return 0;
    }
}

int devdrv_get_ts_group_info(struct get_ts_group_para *group_para,
                             struct ts_group_info *group_info, int group_info_num)
{
    int i = 0;
    int ret;
    unsigned int device_id = group_para->device_id;
    int ts_id = group_para->ts_id;
    int group_id = group_para->group_id;

    ret = devdrv_get_ts_group_info_check_para(group_para, group_info);
    if (ret != 0) {
        devdrv_drv_err("para check fail ret = %d\n", ret);
        return ret;
    }
    // get specific info
    if (group_id != -1) {
        ret = devdrv_get_ts_spec_group_info(device_id, ts_id, group_id, group_info);
        if (ret != 0) {
            devdrv_drv_err("get ts group info fail device_id = %d, ts_id  = %d, group_id  = %d\n",
                           group_para->device_id, group_para->ts_id, group_id);
            return ret;
        }
        return 0;
    }
    if (group_info_num < DEVDRV_TS_GROUP_NUM) {
        devdrv_drv_err("group info num = %d, less than %d\n", group_info_num, DEVDRV_TS_GROUP_NUM);
        return -EINVAL;
    }
    // get all group info
    for (i = 0; i < DEVDRV_TS_GROUP_NUM; i++) {
        ret = devdrv_get_ts_spec_group_info(device_id, ts_id, i, &group_info[i]);
        if (ret != 0) {
            devdrv_drv_err("get ts group info fail device_id = %d, ts_id  = %d, group_id  = %d\n",
                           group_para->device_id, group_para->ts_id, i);
            return ret;
        }
    }
    return 0;
}

STATIC int devdrv_manager_comm_get_user_info(common_status_info_t *common_status,
                                             void *info, unsigned int info_len, unsigned long arg)
{
    int ret;
    if (common_status == NULL || info == NULL) {
        devdrv_drv_err("para is null\n");
        return -EINVAL;
    }
    ret = copy_from_user_safe(common_status, (void *)((uintptr_t)arg), sizeof(common_status_info_t));
    if (ret != 0) {
        devdrv_drv_err("copy common_status failed, ret = %d\n", ret);
        return ret;
    }
    if (common_status->dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) {
        devdrv_drv_err("Invalid devid(%u)\n", common_status->dev_id);
        return -EINVAL;
    }
    if (common_status->commoninfo_len != info_len) {
        devdrv_drv_err("common_status.commoninfo_len = %d not equal %ld\n",
                       common_status->commoninfo_len, sizeof(common_status_info_t));
        return -EINVAL;
    }
    ret = copy_from_user_safe(info, common_status->commoninfo, common_status->commoninfo_len);
    if (ret) {
        devdrv_drv_err("copy commoninfo failed, ret = %d\n", ret);
        return ret;
    }
    return 0;
}

#ifndef CFG_SOC_PLATFORM_MDC_V51
unsigned int map_ts_grp_oper_errcode[GROUP_OPER_ERROR_MAX] = {
    [GROUP_OPER_SUCCESS]     = DRV_ERROR_NONE,
    [GROUP_OPER_COMMON_ERROR] = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_INPUT_DATA_NULL]  = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_GROUP_ID_INVALID]   = DRV_ERROR_PARA_ERROR,
    [GROUP_OPER_STATE_ILLEGAL]   = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_NO_MORE_GROUP_CREATE]     = DRV_ERROR_NO_RESOURCES,
    [GROUP_OPER_NO_LESS_GROUP_DELETE]   = DRV_ERROR_NOT_EXIST,
    [GROUP_OPER_DEFAULT_GROUP_ALREADY_CREATE]   = DRV_ERROR_REPEATED_INIT,
    [GROUP_OPER_NO_MORE_VALID_AICORE_CREATE] = DRV_ERROR_NO_RESOURCES,
    [GROUP_OPER_NO_MORE_VALID_AIVECTOR_CREATE]   = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_NO_MORE_VALID_SDMA_CREATE]  = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_NO_MORE_VALID_AICPU_CREATE] = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_NO_MORE_VALID_ACTIVE_SQ_CREATE]   = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_NO_LESS_VALID_AICORE_DELETE]  = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_NO_LESS_VALID_AIVECTOR_DELETE] = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_NO_LESS_VALID_SDMA_DELETE]   = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_NO_LESS_VALID_AICPU_DELETE]  = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_NO_LESS_VALID_ACTIVE_SQ_DELETE] = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_BUILDIN_GROUP_NOT_CREATE]   = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_SPECIFY_GROUPID_ALREADY_CREATE]  = DRV_ERROR_REPEATED_INIT,
    [GROUP_OPER_SPECIFY_GROUPID_NOT_CREATE] = DRV_ERROR_NOT_EXIST,
    [GROUP_OPER_DISABLE_HWTS_ALLGROUP_FAILED]   = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_AICORE_POOL_FULL]  = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_AIVECTOR_POOL_FULL] = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_SDMA_POOL_FULL]   = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_AICPU_POOL_FULL]  = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_ACTIVE_SQ_POOL_FULL] = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_CREATE_NULL_GROUP]   = DRV_ERROR_PARA_ERROR,
    [GROUP_OPER_CREATE_NULL_SQ_GROUP]   = DRV_ERROR_PARA_ERROR,
    [GROUP_OPER_VM_CONFIG_NOT_INIT]   = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_VM_CONFIG_FAILD]  = DRV_ERROR_INNER_ERR,
    [GROUP_OPER_DELETE_GROUP_STREAM_RUNNING]   = DRV_ERROR_BUSY,
    [GROUP_OPER_CREATE_NOT_SUPPORT_IN_DC]   = DRV_ERROR_NOT_SUPPORT,
};

void devdrv_map_ts_grp_oper_err_to_user(unsigned int ts_errcode, void *usr_arg)
{
    unsigned int drv_errcode = DRV_ERROR_INNER_ERR;
    int ret;

    if (ts_errcode < GROUP_OPER_ERROR_MAX) {
        drv_errcode = map_ts_grp_oper_errcode[ts_errcode];
    }

    ret = copy_to_user_safe(usr_arg, &drv_errcode, sizeof(drv_errcode));
    if (ret) {
        devdrv_drv_warn("Usr errcode may not accurate. (ret=%d; ts_errcode=%u; drv_errcode=%u)\n",
            ret, ts_errcode, drv_errcode);
    }
}
#endif

int devdrv_manager_get_capability_group_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct ts_group_info group_info[DEVDRV_TS_GROUP_NUM] = {0};
    struct get_ts_group_para group_para = {0};
    common_status_info_t common_status = {0};
    int group_count;
    int return_num;
    int group_id;
    int ts_id;
    unsigned int phy_id = 0;
    unsigned int vfid = 0;
    int ret;

    #ifdef CFG_SOC_PLATFORM_MDC_V51
    if (devdrv_manager_ts_is_enable() == false) {
        devdrv_drv_info("can't find ts node, check whether the TS subsystem is deployed in the system.\n");
        return -EOPNOTSUPP;
    }
    #endif
    ret = devdrv_manager_status_check_device_id(&common_status, arg);
    if (ret != 0) {
        devdrv_drv_err("check device id fail ret = %d\n", ret);
        return ret;
    }
    group_count = common_status.reserver[GET_GROUP_COUNT_RES_INDEX];
    ts_id = common_status.reserver[GET_GROUP_TS_ID_RES_INDEX];
    group_id = common_status.reserver[GET_GROUP_ID_RES_INDEX];

    ret = devdrv_manager_container_logical_id_to_physical_id(common_status.dev_id, &phy_id, &vfid);
    if (ret != 0) {
        devdrv_drv_err("can't get phys device id. virt id is %u, ret = %d\n", common_status.dev_id, ret);
        return -EINVAL;
    }

    group_para.device_id = phy_id;
    group_para.ts_id = ts_id;
    group_para.group_id = group_id;
    group_para.group_count = group_count;

    ret = devdrv_get_ts_group_info(&group_para, group_info, DEVDRV_TS_GROUP_NUM);
    if (ret != 0) {
        devdrv_drv_err("get group info fail, ret = %d\n", ret);
        return ret;
    }
    return_num = (group_id == -1 ? DEVDRV_TS_GROUP_NUM : 1);
    ret = devdrv_manager_common_status_to_user(group_info, return_num * sizeof(struct ts_group_info), arg);
    if (ret) {
        devdrv_drv_err("devdrv_manager_common_status_to_user failed, ret = %d\n", ret);
        return ret;
    }
    return 0;
}

int devdrv_manager_delete_capability_group(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int ts_id;
    struct ts_ipcdrv_message req_msg = {0};
    struct ts_ipcdrv_message rsp_msg = {0};
    common_status_info_t common_status = {0};
    struct delete_ts_group_info delete_info = {0};
    struct ipc_operate_group_rsp_payload *rsp_payload = NULL;
    struct ipc_operate_group_req_payload *req_del_payload = NULL;

    #ifdef CFG_SOC_PLATFORM_MDC_V51
    if (devdrv_manager_ts_is_enable() == false) {
        devdrv_drv_info("can't find ts node, check whether the TS subsystem is deployed in the system.\n");
        return -EOPNOTSUPP;
    }
    #endif
    /* check process whitelist */
    ret = devdrv_manager_check_call_process();
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_check_call_process ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_comm_get_user_info(&common_status, &delete_info, sizeof(struct delete_ts_group_info), arg);
    if (ret != 0) {
        devdrv_drv_err("get info from user fail ret = %d\n", ret);
        return ret;
    }
    ts_id = delete_info.ts_id;
    ret = devdrv_check_ts_id(ts_id);
    if (ret != 0) {
        devdrv_drv_err("ts id %d is invalid\n", ts_id);
        return ret;
    }

    if (sizeof(struct ipc_operate_group_req_payload) > sizeof(req_msg.ipcdrv_payload)) {
        devdrv_drv_err("payload %ld is more than  = %ld\n", sizeof(struct ipc_operate_group_req_payload),
                       sizeof(req_msg.ipcdrv_payload));
        return -EINVAL;
    }
    req_del_payload = (struct ipc_operate_group_req_payload *)req_msg.ipcdrv_payload;
    req_del_payload->group_id = delete_info.group_id;
    req_del_payload->operate_flag = TS_GROUP_OPERATE_DELETE;
    ret = tsmng_group_operate_to_ts_sync(common_status.dev_id, ts_id, &req_msg, &rsp_msg);
    if (ret != 0) {
        devdrv_drv_err("send delete group msg failed, ret = %d, device id = %d, ts id=%d\n",
                       ret, common_status.dev_id, ts_id);
        return ret;
    }
    rsp_payload = (struct ipc_operate_group_rsp_payload *)rsp_msg.ipcdrv_payload;
    if (rsp_payload->result != 0) {
#ifndef CFG_SOC_PLATFORM_MDC_V51
        struct delete_ts_group_info *usr_arg = (struct delete_ts_group_info __user*)(common_status.commoninfo);
        devdrv_map_ts_grp_oper_err_to_user(rsp_payload->result, &usr_arg->result);
#endif
        devdrv_drv_err("get ts delete group response, but ts return fail rsp_msg->ipcdrv_payload.result = %d,"
                       "ts id = %d, group id = %d\n", rsp_payload->result, ts_id, delete_info.group_id);
        return -EBADRQC;
    }
    return 0;
}

int devdrv_manager_create_capability_group(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    int ts_id;
    struct ts_group_info group_info = {0};
    common_status_info_t common_status = {0};
    struct ts_ipcdrv_message req_msg = {0};
    struct ts_ipcdrv_message rsp_msg = {0};
    struct ipc_operate_group_req_payload *req_payload = NULL;
    struct ipc_operate_group_rsp_payload *rsp_payload = NULL;
    struct devdrv_info *dev_info = devdrv_get_default_devdrv_info();

    #ifdef CFG_SOC_PLATFORM_MDC_V51
    if (devdrv_manager_ts_is_enable() == false) {
        devdrv_drv_info("can't find ts node, check whether the TS subsystem is deployed in the system.\n");
        return -EOPNOTSUPP;
    }
    #endif

    /* check process whitelist */
    ret = devdrv_manager_check_call_process();
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_check_call_process ret = %d\n", ret);
        return ret;
    }

    if (dev_info == NULL) {
        devdrv_drv_err("dev info is null. maybe the device manager not init ready.\n");
        return -ENODEV;
    }

    ret = devdrv_manager_comm_get_user_info(&common_status, &group_info, sizeof(struct ts_group_info), arg);
    if (ret != 0) {
        devdrv_drv_err("get info from user fail ret = %d\n", ret);
        return ret;
    }

    if (sizeof(struct ipc_operate_group_req_payload) > sizeof(req_msg.ipcdrv_payload)) {
        devdrv_drv_err("payload %ld is more than  = %ld\n", sizeof(struct ipc_operate_group_req_payload),
                       sizeof(req_msg.ipcdrv_payload));
        return -EINVAL;
    }

    devdrv_drv_debug("group info (aicore_num=%u, aivector_num=%u, aicpu_number=%u);"
        "dev info (aicore_num=%u, aivector_num=%u, aicpu_number=%u)\n",
        group_info.aicore_number, group_info.aivector_number, group_info.aicpu_number,
        dev_info->ai_core_num, dev_info->vector_core_num, dev_info->ai_cpu_core_num);

    req_payload = (struct ipc_operate_group_req_payload *)req_msg.ipcdrv_payload;
    req_payload->group_id = group_info.group_id;
    req_payload->operate_flag = TS_GROUP_OPERATE_CREATE;
    req_payload->extend_attribute = group_info.extend_attribute;
    req_payload->aicore_number = group_info.aicore_number;
    req_payload->aivector_number = group_info.aivector_number;
    req_payload->sdma_number = group_info.sdma_number;
    req_payload->aicpu_number = group_info.aicpu_number;
    req_payload->active_sq_number = group_info.active_sq_number;

    ts_id = common_status.reserver[GET_GROUP_TS_ID_RES_INDEX];
    ret = devdrv_check_ts_id(ts_id);
    if (ret != 0) {
        devdrv_drv_err("ts id %d is invalid\n", ts_id);
        return ret;
    }

    ret = tsmng_group_operate_to_ts_sync(common_status.dev_id, ts_id, &req_msg, &rsp_msg);
    if (ret != 0) {
        devdrv_drv_err("create ts group send ipc msg failed, ret = %d, device id = %d, ts id=%d\n",
                       ret, common_status.dev_id, ts_id);
        return ret;
    }

    rsp_payload = (struct ipc_operate_group_rsp_payload *)rsp_msg.ipcdrv_payload;
    if (rsp_payload->result != 0) {
#ifndef CFG_SOC_PLATFORM_MDC_V51
        struct ts_group_info *usr_arg = (struct ts_group_info __user*)(common_status.commoninfo);
        devdrv_map_ts_grp_oper_err_to_user(rsp_payload->result, &usr_arg->result);
#endif
        devdrv_drv_err("create group,receive ts response, but rsp_msg.ipcdrv_payload.result = %d\n"
                       "device id=%d, ts id=%d, group_id=%d, extend_attribute=%d, aicore_number=%d\n"
                       "aivector_number=%d, sdma_number=%d, aicpu_number=%d, active_sq_number=%d\n",
                       rsp_payload->result, common_status.dev_id, ts_id, group_info.group_id,
                       group_info.extend_attribute, group_info.aicore_number, group_info.aivector_number,
                       group_info.sdma_number, group_info.aicpu_number, group_info.active_sq_number);
        return -EBADRQC;
    }

    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
int devdrv_manager_get_emu_subsys_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    common_status_info_t common_status = {0};
    struct drv_safety_island_ipc_info req_msg = {0};
    struct drv_safety_island_ipc_info rsp_msg = {0};

    ret = devdrv_manager_status_check_device_id(&common_status, arg);
    if (ret != 0) {
        devdrv_drv_err("check device id fail ret = %d\n", ret);
        return ret;
    }
    req_msg.cmd_type = IPC_GET_SAFETYISLAND_INFO_CMD_TYPE;
    req_msg.cmd = IPC_GET_SAFETYISLAND_INFO_CMD;
    req_msg.data_type = IPC_SAFETY_ISLAND_REQUEST;
    req_msg.len = IPC_GET_SAFETYISLAND_INFO_FUN_TYPE_LEN;
    req_msg.payload[0] = GET_EMU_INFO;
    req_msg.cmd_src = DEVDRV_SEND_TAISHAN_SUBID;
    req_msg.cmd_dest = SAFETY_ISLAND;
    req_msg.crc = sils_crc16((unsigned char *)&req_msg, req_msg.len + SILS_IPC_HEAD_LEN);

    ret = devdrv_get_msg_from_safetyisland(common_status.dev_id, &req_msg, &rsp_msg);
    if (ret) {
        devdrv_drv_err("get msg from safetyisland failed, ret = %d\n", ret);
        return ret;
    }

    ret = check_ipc_msg_from_safetyisland(&rsp_msg, &req_msg, IPC_GET_EMU_INFO_PAYLOAD_LEN);
    if (ret != 0) {
        devdrv_drv_err("msg from safetyisland is invalid, ret = %d\n", ret);
        print_ipc_msg_info(&rsp_msg, &req_msg);
        return ret;
    }

    ret = devdrv_manager_common_status_to_user(rsp_msg.payload, DRVFAULT_IPC_MSG_LENGTH, arg);
    if (ret) {
        devdrv_drv_err("devdrv_manager_common_status_to_user failed, ret = %d\n", ret);
        return ret;
    }

    return ret;
}

int devdrv_manager_get_safetyisland_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    common_status_info_t common_status = {0};
    struct drv_safety_island_ipc_info req_msg = {0};
    struct drv_safety_island_ipc_info rsp_msg = {0};

    ret = devdrv_manager_status_check_device_id(&common_status, arg);
    if (ret != 0) {
        devdrv_drv_err("check device id fail ret = %d\n", ret);
        return ret;
    }

    req_msg.cmd_type = IPC_GET_SAFETYISLAND_INFO_CMD_TYPE;
    req_msg.cmd = IPC_GET_SAFETYISLAND_INFO_CMD;
    req_msg.data_type = IPC_SAFETY_ISLAND_REQUEST;
    req_msg.len = IPC_GET_SAFETYISLAND_INFO_FUN_TYPE_LEN;
    req_msg.payload[0] = GET_SAFETY_ISLAND_INFO;
    req_msg.cmd_src = DEVDRV_SEND_TAISHAN_SUBID;
    req_msg.cmd_dest = SAFETY_ISLAND;
    req_msg.crc = sils_crc16((unsigned char *)&req_msg, req_msg.len + SILS_IPC_HEAD_LEN);

    ret = devdrv_get_msg_from_safetyisland(common_status.dev_id, &req_msg, &rsp_msg);
    if (ret) {
        devdrv_drv_err("get msg from safetyisland failed, ret = %d\n", ret);
        return ret;
    }

    ret = check_ipc_msg_from_safetyisland(&rsp_msg, &req_msg, IPC_GET_SAFETYISLAND_INFO_PAYLOAD_LEN);
    if (ret != 0) {
        devdrv_drv_err("msg from safetyisland is invalid, ret = %d\n", ret);
        print_ipc_msg_info(&rsp_msg, &req_msg);
        return ret;
    }

    ret = devdrv_manager_common_status_to_user(rsp_msg.payload, DRVFAULT_IPC_MSG_LENGTH, arg);
    if (ret) {
        devdrv_drv_err("devdrv_manager_common_status_to_user failed, ret = %d\n", ret);
        return ret;
    }

    return ret;
}
#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
int devdrv_manager_get_ufs_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    devdrv_drv_info("lite els do not support.\n");
    return 0;
}
#else
int devdrv_manager_get_ufs_status(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct ufs_status_stru ufs_status = {0}; /*lint !e64*/
    common_status_info_t common_status = {0};

    ret = devdrv_manager_status_check_device_id(&common_status, arg);
    if (ret != 0) {
        devdrv_drv_err("check device id fail ret = %d\n", ret);
        return ret;
    }

    ret = get_ufs_status(&ufs_status);
    if (ret) {
        devdrv_drv_err("get ufs status failed, ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_common_status_to_user(&ufs_status, sizeof(struct ufs_status_stru), arg);
    if (ret) {
        devdrv_drv_err("copy to user failed, ret = %d\n", ret);
        return ret;
    }

    return ret;
}
#endif
int devdrv_manager_get_ufs_buf_len(common_status_info_t *common_status, unsigned int *len)
{
    int ret;
    unsigned int sub_cmd;

    sub_cmd = common_status->reserver[0];
    switch (sub_cmd) {
        case UFS_SUB_CMD_CONFIG:
            *len = sizeof(struct ufs_config_stru);
            ret = 0;
            break;
        case UFS_SUB_CMD_STATUS:
            *len = sizeof(struct ufs_status_stru);
            ret = 0;
            break;
        case UFS_SUB_CMD_INLINE_CRYPTO:
            *len = sizeof(struct ufs_inline_crypto_stru);
            ret = 0;
            break;
        case UFS_SUB_CMD_CREATE_KEY:
            *len = sizeof(struct ufs_key_index_stru);
            ret = 0;
            break;
        case UFS_SUB_CMD_DELETE_KEY:
            *len = sizeof(struct ufs_key_index_stru);
            ret = 0;
            break;
        case UFS_SUB_CMD_QUERY_DESCRIPTOR:
            *len = sizeof(struct ufs_descriptor_stru);
            ret = 0;
            break;
        default:
            devdrv_drv_err("invalid cmd(%d).\n", sub_cmd);
            ret = -EINVAL;
            break;
    }

    return ret;
}
#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
int devdrv_manager_get_ufs_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    devdrv_drv_info("lite els do not support.\n");
    return 0;
}
#else
int devdrv_manager_get_ufs_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    common_status_info_t common_status = {0};
    unsigned int sub_cmd;
    unsigned char *cfg_info = NULL;
    unsigned int out_size = 0;
    unsigned int sub_len = 0;

    ret = devdrv_manager_status_check_device_id(&common_status, arg);
    if (ret != 0) {
        devdrv_drv_err("check device id fail ret = %d\n", ret);
        return ret;
    }

    sub_cmd = common_status.reserver[0];
    if (devdrv_manager_get_ufs_buf_len(&common_status, &sub_len) != 0) {
        devdrv_drv_err("ufs sub_cmd buf len get failed, sub_cmd = %d\n", sub_cmd);
        return -EINVAL;
    }

    if (common_status.commoninfo_len > sub_len) {
        devdrv_drv_err("sub_cmd(%d) len %d is over %d.\n", sub_cmd, common_status.commoninfo_len, sub_len);
        return -EINVAL;
    }

    if (common_status.commoninfo_len == 0) {
        devdrv_drv_err("common_status.commoninfo_len is zero\n");
        return -EINVAL;
    }

    cfg_info = kzalloc(common_status.commoninfo_len, GFP_KERNEL | __GFP_ACCOUNT);
    if (cfg_info == NULL) {
        devdrv_drv_err("kzalloc fail\n");
        return -EINVAL;
    }

    ret = get_ufs_info((UFS_SUB_CMD)sub_cmd, cfg_info, common_status.commoninfo_len, &out_size);
    if (ret != 0) {
        devdrv_drv_err("get_ufs_info failed, ret = %d, ufs sub_cmd = %d\n", ret, sub_cmd);
        goto out;
    }

    ret = copy_to_user_safe(common_status.commoninfo, cfg_info, common_status.commoninfo_len);
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret = %d\n", ret);
        goto out;
    }
    common_status.commoninfo_len = out_size;

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &common_status, sizeof(common_status_info_t));
    if (ret != 0) {
        devdrv_drv_err("copy input_arg failed, ret = %d\n", ret);
        goto out;
    }

out:
    kfree(cfg_info);
    cfg_info = NULL;
    return ret;
}
#endif

typedef int (*dms_set_ufs_info_ops)(UFS_SUB_CMD sub_cmd, void *buf, unsigned int buf_len);
#define DMS_OPS_SET_UFS_INFO "set_ufs_info"

int devdrv_manager_set_ufs_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    common_status_info_t common_status = {0};
    unsigned int sub_cmd;
    unsigned char *cfg_info = NULL;
    unsigned int sub_len = 0;
    dms_set_ufs_info_ops set_ufs_info_func = NULL;

    /* check process whitelist */
    ret = devdrv_manager_check_call_process();
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_check_call_process ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_status_check_device_id(&common_status, arg);
    if (ret != 0) {
        devdrv_drv_err("check device id fail ret = %d\n", ret);
        return ret;
    }

    sub_cmd = common_status.reserver[0];
    ret = devdrv_manager_get_ufs_buf_len(&common_status, &sub_len);
    if (ret != 0) {
        devdrv_drv_err("ufs sub_cmd buf len get failed, ret = %d\n", ret);
        return ret;
    }

    if (common_status.commoninfo_len > sub_len) {
        devdrv_drv_err("sub_cmd(%d) len %d is over %d.\n", sub_cmd, common_status.commoninfo_len, sub_len);
        return -EINVAL;
    }

    if (common_status.commoninfo_len == 0) {
        devdrv_drv_err("common_status.commoninfo_len is zero\n");
        return -EINVAL;
    }

    cfg_info = kzalloc(common_status.commoninfo_len, GFP_KERNEL | __GFP_ACCOUNT);
    if (cfg_info == NULL) {
        devdrv_drv_err("kzalloc fail\n");
        return -EINVAL;
    }

    ret = copy_from_user_safe(cfg_info, common_status.commoninfo, common_status.commoninfo_len);
    if (ret) {
        devdrv_drv_err("copy commoninfo failed, ret = %d\n", ret);
        goto out;
    }
    set_ufs_info_func = (dms_set_ufs_info_ops)(uintptr_t)__symbol_get(DMS_OPS_SET_UFS_INFO);
    if (set_ufs_info_func == NULL) {
        devdrv_drv_err("Lookup symbol: %s failed.\n", DMS_OPS_SET_UFS_INFO);
        ret = -EINVAL;
        goto out;
    }

    ret = set_ufs_info_func((UFS_SUB_CMD)sub_cmd, cfg_info, common_status.commoninfo_len);
    __symbol_put(DMS_OPS_SET_UFS_INFO);
    if (ret) {
        devdrv_drv_err("set_ufs_info failed, ret = %d, ufs sub_cmd = %d\n", ret, sub_cmd);
        goto out;
    }
out:
    kfree(cfg_info);
    cfg_info = NULL;
    return ret;
}

int devdrv_parse_single_power_state(u32 dev_id, u32 os_idx, u32 power_type, u32 *status)
{
    struct devdrv_sub_os_info *sub_os = NULL;
    void __iomem *addr = NULL;
    u32 reg_val;

    if (status == NULL) {
        devdrv_drv_err("para err, dev(%d) os_type(%d) power_type(%d).\n", dev_id, os_idx, power_type);
        return -EINVAL;
    }

    if (os_idx >= DEVDRV_SUB_OS_MAX) {
        devdrv_drv_err("os_type(%d) err, dev(%d) power_type(%d).\n", os_idx, dev_id, power_type);
        return -EINVAL;
    }

    if ((power_type != POWER_STATE_POWEROFF) && (power_type != POWER_STATE_RESET)) {
        devdrv_drv_err("invalid DSMI_POWER_STATE! type = %d\n", power_type);
        return -EINVAL;
    }

    sub_os = devdrv_get_sub_os_info();
    if (sub_os == NULL) {
        devdrv_drv_err("sub_os null, dev(%d) os_type(%d) power_type(%d).\n", dev_id, os_idx, power_type);
        return -EINVAL;
    }

    if (sub_os->pw_pdata[os_idx].valid != DEVDRV_SUB_OS_VALID) {
        return -ENODATA;
    }

    if (power_type == POWER_STATE_RESET) {
        addr = sub_os->pw_pdata[os_idx].boot_vaddr;
    } else {
        addr = sub_os->pw_pdata[os_idx].shut_vaddr;
    }

    reg_val = readl(addr);
    if ((reg_val == DEVDRV_SUB_OS_BOOT_MAGIC) || (reg_val == DEVDRV_SUB_OS_SHUT_MAGIC)) {
        *status = DEVDRV_SUB_OS_OFFLINE;
    } else {
        *status = DEVDRV_SUB_OS_ONLINE;
    }

    return 0;
}

int devdrv_get_single_power_state(u32 dev_id, u32 os_idx, u32 power_type, u32 *status)
{
    u32 tmp = 0;
    int ret;

    ret = devdrv_parse_single_power_state(dev_id, os_idx, power_type, &tmp);
    if (ret == -EINVAL) {
        devdrv_drv_err("get os(%d) power_type(%d) state failed, ret = %d.\n", os_idx, power_type, ret);
        return ret;
    }

    if (ret == -ENODATA) {
        *status = DEVDRV_SUB_OS_OFFLINE;
        return 0;
    }

    *status = tmp;

    return 0;
}

int devdrv_get_all_power_state(u32 dev_id, u32 power_type, u32 *status)
{
    u32 power_state = 0;
    u32 tmp = 0;
    u32 os_idx;
    int ret;

    if (status == NULL) {
        devdrv_drv_err("para err, dev(%d) power_type(%d).\n", dev_id, power_type);
        return -EINVAL;
    }

    for (os_idx = 0; os_idx < DEVDRV_SUB_OS_MAX; os_idx++) {
        ret = devdrv_get_single_power_state(dev_id, os_idx, power_type, &tmp);
        if (ret != 0) {
            devdrv_drv_err("get os(%d) power_type(%d) state failed, ret = %d.\n", os_idx, power_type, ret);
            return ret;
        }

        power_state |= (tmp << os_idx);
    }

    if ((power_state & DEVDRV_SUB_OS_BITMAP) == DEVDRV_SUB_OS_BITMAP) {
        *status = DEVDRV_SUB_OS_OFFLINE;
    } else {
        *status = DEVDRV_SUB_OS_ONLINE;
    }

    return 0;
}


int devdrv_get_power_state(u32 dev_id, DEVDRV_SUB_OS_TYPE os_type, DSMI_POWER_STATE power_type, u32 *power_state)
{
    int ret = 0;

    if (power_state == NULL) {
        devdrv_drv_err("para null, dev(%d)os_type(%d)power_type(%d).\n", dev_id, os_type, power_type);
        return -EINVAL;
    }

    switch (os_type) {
        case SUB_OS_SD:
        case SUB_OS_CLUSTER:
        case SUB_OS_COMIS:
            ret = devdrv_get_single_power_state(dev_id, (u32)os_type, (u32)power_type, power_state);
            if (ret != 0) {
                devdrv_drv_err("get os(%d) power_type(%d) state failed, ret = %d\n", os_type, power_type, ret);
                return ret;
            }

            break;
        case SUB_OS_ALL:
            ret = devdrv_get_all_power_state(dev_id, (u32)power_type, power_state);
            if (ret != 0) {
                devdrv_drv_err("get all os power_type(%d) state failed, ret = %d\n", power_type, ret);
                return ret;
            }

            break;

        default:
            devdrv_drv_err("invalid type! type = %d\n", os_type);
            return -EINVAL;
    }

    return ret;
}

int devdrv_manager_get_power_state(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct ioctl_arg arg_info = { 0 };
    DEVDRV_SUB_OS_TYPE os_type;
    DSMI_POWER_STATE power_type;
    unsigned int power_state = 0;

    ret = copy_from_user_safe(&arg_info, (void *)((uintptr_t)arg), sizeof(arg_info));
    if (ret != 0) {
        devdrv_drv_err("copy common_status failed, ret = %d\n", ret);
        return ret;
    }

    if (dev_manager_info == NULL || arg_info.dev_id >= dev_manager_info->num_dev) {
        devdrv_drv_err("device_manager is null or invalid device id, id = %d.\n", arg_info.dev_id);
        return -EINVAL;
    }

    os_type = (DEVDRV_SUB_OS_TYPE)arg_info.type;
    power_type = (DSMI_POWER_STATE)arg_info.data1;
    if ((power_type != POWER_STATE_POWEROFF) && (power_type != POWER_STATE_RESET)) {
        devdrv_drv_err("invalid DSMI_POWER_STATE! type = %d\n", power_type);
        return -EINVAL;
    }

    ret = devdrv_get_power_state(arg_info.dev_id, os_type, power_type, &power_state);
    if (ret != 0) {
        devdrv_drv_err("devdrv_get_power_state failed, os_type(%d) power_type(%d) ret(%d).\n",
            os_type, power_type, ret);
        return ret;
    }

    arg_info.data3 = power_state;
    ret = copy_to_user_safe((void *)((uintptr_t)arg), &arg_info, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("[dev_id = %d]:copy_to_user_safe failed.\n", arg_info.dev_id);
        return -EINVAL;
    }

    return 0;
}

int devdrv_manager_get_sils_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret, chip_type;
    common_status_info_t common_status = {0};
    struct drv_safety_island_ipc_info req_msg = {0};
    struct drv_safety_island_ipc_info rsp_msg = {0};

    ret = devdrv_manager_get_chip_type(&chip_type);
    if (ret != 0) {
        devdrv_drv_err("get chip type failed, ret = %d.\n", ret);
        return ret;
    }

    if ((chip_type != CHIP_TYPE_ASCEND_V2) && (chip_type != CHIP_TYPE_ASCEND_V51_LITE)) {
        devdrv_drv_warn("chip type=%d not support.\n", chip_type);
        return -EOPNOTSUPP;
    }

    ret = devdrv_manager_status_check_device_id(&common_status, arg);
    if (ret != 0) {
        devdrv_drv_err("check device id fail ret = %d\n", ret);
        return ret;
    }

    if (common_status.reserver[0] != IPC_SUBCMD_SILS_PMUWDG_STATUS) {
        devdrv_drv_err("sub_cmd is not support! sub_cmd=%d.\n", common_status.reserver[0]);
        return -EINVAL;
    }

    req_msg.cmd_type = IPC_CMDTYPE_SILS_PMUWDG_EN;
    req_msg.cmd = CMD_INQUIRY;
    req_msg.cmd_dest = SAFETY_ISLAND;
    req_msg.cmd_src = DEVDRV_SEND_TAISHAN_SUBID;
    req_msg.data_type = IPC_SAFETY_ISLAND_REQUEST;
    req_msg.len = 0;
    req_msg.crc = sils_crc16((unsigned char *)&req_msg, req_msg.len + SILS_IPC_HEAD_LEN);
    ret = devdrv_get_msg_from_safetyisland(common_status.dev_id, &req_msg, &rsp_msg);
    if (ret != 0) {
        devdrv_drv_err("get msg from safetyisland failed, ret=%d sub_cmd=%d\n", ret, common_status.reserver[0]);
        return ret;
    }

    ret = check_ipc_msg_from_safetyisland(&rsp_msg, &req_msg, IPC_GET_SAFETYISLAND_INFO_PAYLOAD_LEN);
    if (ret != 0) {
        devdrv_drv_err("msg from safetyisland is invalid, ret=%d sub_cmd=%d\n", ret, common_status.reserver[0]);
        print_ipc_msg_info(&rsp_msg, &req_msg);
        return ret;
    }

    ret = devdrv_manager_common_status_to_user(rsp_msg.payload, DRVFAULT_IPC_MSG_LENGTH, arg);
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret=%d sub_cmd=%d\n", ret, common_status.reserver[0]);
        return ret;
    }

    return ret;
}

int devdrv_manager_set_sils_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret, chip_type;
    common_status_info_t common_status = {0};
    struct drv_safety_island_ipc_info req_msg = {0};
    struct drv_safety_island_ipc_info rsp_msg = {0};

    ret = devdrv_manager_get_chip_type(&chip_type);
    if (ret != 0) {
        devdrv_drv_err("get chip_type failed, ret = %d.\n", ret);
        return ret;
    }

    if ((chip_type != CHIP_TYPE_ASCEND_V2) && (chip_type != CHIP_TYPE_ASCEND_V51_LITE)) {
        devdrv_drv_warn("chip_type=%d not support.\n", chip_type);
        return -EOPNOTSUPP;
    }

    ret = devdrv_manager_status_check_device_id(&common_status, arg);
    if (ret != 0) {
        devdrv_drv_err("check device id fail ret = %d\n", ret);
        return ret;
    }

    if ((common_status.reserver[0] != IPC_SUBCMD_SILS_PMUWDG_DISABLE)
        && (common_status.reserver[0] != IPC_SUBCMD_SILS_PMUWDG_ENABLE)) {
        devdrv_drv_err("sub_cmd is not support! sub_cmd=%d.\n", common_status.reserver[0]);
        return -EINVAL;
    }

    req_msg.cmd_type = IPC_CMDTYPE_SILS_PMUWDG_EN;
    req_msg.cmd = CMD_SETTING;
    req_msg.cmd_src = DEVDRV_SEND_TAISHAN_SUBID;
    req_msg.cmd_dest = SAFETY_ISLAND;
    req_msg.data_type = IPC_SAFETY_ISLAND_REQUEST;
    req_msg.len = IPC_CMDLEN_SILS_PMUWDG_EN;
    req_msg.payload[0] = (char)common_status.reserver[0];
    req_msg.crc = sils_crc16((unsigned char *)&req_msg, req_msg.len + SILS_IPC_HEAD_LEN);
    ret = devdrv_get_msg_from_safetyisland(common_status.dev_id, &req_msg, &rsp_msg);
    if (ret != 0) {
        devdrv_drv_err("get msg from safetyisland failed, ret=%d sub_cmd=%d\n", ret, common_status.reserver[0]);
        return ret;
    }

    if (check_ipc_msg_from_safetyisland(&rsp_msg, &req_msg, IPC_SET_SAFETYISLAND_INFO_PAYLOAD_LEN) != 0) {
        devdrv_drv_err("msg from safetyisland is invalid, ret=%d sub_cmd=%d\n", ret, common_status.reserver[0]);
        print_ipc_msg_info(&rsp_msg, &req_msg);
        return -EINVAL;
    }

    ret = devdrv_manager_common_status_to_user(rsp_msg.payload, IPC_RSPLEN_SILS_PMUWDG_EN, arg);
    if (ret != 0) {
        devdrv_drv_err("copy to user failed, ret=%d sub_cmd=%d\n", ret, common_status.reserver[0]);
        return ret;
    }

    return ret;
}

#endif
#endif /* CFG_FEATURE_LP_ENABLE */

STATIC int devdrv_manager_get_errstr(unsigned long arg)
{
    struct bb_err_string user_arg = {0};
    int ret;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct bb_err_string));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    if ((user_arg.dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (user_arg.buf_len != BBOX_ERRSTR_LEN)) {
        devdrv_drv_err(
            "invalid device id, id = %u. or invalid buffer len, user_arg.buf_len = %d, user_arg.buf_len should be %d\n",
            user_arg.dev_id, user_arg.buf_len, BBOX_ERRSTR_LEN);
        return -EINVAL;
    }

    ret = bbox_get_device_ecode_info(user_arg.errcode, user_arg.errstr, user_arg.buf_len);
    if (ret) {
        devdrv_drv_err("get error str failed or the errcode not exist. ret = %d.\n", ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &user_arg, sizeof(struct bb_err_string));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }
    return 0;
}

/*
 * register operation, read or write
 */
int devdrv_reg_op(unsigned char op_type, unsigned long base_phy_reg, unsigned long reg_offset,
    unsigned long map_size, unsigned int *val)
{
    int ret;
    void __iomem *regs = NULL;

    regs = ioremap(base_phy_reg, map_size);
    if (regs == NULL) {
        devdrv_drv_err("ioremap error.\n");
        ret = -ENOMEM;
        return ret;
    }

    if (op_type == DEVDRV_REG_WR) {
        writel(*val, (void __iomem *)(uintptr_t)((long)(uintptr_t)regs + reg_offset));
    } else {
        *val = readl((void __iomem *)(uintptr_t)((long)(uintptr_t)regs + reg_offset));
    }

    (void)iounmap(regs);
    regs = NULL;

    return 0;
}

STATIC int devdrv_reset_i2c_controller(unsigned long arg)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
    return -EINVAL;
#else
    int ret;
    struct ioctl_arg user_arg = {0};
    unsigned int reg_val;
    unsigned int dev_id;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    dev_id = user_arg.dev_id;
    if (dev_manager_info == NULL || dev_id >= dev_manager_info->num_dev) {
        devdrv_drv_err("device_manager is null or invalid device id, id = %d.\n", dev_id);
        return -EINVAL;
    }

    /* reset the i2c controller by set the register */
    reg_val = 1;
    ret = devdrv_reg_op(DEVDRV_REG_WR, SM_BUS_CTL_BASE_ADDR, RESET_I2C_OFFSET, I2C_RESET_MAP_SIZE, &reg_val);
    if (ret < 0) {
        devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
        return ret;
    }

    /* unreset the i2c controller by set the register */
    reg_val = 1;
    ret = devdrv_reg_op(DEVDRV_REG_WR, SM_BUS_CTL_BASE_ADDR, UNRESET_I2C_OFFSET, I2C_RESET_MAP_SIZE, &reg_val);
    if (ret < 0) {
        devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
#endif
}

STATIC int devdrv_get_xloader_boot_info(unsigned long arg)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
    return -EINVAL;
#else
    int ret;
    unsigned int op_flag;
    unsigned int op_area = 0;
    unsigned int reg_val = 0;
    unsigned int idx = 0;
    struct ioctl_arg user_arg = {0};

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    op_flag = user_arg.data1;

    if (op_flag == DEV_GET_CURR_BOOT_AREA) {
        /*
          sysctrl reg, 0x1100c3448 byte1:
          0x1--xloader0 boot
          0x2--xloader1 boot
 */
        reg_val = 0;
        ret = devdrv_reg_op(DEVDRV_REG_RD, SYSCTRL_REG_BASE, SC_BAK_DATA14, SIZE_OF_64K, &reg_val);
        if (ret < 0) {
            devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
            return ret;
        }

        idx = ((reg_val >> OFFSET_EIGHT) & 0xff);
        if ((idx != BOOT_FROM_XLOADER0) && (idx != BOOT_FROM_XLOADER1)) {
            devdrv_drv_err("boot area info err: idx=%u\n", idx);
            user_arg.data3 = ERRNO_NONSUPPORT_ITEM;
            user_arg.data1 = 0;
            goto out;
        }

        op_area = (idx == BOOT_FROM_XLOADER0) ? DEV_UPDATE_XLOADER_AREA0 : DEV_UPDATE_XLOADER_AREA1;
    } else if (op_flag == DEV_CLEAR_BOOT_COUNT) {
        /* upgrade or success, need clear boot count */
        ret = devdrv_reg_op(DEVDRV_REG_RD, SYSCTRL_REG_BASE, SC_SOFT_POR_RSV3, SIZE_OF_64K, &reg_val);
        if (ret < 0) {
            devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
            return ret;
        }

        reg_val = 0;
        ret = devdrv_reg_op(DEVDRV_REG_WR, SYSCTRL_REG_BASE, SC_SOFT_POR_RSV3, SIZE_OF_64K, &reg_val);
        if (ret < 0) {
            devdrv_drv_err("Register operation failed. (ret=%d)\n", ret);
            return ret;
        }
    } else {
        devdrv_drv_err("dev_upgrade_xloader_boot_area_op fail, op_flag(%x)err.\n", op_flag);
    }

    devdrv_drv_info("op_flag: %x, value: %x, idx: %u\n", op_flag, reg_val, idx);

    user_arg.data1 = op_area;
    user_arg.data3 = 0;

out:
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }

    return 0;
#endif
}

STATIC int devdrv_gpio_read(unsigned long arg)
{
    int ret;
    int gpio_num;
    unsigned int gpio_val;
    struct ioctl_arg user_arg = {0};
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)arg), sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe failed.\n");
        return ret;
    }

    ret = devdrv_manager_trans_and_check_id(user_arg.dev_id, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", user_arg.dev_id, ret);
        return ret;
    }

    DRV_CHECK_EXP_ACT(phys_id >= DEVDRV_MANGER_MAX_DEVICE_NUM, return -EINVAL, "Invalid devid(%u)\n", phys_id);
    gpio_num = user_arg.data1;

    if (!gpio_is_valid(gpio_num)) {
        devdrv_drv_err("invalid gpio, gpio=%d\n", gpio_num);
        return -EINVAL;
    }

    ret = gpio_request(gpio_num, DEVDRV_GPIO_NAME);
    if (ret) {
        devdrv_drv_err("gpio [%d] request failed\n", gpio_num);
        return ret;
    }

    gpio_val = gpio_get_value(gpio_num);

    (void)gpio_free(gpio_num);

    user_arg.data1 = gpio_val;
    ret = copy_to_user_safe((void *)(uintptr_t)arg, &user_arg, sizeof(struct ioctl_arg));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return ret;
    }

    return 0;
}

int devdrv_manager_inquiry_info(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    switch (cmd) {
        case DEVDRV_MANAGER_GET_PCIE_ID_INFO:
            ret = devdrv_get_pcie_id(arg);
            break;
        case DEVDRV_MANAGER_GET_HEALTH_CODE:
            ret = devdrv_get_health_code(arg);
            break;
        case DEVDRV_MANAGER_GET_ERROR_CODE:
            ret = devdrv_get_error_code(arg);
            break;
        case DEVDRV_MANAGER_GET_DDR_CAPACITY:
            ret = devdrv_get_ddr_capacity(arg);
            break;
#ifdef CFG_FEATURE_HISS
        case DEVDRV_MANAGER_GET_FLASH_COUNT:
            ret = devdrv_get_flash_count(arg);
            break;
#endif
        default:
            devdrv_drv_err("invalid cmd.\n");
            ret = -1;
            break;
    }
    return ret;
}

int devdrv_manager_inquiry_info_ex(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;

    switch (cmd) {
        case DEVDRV_MANAGER_GET_LLC_PERF_PARA:
            ret = devdrv_get_llc_perf_para(arg);
            break;
        case DEVDRV_MANAGER_RST_I2C_CTROLLER:
            ret = devdrv_reset_i2c_controller(arg);
            break;
        case DEVDRV_MANAGER_GET_XLOADER_BOOT_INFO:
            ret = devdrv_get_xloader_boot_info(arg);
            break;
        case DEVDRV_MANAGER_GET_GPIO_STATE:
            ret = devdrv_gpio_read(arg);
            break;
        default:
            devdrv_drv_err("invalid cmd.\n");
            ret = -1;
            break;
    }

    return ret;
}

int devdrv_manager_imu_cmd(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0, dev_id = 0;

    ret = devdrv_manager_trans_and_check_id(dev_id, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    switch (cmd) {
#if (defined CFG_BUILD_DEBUG) && (!defined CFG_SOC_PLATFORM_MINIV3)
        case DEVDRV_MANAGER_IMU_SMOKE:
#ifndef CFG_SOC_PLATFORM_MINIV2
            ret = devdrv_imu_smoke_ipc(arg);
#else
            ret = devdrv_lp_smoke_ipc(arg);
#endif
            break;
        case DEVDRV_MANAGER_GET_IMU_INFO:
            ret = devdrv_get_imu_info(arg);
            break;
        case DEVDRV_MANAGER_DEBUG_INFORM:
            ret = devdrv_imu_debug(arg);
            break;
#endif
        case DEVDRV_MANAGER_SEND_TO_IMU:
            ret = devdrv_ipc_send_to_imu(arg);
            break;
        case DEVDRV_MANAGER_RECV_FROM_IMU:
            ret = devdrv_ipc_recv_from_imu(arg);
            break;
        case DEVDRV_MANAGER_CONFIG_ECC_ENABLE:
            ret = devdrv_config_ecc_enable(arg);
            break;
        case DEVDRV_MANAGER_COMPUTE_POWER:
            ret = tsmng_get_ai_computing_power(arg);
            break;
        case DEVDRV_MANAGER_GET_BBOX_ERRSTR:
            ret = devdrv_manager_get_errstr(arg);
            break;
        default:
            devdrv_drv_err("invalid cmd.\n");
            ret = -1;
            break;
    }
    return ret;
}

int devdrv_manager_get_pmu_voltage(struct file *filep, unsigned int cmd, unsigned long arg)
{
#if defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2)

#ifdef CONFIG_HISI_SPMI_PMIC_VOLTAGE
    struct pmu_voltage_stru pmu_voltage = {0};
    unsigned int value = 0;
    int ret;

    if ((void *)(uintptr_t)arg == NULL) {
        devdrv_drv_err("parameter check failed.\n");
        return -EINVAL;
    }

    ret = copy_from_user_safe(&pmu_voltage, (void *)(uintptr_t)arg, sizeof(struct pmu_voltage_stru));
    if (ret) {
        devdrv_drv_err("copy from user failed.\n");
        return -EINVAL;
    }
    // stub for pmu get interface
    switch (pmu_voltage.pmu_type) {
        case PMU21_VBUCK_VOLTAGE:
            ret = get_main_pmu_buck_volt(pmu_voltage.channel, &value);
            break;
        case PMU21_VOUT_VOLTAGE:
            ret = get_main_pmu_ldo_volt(pmu_voltage.channel, &value);
            break;
        case PMU22_VBUCK_VOLTAGE:
            ret = get_second_pmu_buck_volt(pmu_voltage.device_id, pmu_voltage.channel, &value);
            break;
        case PMU_ADC_VOLTAGE:
            ret = hisi_adc_get_value(pmu_voltage.channel);
            break;
        default:
            devdrv_drv_err("undefine PMU type!:%u\n", pmu_voltage.pmu_type);
            ret = -EINVAL;
            break;
    }

    pmu_voltage.get_value = value;
    pmu_voltage.return_value = ret;

    ret = copy_to_user_safe((void *)(uintptr_t)arg, &pmu_voltage, sizeof(struct pmu_voltage_stru));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe failed.\n");
        return -EINVAL;
    }

    return ret;
#else
    return 0;
#endif

#else
    return 0;
#endif
}

/*
 * map register memory
 */
int devdrv_map_register_memory(unsigned int dev_id)
{
    int ret;

    /* llc */
    ret = devdrv_llc_init(dev_id);
    if (ret < 0) {
        devdrv_drv_err("devdrv_llc_init failed.\n");
        return -EINVAL;
    }

    return 0;
}

/*
 * unmap register memory
 */
void devdrv_unmap_register_memory(unsigned int dev_id)
{
    devdrv_llc_exit(dev_id); /* llc */
}

 /*
  * result = 0: no task exist in the TS
  * others: task exist in the TS
  */
int devdrv_inquire_aicore_task(unsigned int dev_id, unsigned int fid,
    unsigned int tgid, unsigned int *result)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
    rproc_msg_t ack_buffer[IPCDRV_RPROC_MSG_LENGTH];
    struct ipcdrv_message *ipc_msg = NULL;
    int ret;

    if ((dev_id >= MAX_CHIP_NUM) || (result == NULL)) {
        devdrv_drv_err("invalid parameter, dev_id = %u, result = %pK.\n", dev_id, result);
        return -EINVAL;
    }

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_INQUIRY_AICORE_TASK;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_SYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_index = 0;
    *(unsigned int *)ipc_msg->ipcdrv_payload = tgid;
    *((unsigned int *)ipc_msg->ipcdrv_payload + 1) = fid;
    ipc_msg->ipc_msg_header.msg_length = sizeof(unsigned int) + sizeof(unsigned int);
    devdrv_drv_debug("inquire aicore_task, fid = %u, pid = %u.\n", fid, tgid);

    ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, HISI_RPROC_TX_TS_MBX17),
        (rproc_msg_t *)&msg, IPCDRV_RPROC_MSG_LENGTH, (rproc_msg_t *)ack_buffer,
        IPCDRV_RPROC_MSG_LENGTH);
    if (ret) {
        devdrv_drv_warn("devdrv_info_ipc_to_ts_sync failed. ret = %d.\n", ret);
        return ret;
    }
    ipc_msg = (struct ipcdrv_message *)ack_buffer;
    *result = *(unsigned int *)ipc_msg->ipcdrv_payload;
#endif
    return 0;
}
EXPORT_SYMBOL(devdrv_inquire_aicore_task);

int devdrv_manager_get_emmc_voltage(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    u32 phys_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;
    struct devdrv_emmc_voltage_para devdrv_emmc_voltage = { 0, 0 };

    ret = devdrv_manager_trans_and_check_id(0, &phys_id, &vfid, ONLY_DOES_SUPPORT_PF);
    if (ret != 0) {
        devdrv_drv_err("Failed to transfer logical ID to physical ID. (ret=%d)\n", ret);
        return ret;
    }

    devdrv_drv_debug("devdrv_manager_get_emmc_voltage start\n");
#if defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2)
    devdrv_emmc_voltage.emmc_vcc = hisi_adc_get_value(PMU_EMMC_VCC_CHANNEL);
    devdrv_emmc_voltage.emmc_vccq = hisi_adc_get_value(ADCIN8_SLOT1);
#else
    devdrv_emmc_voltage.emmc_vcc = -1;
    devdrv_emmc_voltage.emmc_vccq = -1;
#endif
    if (copy_to_user_safe((void *)((uintptr_t)arg), &devdrv_emmc_voltage, sizeof(struct devdrv_emmc_voltage_para))) {
        devdrv_drv_err("copy_to_user_safe failed\n");
        return -EFAULT;
    }

    devdrv_drv_debug("devdrv_manager_get_emmc_voltage finish\n");

    return 0;
}

#if defined(CFG_TEE_EFUSE_LDO_DBG)

int devdrv_manager_enable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg)
{
#if !defined(CFG_SOC_PLATFORM_MINIV2)
    int ret = 0;
#if defined(CFG_SOC_PLATFORM_CLOUD)
    int status = 0;
#endif

    devdrv_drv_debug("devdrv_manager_enable_efuse_ldo2 start\n");
#if defined(CFG_SOC_PLATFORM_MINI)
    ret = pmu_ldo2_enable();
#elif defined(CFG_SOC_PLATFORM_CLOUD)
    ret = drv_cpld_efuse_power_on_status(&status);
    if (ret) {
        devdrv_drv_err("get efuse power status failed!!\n");
        return ret;
    }
    if (status == 1) {
        devdrv_drv_info("efuse power already on\n");
        return 0;
    }
    ret = drv_cpld_efuse_power_ctrl(ENABLE_EFUSE_POWER);
#endif
    if (ret) {
        devdrv_drv_err("enable efuse ldo2 failed!!\n");
        return ret;
    }
    devdrv_drv_debug("devdrv_manager_disable_efuse_ldo finish\n");
#endif
    return 0;
}

int devdrv_manager_disable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg)
{
#if !defined(CFG_SOC_PLATFORM_MINIV2)
    int ret = 0;
#if defined(CFG_SOC_PLATFORM_CLOUD)
    int status = 0;
#endif

    devdrv_drv_debug("devdrv_manager_disable_efuse_ldo2 start\n");
#if defined(CFG_SOC_PLATFORM_MINI)
    ret = pmu_ldo2_disable();
#elif defined(CFG_SOC_PLATFORM_CLOUD)
    ret = drv_cpld_efuse_power_on_status(&status);
    if (ret) {
        devdrv_drv_err("get efuse power status failed!!\n");
        return ret;
    }
    if (status == 0) {
        devdrv_drv_info("efuse power already off\n");
        return 0;
    }
    ret = drv_cpld_efuse_power_ctrl(DISABLE_EFUSE_POWER);
#endif
    if (ret) {
        devdrv_drv_err("disable efuse ldo2 failed!!\n");
        return ret;
    }
    devdrv_drv_debug("devdrv_manager_disable_efuse_ld02 finish\n");
#endif
    return 0;
}

#else
int devdrv_manager_enable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg)
{
    (void)filep;
    (void)cmd;
    (void)arg;
    return 0;
}

int devdrv_manager_disable_efuse_ldo2(struct file *filep, unsigned int cmd, unsigned long arg)
{
    (void)filep;
    (void)cmd;
    (void)arg;
    return 0;
}
#endif
#if (defined CFG_FEATURE_LP_ENABLE) || (defined CFG_SOC_PLATFORM_MDC_V11)
STATIC int devdrv_manager_set_power_state_para_check(u32 device_id, DSMI_POWER_STATE cmd_type)
{
    if (device_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) {
        devdrv_drv_err("invalid device id, id = %d.\n", device_id);
        return -EINVAL;
    }

    if (cmd_type >= POWER_STATE_MAX) {
        devdrv_drv_err("invalid DSMI_POWER_STATE! type = %d\n", cmd_type);
        return -EINVAL;
    }
    return OK;
}

STATIC int devdrv_manager_set_power_state_pre_check(struct ioctl_arg *arg_info)
{
    int ret;

    /* check process whitelist */
    ret = devdrv_manager_check_call_process();
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_check_call_process ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_set_power_state_para_check(arg_info->dev_id, arg_info->type);
    if (ret != OK) {
        devdrv_drv_err("set_power_state_para_check failed! ret = %d\n", ret);
        return ret;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (arg_info->type == POWER_STATE_BIST) {
        ret = devdrv_manager_set_power_state_bist_pre_check();
        if (ret != OK) {
            devdrv_drv_err("set_power_state_bist_pre_check failed! ret = %d\n", ret);
            return ret;
        }
    }
#endif

    return 0;
}
#endif

#if (defined CFG_FEATURE_LP_ENABLE)
typedef int (*dms_set_hsm_reset)(uint32_t dev_id);
#define DMS_OPS_SET_HSM_RESET "hsm_notify_reset_action"

int devdrv_manager_set_power_state(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct ioctl_arg arg_info = { 0 };
    dms_set_hsm_reset set_hsm_reset_func = NULL;

    ret = copy_from_user_safe(&arg_info, (void *)((uintptr_t)arg), sizeof(arg_info));
    if (ret != 0) {
        devdrv_drv_err("copy common_status failed, ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_set_power_state_pre_check(&arg_info);
    if (ret != 0) {
        devdrv_drv_err("pre check failed, ret=%d.\n", ret);
        return ret;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V51
    devdrv_manager_set_safetyisland_power();
    mdelay(SLEEP_POWER_DELAY_MS);
#endif

    switch (arg_info.type) {
        case POWER_STATE_SUSPEND:
#ifdef CFG_SOC_PLATFORM_MDC_V51
        case POWER_STATE_BIST:
#endif
            devdrv_manager_notify_lp_suspend(arg_info.dev_id, (DSMI_POWER_STATE)arg_info.type);
            devdrv_drv_event("devdrv_suspend_proc start, type=%u.\n", arg_info.type);
            ret = devdrv_suspend_proc(arg_info.dev_id);
            if (ret != 0) {
                devdrv_drv_err("devdrv_suspend_proc failed, ret = %d\n", ret);
                break;
            }
#ifdef CFG_SOC_PLATFORM_MDC_V51
            if (suspend_stats.failed_resume != 0 || suspend_stats.failed_resume_early != 0 ||
                    suspend_stats.failed_resume_noirq != 0) {
                devdrv_drv_err("invoke os interface pm_suspend failed in resume\n");
                ret = -DRV_KERNEL_ERROR_RESUME;
                break;
            }
#endif
            devdrv_drv_event("devdrv_suspend_proc end, type=%u.\n", arg_info.type);
            break;
        case POWER_STATE_POWEROFF:
        case POWER_STATE_RESET:
#ifdef CFG_SOC_PLATFORM_MDC_V51
            devdrv_manager_record_reset_reason(RST_REASON_CUR_L3SRAM_OFFSET1, RST_REASON_BIT_DSMI_CMD);
            devdrv_drv_event("dsmi set power state(%u)\n", arg_info.type);
#endif
            set_hsm_reset_func = (dms_set_hsm_reset)(uintptr_t)__symbol_get(DMS_OPS_SET_HSM_RESET);
            if (set_hsm_reset_func == NULL) {
                devdrv_drv_err("Lookup symbol: %s failed.\n", DMS_OPS_SET_HSM_RESET);
                return -EINVAL;
            }

            ret = set_hsm_reset_func(arg_info.dev_id);
            __symbol_put(DMS_OPS_SET_HSM_RESET);
            if (ret != 0) {
                devdrv_drv_err("send power msg to hiss failed, ret = %d, dev_id = %d\n", ret, arg_info.dev_id);
                break;
            }
            devdrv_drv_event("send power msg to hiss success.\n");
            break;
        default:
            devdrv_drv_err("invalid type! type = %d\n", arg_info.type);
            return -EINVAL;
        }

    return ret;
}
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V11
int devdrv_manager_set_power_state_v2(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct ioctl_arg arg_info = { 0 };

    ret = copy_from_user_safe(&arg_info, (void *)((uintptr_t)arg), sizeof(arg_info));
    if (ret != 0) {
        devdrv_drv_err("copy common_status failed, ret = %d\n", ret);
        return ret;
    }

    ret = devdrv_manager_set_power_state_pre_check(&arg_info);
    if (ret != 0) {
        devdrv_drv_err("pre check failed, ret=%d.\n", ret);
        return ret;
    }
    switch (arg_info.type) {
        case POWER_STATE_POWEROFF:
        case POWER_STATE_RESET:
            devdrv_manager_record_reset_reason(RST_REASON_CUR_L3SRAM_OFFSET1, RST_REASON_BIT_DSMI_CMD);
            devdrv_drv_event("dsmi set power state(%u)\n", arg_info.type);
            break;
        default:
            devdrv_drv_err("invalid type! type = %d\n", arg_info.type);
            return -EINVAL;
    }
    return ret;
}
#endif
#else /* DEVMNG_UT */
int devdrv_get_error_code(unsigned long arg)
{
    return 0;
}
#endif

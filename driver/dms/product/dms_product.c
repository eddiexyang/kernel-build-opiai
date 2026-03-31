/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description:
 * Author: Huawei
 * Create: 2022-3-24
 * File Name     : dms_product.c
 * Version       : Initial product
 * Author        :
 * Created       : 2022-3-24
 * Last Modified :
 * Description   :

 *  History       :
 * 1.Date        : 2022-3-24
 *    Author      :
 *    Modification: Created file
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>

#include "dms_template.h"
#include "dms_cmd_def.h"
#include "dms_acc_ctrl.h"
#include "devdrv_user_common.h"
#include "devdrv_manager_common.h"
#include "dms_product.h"
#ifdef CFG_FEATURE_DMS_PRODUCT_HOST
#include "dms_product_host.h"
#endif

BEGIN_DMS_MODULE_DECLARATION(DMS_PRODUCT_CMD_NAME)
BEGIN_FEATURE_COMMAND()
ADD_FEATURE_COMMAND(DMS_PRODUCT_CMD_NAME, DMS_MAIN_CMD_PRODUCT, DMS_SUBCMD_GET_PCIE_ID_INFO_ALL, NULL, NULL,
                    DMS_SUPPORT_ALL, devdrv_get_pcie_id_all)
#ifdef CFG_FEATURE_DMS_PRODUCT_HOST
ADD_FEATURE_COMMAND(DMS_PRODUCT_CMD_NAME, DMS_MAIN_CMD_PRODUCT, DMS_SUBCMD_GET_WORK_MODE, NULL, NULL,
                    DMS_SUPPORT_ALL, devdrv_get_work_mode)
#endif
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

int dms_product_init(void)
{
    CALL_INIT_MODULE(DMS_PRODUCT_CMD_NAME);
    return 0;
}

void dms_product_exit(void)
{
    CALL_EXIT_MODULE(DMS_PRODUCT_CMD_NAME);
}

int devdrv_get_pcie_id_all(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct dmanage_pcie_id_info_all *pcie_id_info = NULL;
    struct devdrv_pcie_id_info id_info = {0};
    unsigned int dev_id, virt_id, vfid;
    int ret;

    if (in == NULL || (in_len != sizeof(unsigned int))) {
        dms_err("Input char is NULL or in_len is wrong.");
        return -EINVAL;
    }

    if (out == NULL || (out_len != sizeof(struct dmanage_pcie_id_info_all))) {
        dms_err("Out char is NULL or in_len is wrong.");
        return -EINVAL;
    }

    virt_id = *(unsigned int *)in;
    ret = devdrv_manager_container_logical_id_to_physical_id(virt_id, &dev_id, &vfid);
    if (ret != 0) {
        dms_err("Can't transfor virt id, (virt id = %u, ret = %d)\n", virt_id, ret);
        return ret;
    }

    ret = devdrv_get_pcie_id_info(dev_id, &id_info);
    if (ret != 0) {
        dms_err("Devdrv_get_pcie_id_info failed.\n");
        return ret;
    }

    pcie_id_info = (struct dmanage_pcie_id_info_all *)out;
    pcie_id_info->venderid = id_info.venderid;
    pcie_id_info->subvenderid = id_info.subvenderid;
    pcie_id_info->deviceid = id_info.deviceid;
    pcie_id_info->subdeviceid = id_info.subdeviceid;
    pcie_id_info->domain = id_info.domain;
    pcie_id_info->bus = id_info.bus;
    pcie_id_info->device = id_info.device;
    pcie_id_info->fn = id_info.fn;

    return 0;
}
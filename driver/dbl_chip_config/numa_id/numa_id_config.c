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
 * Create: 2022-08-10
 */

#include <linux/types.h>
#include <linux/numa.h>

#include "dbl/dev_identity.h"
#include "dbl/chip_config.h"
#include "chip_config_module.h"
#include "numa_id.h"
#include "numa_id_config.h"

/*
 * ---------------------------------------------------------------------------------------------------------
 *                                            numa id config of 1p
 * ---------------------------------------------------------------------------------------------------------
 * |      memtype       |                  DDR                  |                  HBM                  |
 * ---------------------------------------------------------------------------------------------------------
 * |    sub_memtype     |  ALL  |  P2P  |  TS   |  AI   |  CTRL |  ALL  |  P2P  |  TS   |  AI   |  CTRL |
 * ---------------------------------------------------------------------------------------------------------
 * |        |ascend310  |   0   |       |   0   |   0   |   0   |       |       |       |       |       |
 * -        ------------------------------------------------------------------------------------------------
 * |        |ascend310B | 0,1,2 |   1   |   2   | 0,1,2 |   0   |       |       |       |       |       |
 * -        ------------------------------------------------------------------------------------------------
 * |  chip  |ascend310p | 0,1,2 |   1   |   2   | 0,1,2 |   0   |       |       |       |       |       |
 * -        ------------------------------------------------------------------------------------------------
 * |  type  |ascend910  |  0,3  |       |   3   |   0   |   0   |  1,2  |   2   |       |  1,2  |       |
 * -        ------------------------------------------------------------------------------------------------
 * |        |ascend910B |       |       |       |       |       | 32~59 | 33~59 | 33~59 | 33~59 |  32   |
 * -        ------------------------------------------------------------------------------------------------
 * |        |ascend610  |   0   |       |   0   |   0   |   0   |       |       |       |       |       |
 * -        ------------------------------------------------------------------------------------------------
 * |        |helper310p |  0,2  |       |   2   |  0,2  |   0   |       |       |       |       |       |
 * ---------------------------------------------------------------------------------------------------------
 */

#define CLOUD_V1_NUMA_PER_DEV                4
#define CLOUD_V1_HBM_NID_OFFSET              2
#define CLOUD_V1_HBM_NID_NUM                 2
#define CLOUD_V1_TS_NID_OFFSET               3

#define MINI_V2_NUMA_PER_DEV                 3
#define MINI_V2_HELPER_DEV_NUM               2
#define MINI_V2_HELPER_DEV_NUMA_NUM          2

#define CLOUD_V2_NUMA_PER_DEV               32
#define CLOUD_V2_NUMA_ID_BASE               32

#ifndef EMU_ST
#define dbl_node_online(node) node_online(node)
#define dbl_num_online_nodes num_online_nodes
#endif

typedef int (*init_dev_nid_info_func)(u32 devid, u32 dev_num, u32 numa_num);
typedef int (*get_ai_nid)(u32 devid, int nids[], int num);

static int g_memctrl_type = 0;
static get_ai_nid g_get_ai_nid = NULL;

static void numa_id_set_memctrl_type(int memctrl_type)
{
    g_memctrl_type = memctrl_type;
}

static int numa_id_config_mini_v1(u32 devid, u32 dev_num, u32 numa_num)
{
    nid_info_t nids_info[1] = {0};
    int nid_num = 1;
    int ret = 0;

    nids_info[0].nid = (int)devid;
    nids_info[0].size.total_size = 0x200000000; /* just 8G, size not used */
    nids_info[0].size.free_size = 0x200000000; /* just 8G, size not used */

    /* DDR */
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_TS, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_CTRL, nids_info, nid_num);

    /* ALL */
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_TS, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_CTRL, nids_info, nid_num);

    numa_id_set_memctrl_type(DBL_NID_MEMCTRL_SHARED_TYPE);
    return ret;
}

/*
 * 910 cloud
 * devid     0  1  2  3     1p:0   2p:0 1   3p:0  1  2
 * numa id
 * DDR       0  1  2  3        0      0 1      0  1  2
 * HBM       4  6  8 10        1      2 4      3  5  7      may support p2p
 * P2P HBM   5  7  9 11        2      3 5      4  6  8
 * ts DDR   12 13 14 15        3      6 7      9 10 11
 */
static int numa_id_config_cloud_v1(u32 devid, u32 dev_num, u32 numa_num)
{
    nid_info_t nids_info[CLOUD_V1_NUMA_PER_DEV] = {0};
    int nid_num = 1;
    int ret = 0;

    /* DDR */
    nid_num = 2;    /* 2 numa node */
    nids_info[0].nid = (int)devid;
    nids_info[1].nid = (int)(devid + dev_num * CLOUD_V1_TS_NID_OFFSET); /* ts */

    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_TS, &nids_info[1], 1);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_AI, &nids_info[0], 1);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_CTRL, &nids_info[0], 1);

    /* HBM */
    nid_num = 2;    /* 2 numa node */
    nids_info[0].nid = (int)(devid * CLOUD_V1_HBM_NID_OFFSET + dev_num);
    nids_info[1].nid = (int)(devid * CLOUD_V1_HBM_NID_OFFSET + dev_num) + 1; /* p2p */

    /* p2p is config by svm, devmm_config_dbl_numa_cloud_v1 */
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_TS, nids_info, nid_num);

    /* ALL */
    nid_num = 4;    /* 4 numa node */
    nids_info[0].nid = (int)devid;
    nids_info[1].nid = (int)(devid * CLOUD_V1_HBM_NID_OFFSET + dev_num);
    nids_info[2].nid = (int)(devid * CLOUD_V1_HBM_NID_OFFSET + dev_num) + 1; /* p2p */
    nids_info[3].nid = (int)(devid + dev_num * CLOUD_V1_TS_NID_OFFSET); /* ts */

    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_TS, &nids_info[3], 1);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_AI, &nids_info[0], 3);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_CTRL, &nids_info[0], 1);

    numa_id_set_memctrl_type(DBL_NID_MEMCTRL_SHARED_TYPE);
    return ret;
}

/*
 * dc
 * devid    1p:0   2P:master,slave
 * numa id
 * DDR      0,1,2  0,2,4(master),1,3,5(slave)
 * P2P DDR  1      2(master),3(slave)
 * TS DDR   2      4(master),5(slave)
 *
 * mdc
 * devid    0
 * numa id
 * DDR      0
 * TS DDR   0
 *
 * helper
 * devid    1p:0   2P:master,slave
 * numa id
 * DDR      0,2    0,2(master),1,3(slave)
 * TS DDR   2      2(master),3(slave)
 */
static int numa_id_config_mini_v2(u32 devid, u32 dev_num, u32 numa_num)
{
    nid_info_t nids_info[MINI_V2_NUMA_PER_DEV] = {0};
    int nid_num;
    int ret = 0;

    if (numa_num == 1) {
        /* mdc 1P */
        nid_num = 1;    /* 1 numa node */
        nids_info[0].nid = 0;

        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_TS, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_CTRL, nids_info, nid_num);

        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_TS, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_CTRL, nids_info, nid_num);
    } else if ((numa_num == MINI_V2_HELPER_DEV_NUMA_NUM) ||
        (numa_num== (MINI_V2_HELPER_DEV_NUM * MINI_V2_HELPER_DEV_NUMA_NUM))) {
        /* helper 1P helper 2P */
        nid_num = 2;    /* 2 numa node */
        nids_info[0].nid = devid;
        nids_info[1].nid = (int)(devid + MINI_V2_HELPER_DEV_NUMA_NUM); /* node ts */
#ifndef EMU_ST
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_TS, &nids_info[1], 1);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_CTRL, &nids_info[0], 1);

        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_TS, &nids_info[1], 1);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_CTRL, &nids_info[0], 1);
#endif
    } else {
        /* dc */
        nid_num = 3;    /* 3 numa node */
        nids_info[0].nid = (int)devid;
        nids_info[1].nid = (int)(devid + dev_num); /* p2p */
        nids_info[2].nid = (int)(devid + dev_num + dev_num); /* node ts */
#ifndef EMU_ST
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_TS, &nids_info[2], 1);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_CTRL, &nids_info[0], 1);

        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_TS, &nids_info[2], 1);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
        ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_CTRL, &nids_info[0], 1);
#endif
    }

    numa_id_set_memctrl_type(DBL_NID_MEMCTRL_SHARED_TYPE);
    return ret;
}

/*
 * cloudv2 fpga 8G
 * die_id    1die:0      2die:0 1
 * numa id
 * 2G             0           0 1
 * 62G            1           2 3
 *
 * cloudv2 asic 16G
 * die_id    1die:0      2die:0 1
 * numa id
 * 16G            0     0 1
 *
 * cloudv2 asic 32G
 * die_id    1die:0      2die:0 1
 * numa id
 * 2G             0      0     1
 * 2G-30G     32-45      32-45 64-77
 *
 * cloudv2 asic 64G
 * die_id    1die:0      2die:0 1
 * numa id
 * 3G             0      0     1 os use to hbm mirro
 * 6G-64G      32-59      32-59 64-92
 *
 */
static int numa_id_config_cloud_v2(u32 devid, u32 dev_num, u32 numa_num)
{
    nid_info_t nids_info[CLOUD_V2_NUMA_PER_DEV] = {0};
    u32 base_nid, i, nid_num;
    int ret = 0;

    /*
     * asic 32G/64G
     * node 0    is for os mirror
     * node 32   is for ctrl
     * node >=33 is for ai
     */

    base_nid = devid * CLOUD_V2_NUMA_PER_DEV + CLOUD_V2_NUMA_ID_BASE;
    for (i = 0, nid_num = 0; i < CLOUD_V2_NUMA_PER_DEV; i++) {
        if (dbl_node_online(base_nid + i)) {
            nids_info[nid_num].nid = (int)(base_nid + i);
            nid_num++;
        }
    }
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_TS, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_CTRL, nids_info, nid_num);

    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_ALL, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_TS, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_AI, nids_info, nid_num);
    ret += dbl_nid_add_dev(devid, DBL_MEMTYPE_ALL, DBL_SUB_MEMTYPE_CTRL, nids_info, nid_num);

    numa_id_set_memctrl_type(DBL_NID_MEMCTRL_UNSHARED_TYPE);
    return ret;
}
#ifndef EMU_ST
static int dbl_nid_get_ai_nid_func_init(u32 chip_type)
{
    if (chip_type >= HISI_CHIP_NUM) {
        ccfg_err("Chip type is wrong. (chip_type = %u)\n", chip_type);
        return -ENODEV;
    }

    if (chip_type == HISI_CLOUD_V1) {
        /* for cloud v1, AI_MEM can only be alloced by svm */
        g_get_ai_nid = dbl_get_ddr_ai_nid;
    } else {
        g_get_ai_nid = dbl_get_all_ai_nid;
    }
    return 0;
}
#endif
int dbl_nid_auto_set_nid(u32 chip_type, u32 dev_num, u32 devid)
{
    init_dev_nid_info_func init_nid_info_func[HISI_CHIP_NUM] = {
        [HISI_MINI_V1] = numa_id_config_mini_v1,
        [HISI_CLOUD_V1] = numa_id_config_cloud_v1,
        [HISI_MINI_V2] = numa_id_config_mini_v2,
        [HISI_CLOUD_V2] = numa_id_config_cloud_v2,
        [HISI_MINI_V3] = numa_id_config_mini_v2,
    };
    u32 numa_num;
    int ret;

    if (chip_type >= HISI_CHIP_NUM) {
        ccfg_err("Chip type is wrong. (chip_type = %u)\n", chip_type);
        return -ENODEV;
    }

    numa_num = dbl_num_online_nodes();
    ret = init_nid_info_func[chip_type](devid, dev_num, numa_num);
    if (ret != 0) {
        ccfg_err("NUMA id auto set fail. (chip_type=%u; devid=%u; dev_num=%u; numa_num=%u)\n",
            chip_type, devid, dev_num, numa_num);
        dbl_nid_del_dev(devid, DBL_MEMTYPE_NUM, DBL_SUB_MEMTYPE_NUM);
        return ret;
    }
#ifndef EMU_ST
    dbl_nid_get_ai_nid_func_init(chip_type);
#endif
    ccfg_info("NUMA id auto set success. (chip_type=%u; devid=%u; dev_num=%u; numa_num=%u)\n",
        chip_type, devid, dev_num, numa_num);
    return 0;
}
EXPORT_SYMBOL(dbl_nid_auto_set_nid);
#ifndef EMU_ST
int dbl_get_ai_nid(u32 devid, int nids[], int num)
{
    return g_get_ai_nid(devid, nids, num);
}
EXPORT_SYMBOL(dbl_get_ai_nid);
#endif
int dbl_nid_get_memctrl_type(void)
{
    return g_memctrl_type;
}
EXPORT_SYMBOL(dbl_nid_get_memctrl_type);


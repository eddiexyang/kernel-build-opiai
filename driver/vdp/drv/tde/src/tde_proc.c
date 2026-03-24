/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: proc manage
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */
#ifdef CONFIG_OT_PROC_SHOW_SUPPORT

#include "tde_proc.h"
#endif
#include "tde_check_para.h"
#include "securec.h"


#ifdef CONFIG_OT_PROC_SHOW_SUPPORT
typedef struct {
    td_u32 cur_node;
    tde_hw_node st_tde_hw_node[TDE_MAX_PROC_NUM];
    td_bool b_proc_enable;
} tde_procinfo;

tde_procinfo g_st_tde_proc_info = {
    .cur_node = 0,
    .b_proc_enable = TD_TRUE,
};

tde_procinfo *g_pst_tde_proc_info = &g_st_tde_proc_info;

td_void tde_proc_record_node(const tde_hw_node *hw_node)
{
    if ((g_pst_tde_proc_info == TD_NULL) || (g_pst_tde_proc_info->b_proc_enable == HI_FALSE) || (hw_node == TD_NULL)) {
        return;
    }

    tde_check_array_over_return_novalue(g_pst_tde_proc_info->cur_node, TDE_MAX_PROC_NUM);
    if (memcpy_s(&g_pst_tde_proc_info->st_tde_hw_node[g_pst_tde_proc_info->cur_node], sizeof(tde_hw_node), hw_node,
        sizeof(tde_hw_node)) != EOK) {
        tde_error("secure function failure\n");
        return;
    }

    g_pst_tde_proc_info->cur_node++;
    g_pst_tde_proc_info->cur_node = (g_pst_tde_proc_info->cur_node) % TDE_MAX_PROC_NUM;

    return;
}

td_s32 tde_read_proc(const osal_proc_entry_t *p)
{
    td_u32 j;
    td_u32 *pu32_cur = TD_NULL;
    tde_hw_node *pst_hw_node = TD_NULL;

    if (p == TD_NULL || g_pst_tde_proc_info == TD_NULL) {
        tde_error("proc entry ptr is %d proc info ptr is %d\n", p == TD_NULL, g_pst_tde_proc_info == TD_NULL);
        return TD_FAILURE;
    }

    pst_hw_node = g_pst_tde_proc_info->st_tde_hw_node;
    p = wprintinfo(p);

    for (j = 0; (j < g_pst_tde_proc_info->cur_node) && (j < TDE_MAX_PROC_NUM); j++) {
        pu32_cur = (td_u32 *)&pst_hw_node[j];
        (void)tde_hal_node_print_info(p, pu32_cur);
    }

    return TD_SUCCESS;
}

#endif

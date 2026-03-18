/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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
 * Create: 2020-4-1
 */

#include "pngd.h"
#include <linux/module.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // end of #ifdef __cplusplus

hi_void pngd_proc_himpi(const struct osal_proc_dir_entry *s)
{
    hi_s32 i;
    pngd_context *p_chn = HI_NULL;

    osal_seq_printf(s,
        "\n----- Detail HOST MPI STATE--------------------------------"
        "--------------------------------------------------\n");

    osal_seq_printf(s, "%4s"
        "%15s"
        "%15s"
        "%15s"
        "%15s"
        "\n",
        "ID", "SendTimerSetup", "SendTimerKill", "SendTimerRun", "SendTimeout");
    for (i = 0; i < PngdMaxChnNum; i++) {
        p_chn = &g_pngd_chn[i];
        if (osal_down(&p_chn->pngd_sem)) {
            return;
        }
        if ((p_chn->chn_state != PNGD_DESTROYED) && (p_chn->chn_state != PNGD_DESTROYING)) {
            osal_seq_printf(s, "%4d"
                "%15llu"
                "%15llu"
                "%15llu"
                "%15llu"
                "\n",
                i,
                g_pngd_proc[i].mpi_timer_setup,
                g_pngd_proc[i].mpi_timer_kill,
                g_pngd_proc[i].mpi_timer_run,
                g_pngd_proc[i].mpi_timer_timeout);
        }
        osal_up(&p_chn->pngd_sem);
    }
}

hi_void pngd_proc_acl(const struct osal_proc_dir_entry *s)
{
    hi_s32 i;
    pngd_context *p_chn = HI_NULL;

    osal_seq_printf(s,
        "\n----- Detail ACL EVENT STATE --------------------------------------"
        "--------------------------------------------------\n");

    osal_seq_printf(s, "%4s"
        "%13s"
        "%20s"
        "%15s"
        "%13s"
        "%13s"
        "%13s"
        "\n",
        "ID", "SendFrame", "SubmitCompleteSucc", "SubmitNotFull", "AclGetFrame",
        "FullState", "FullCount");
    for (i = 0; i < PngdMaxChnNum; i++) {
        p_chn = &g_pngd_chn[i];
        if (osal_down(&p_chn->pngd_sem)) {
            return;
        }
        if ((p_chn->chn_state != PNGD_DESTROYED) && (p_chn->chn_state != PNGD_DESTROYING)) {
            osal_seq_printf(s, "%4d"
                "%13llu"
                "%17llu"
                "%15llu"
                "%13llu"
                "%13u"
                "%13llu"
                "\n",
                i,
                g_pngd_proc[i].mpi_send_strm_times,
                g_pngd_proc[i].acl_submit_complete_succ_times,
                g_pngd_proc[i].acl_submit_not_full_times,
                g_pngd_proc[i].acl_get_frame_times,
                g_pngd_proc[i].acl_is_full,
                g_pngd_proc[i].acl_full_count);
        }
        osal_up(&p_chn->pngd_sem);
    }
}

hi_void pngd_proc_chn_info(const struct osal_proc_dir_entry *s)
{
    hi_s32 i;
    pngd_context *p_chn = HI_NULL;

    osal_seq_printf(s,
        "\n----- Detail Pngd Chn Id Info --------------------------------------"
        "--------------------------------------------------\n");

    osal_seq_printf(s, "%4s" "%15s" "%15s" "%15s\n",
        "ID", "UserChnId", "VFID", "Pid");

    for (i = 0; i < PngdMaxChnNum; i++) {
        p_chn = &g_pngd_chn[i];
        if (osal_down(&p_chn->pngd_sem)) {
            return;
        }
        if ((p_chn->chn_state != PNGD_DESTROYED) && (p_chn->chn_state != PNGD_DESTROYING)) {
            osal_seq_printf(s, "%4d" "%15llu" "%15llu" "%15llu\n",
                i,
                p_chn->user_chn_id,
                p_chn->vf_id,
                p_chn->pid);
        }
        osal_up(&p_chn->pngd_sem);
    }
}

/*****************************************************************************
 prototype    : pngd_proc_show
 description  : support proc infomation for pngd chn running status
 input        : struct seq_file *s
 output       : none
*****************************************************************************/
hi_s32 pngd_proc_show(const struct osal_proc_dir_entry *s)
{
    bool ret = try_module_get(THIS_MODULE);
    if (ret == false) {
        return HI_SUCCESS;
    }
    osal_seq_printf(s, "\n[PNGD] Version: [" MPP_VERSION "]\n\n");

    if (g_pngd_state != PNGD_STATE_STARTED) {
        module_put(THIS_MODULE);
        return HI_SUCCESS;
    }

    osal_atomic_inc_return(&g_pngd_user_ref);

    pngd_proc_acl(s);
    pngd_proc_himpi(s);
    pngd_proc_chn_info(s);

    osal_atomic_dec_return(&g_pngd_user_ref);
    module_put(THIS_MODULE);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // end of #ifdef __cplusplus



/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
* Description:
* Author: Hisilicon multimedia software group
* Create: 2011/06/28
*/
#ifndef MOD_EXT_H__
#define MOD_EXT_H__

#include "hi_osal.h"
#include "hi_type.h"
#include "hi_common_adapt.h"

#define MAX_MPP_MODULES ((hi_s32)HI_ID_VFMW_SERVER + 1)

#define VERSION_MAGIC   20190131

#define MAX_MOD_NAME    16

typedef enum {
    MOD_NOTICE_STOP = 0x11,
} mod_notice_id;

typedef enum {
    MOD_STATE_FREE = 0x11,
    MOD_STATE_BUSY = 0X22,
    MOD_STATE_BYPASS = 0x33,
} mod_state;

typedef enum {
    MOD_REF_GET = 0,
    MOD_REF_PUT,
} module_ref_ctrl;

typedef hi_s32 fn_mod_init(hi_void *);
typedef hi_void fn_mod_exit(hi_void);
typedef hi_void fn_mod_bind(hi_void);
typedef hi_void fn_mod_ref_ctrl(module_ref_ctrl ref_ctrl);
typedef hi_void fn_mod_notify(mod_notice_id notice_id);
typedef hi_void fn_mod_query_state(mod_state *state);
typedef hi_u32 fn_mod_ver_checker(hi_void);

typedef struct {
    struct osal_list_head list;

    hi_char mod_name[MAX_MOD_NAME];
    hi_mod_id mod_id;

    fn_mod_init *pfn_init;
    fn_mod_exit *pfn_exit;
    fn_mod_bind *pfn_bind;
    fn_mod_query_state *pfn_query_state;
    fn_mod_notify *pfn_notify;
    fn_mod_ver_checker *pfn_ver_checker;
    fn_mod_ref_ctrl *pfn_ref_ctrl;
    hi_bool inited;

    hi_void *export_funcs;
    hi_void *data;

    hi_char *version;
    hi_bool has_get_ref;
} umap_module;

extern hi_char *cmpi_get_module_name(hi_mod_id mod_id);
extern umap_module *cmpi_get_module_by_id(hi_mod_id mod_id);
extern hi_void *cmpi_get_module_func_by_id(hi_mod_id mod_id);

extern hi_void cmpi_stop_modules(hi_void);
extern hi_s32 cmpi_query_modules(hi_void);
extern hi_s32 cmpi_init_modules(hi_void);
extern hi_void cmpi_exit_modules(hi_void);
extern hi_s32 cmpi_register_module(umap_module *mod);
extern hi_void cmpi_unregister_module(hi_mod_id mod_id);

#define FUNC_ENTRY(type, id) ((type *)cmpi_get_module_func_by_id(id))
#define CHECK_FUNC_ENTRY(id) (cmpi_get_module_func_by_id(id) != NULL)
#define FUNC_ENTRY_NULL(id) (!CHECK_FUNC_ENTRY(id))
#define func_entry(type, id) ((type *)cmpi_get_module_func_by_id(id))
#define check_func_entry(id) (cmpi_get_module_func_by_id(id) != NULL)
#define func_entry_null(id) (!check_func_entry(id))

#endif /*  MOD_EXT_H__ */


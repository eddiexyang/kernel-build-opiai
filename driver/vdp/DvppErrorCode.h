/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Dvpp Common Head File.
 */

#ifndef HI_DVPP_ERR_CODE_H
#define HI_DVPP_ERR_CODE_H

#define HI_ERR_APPID (0x80000000UL + 0x20000000UL)

#define HI_DEF_ERR(module, level, errid) \
    ((int32_t)((HI_ERR_APPID) | ((module) << 16) | ((level) << 13) | (errid)))

typedef enum hiEN_ERR_CODE_E {
    EN_ERR_INVALID_DEVID = 1, // invlalid device ID
    EN_ERR_INVALID_CHNID = 2, // invlalid channel ID
    EN_ERR_ILLEGAL_PARAM = 3, // at lease one parameter is illagal. eg, an illegal enumeration value, illegal stream
    EN_ERR_EXIST         = 4, // resource exists
    EN_ERR_UNEXIST       = 5, // resource unexists

    EN_ERR_NULL_PTR      = 6, // using a NULL point

    EN_ERR_NOT_CONFIG    = 7, // try to enable or initialize system, device or channel, before configing attribute

    EN_ERR_NOT_SUPPORT   = 8, // operation or type is not supported by NOW
    EN_ERR_NOT_PERM      = 9, // operation is not permitted. eg, try to change static attribute
    EN_ERR_INVALID_PIPEID = 10, // invlalid pipe ID
    EN_ERR_INVALID_STITCHGRPID  = 11, // invlalid stitch group ID

    EN_ERR_NOMEM         = 12, // failure caused by malloc memory
    EN_ERR_NOBUF         = 13, // failure caused by malloc buffer

    EN_ERR_BUF_EMPTY     = 14, // no data in buffer
    EN_ERR_BUF_FULL      = 15, // no buffer for new data

    EN_ERR_SYS_NOTREADY  = 16, // System is not ready, maybe not initialed or loaded
    EN_ERR_BADADDR       = 17, // bad address. eg, used for copy_from_user & copy_to_user

    EN_ERR_BUSY          = 18, // resource is busy. eg, destroy a venc chn without unregister it
    EN_ERR_SIZE_NOT_ENOUGH = 19, // buffer size is smaller than the actual size required
    EN_ERR_TIMEOUT       = 20, // hardware or software timeout
    EN_ERR_SYS_ERROR     = 21, // Internal system error

    EN_ERR_BUTT          = 63, // maxium code, private error code of all modules must be greater than it
} EN_ERR_CODE_E;

typedef enum hiERR_LEVEL_E {
    EN_ERR_LEVEL_DEBUG = 0, // debug-level
    EN_ERR_LEVEL_INFO = 1, // informational
    EN_ERR_LEVEL_NOTICE = 2, // normal but significant condition
    EN_ERR_LEVEL_WARNING = 3, // warning conditions
    EN_ERR_LEVEL_ERROR = 4, // error conditions
    EN_ERR_LEVEL_CRIT = 5, // critical conditions
    EN_ERR_LEVEL_ALERT = 6, // action must be taken immediately
    EN_ERR_LEVEL_FATAL = 7, // just for compatibility with previous version
    EN_ERR_LEVEL_BUTT
} ERR_LEVEL_E;

typedef enum hiMOD_ID_E {
    HI_ID_SYS = 2,
    HI_ID_VDEC = 5,
    HI_ID_VPC = 7,
    HI_ID_VENC = 8,
    HI_ID_JPEGE = 11,
    HI_ID_JPEGD = 14,
    HI_ID_PNGD = 64,
    HI_ID_BUTT = 0x100,
} MOD_ID_E;

#endif // HI_DVPP_ERR_CODE_H
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-3-1
 */
#ifndef __STARS_NOTIFY_TBL_C_UNION_DEFINE_H__
#define __STARS_NOTIFY_TBL_C_UNION_DEFINE_H__
#define STARS_TABLE_NOTIFY_NUM 512

/*
 * DEFINE REGISTER UNION
 */
/* Define the union StarsNotifyTableSlice */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * 0: notify_id has not be written to,
         *    or there was a notify_wait successfully passed and auto cleared
         * 1: notify_id has been written
         *    Software should ensure when writing to this bit, the value was 0.
         *    Only the case that during TS FW initialziation,
         *    we would allow using write to to notify table entry with flag == 1
         */
        unsigned int notifyTableFlagSlice       : 1;    /* [0] */
        /*
         * 1: notify pending  will be clear.
         * 0: notify pending  will not be clear.
         *    when ffts+ notify wait occurs before notify record for this notify ID,
         *    the corresponding pending will be set 1.when SLICE is reset or destroys.
         *    the pending need to be clear.
         */
        unsigned int notifyTablePendingClrSlice : 1;    /* [1] */
        unsigned int reserved                   : 30;   /* [31:2] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} StarsNotifyTableSlice;

/*
 * DEFINE EVENT TABLE INFO STRUCT
 */
typedef struct {
    StarsNotifyTableSlice starsNotifyTableSlice[STARS_TABLE_NOTIFY_NUM];
    unsigned int          reserved[15872];
} StarsNotifyTableSliceInfo;

/*
 * DEFINE GLOBAL STRUCT
 */
typedef struct {
    StarsNotifyTableSliceInfo StarsNotifyGroupTable[16];
} StarsNotifyTblRegsType;

#endif /* __STARS_NOTIFY_TBL_C_UNION_DEFINE_H__ */

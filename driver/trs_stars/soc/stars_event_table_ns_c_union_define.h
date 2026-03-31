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
#ifndef __STARS_EVENT_TABLE_NS_C_UNION_DEFINE_H__
#define __STARS_EVENT_TABLE_NS_C_UNION_DEFINE_H__
#define STARS_TABLE_EVENT_NUM 4096
/*
 * DEFINE REGISTER UNION
 */
/* Define the union StarsEventTable */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * 1：event record has been occurred for this event ID
         * 0：event record has not been occurred for this event ID
         */
        unsigned int eventTableFlag          : 1;       /* [0] */
        /*
         * 1: event wait occurs before event record for this event ID
         * 0: event wait does not occur before event record for this event ID
         */
        unsigned int eventTablePending       : 1;       /* [1] */
        unsigned int reserved                : 30;      /* [31:2] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} StarsEventTable;
/* Define the union StarsCrossSocEventTable */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * 1：event record has been occurred for this event ID
         * 0：event record has not been occurred for this event ID
         */
        unsigned int crossSocEventTableFlag    : 1;       /* [0] */
        /*
         * 1: event wait occurs before event record for this event ID
         * 0: event wait does not occur before event record for this event ID
         */
        unsigned int crossSocEventTablePending : 1;       /* [1] */
        unsigned int reserved                  : 30;      /* [31:2] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} StarsCrossSocEventTable;
/* Define the union StarsEventTableBaseAddrLowNs */
typedef union {
    /* Define the struct bits */
    struct {
        /* The lower 32bit base address in HBM for storing event flag/pending. */
        unsigned int starsEventTableBaseAddrLowNs : 32;      /* [31:0] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} StarsEventTableBaseAddrLowNs;
/* Define the union StarsEventTableBaseAddrHighNs */
typedef union {
    /* Define the struct bits */
    struct {
        /* The higher 17bit base address in HBM for storing event flag/pending. */
        unsigned int starsEventTableBaseAddrHighNs : 17;      /* [16:0] */
        unsigned int reserved                : 15;      /* [31:17] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} StarsEventTableBaseAddrHighNs;
/* Define the union StarsEventTableBaseAddrIsVirtualNs */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * 1：the address which event flag/pending store in the HBM is virtual address in the cache mode
         * 0: the address which event flag/pending store in the HBM is not virtual address in the cache mode
         */
        unsigned int starsEventTableBaseAddrIsVirtualNs : 1;       /* [0] */
        unsigned int reserved                : 31;      /* [31:1] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} StarsEventTableBaseAddrIsVirtualNs;
/* Define the union StarsEventTableStreamIdNs */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * Corresponding to the requestID in the Axuser field,
         * it is the stream number output by the module,
         * which is the low part of the StreamID, 16 bits, that is 65536 streams;
         */
        unsigned int starsEventTableSubstreamIdNs : 16;      /* [15:0] */
        /*
         * SubStreamID is the substream number of StreamID. When in use,
         * the SMMU is queried together with StreamID. For PCIe devices, PASID is SubstreamID;
         */
        unsigned int starsEventTableStreamIdNs : 16;      /* [31:16] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} StarsEventTableStreamIdNs;
/* Define the union StarsEventTableCtrl */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * event table mode configuration.（static configuration ）
         * 0: cache mode, event id is 0~65535.
         *    Cache size is 4096, most event id flag&pending is stored in HBM.
         *    Frequent cacheline replacement may introduce greater latency for a certain event id.
         *    But in this mode, event id can range from 0 to 65535.
         * 1: table mode, event id is fixed 0~4095.
         *    In this mode, all event id flag&pending is stored in the registers.
         *    Latency performance is better.
         */
        unsigned int starsEventTableMode     : 1;       /* [0] */
        /*
         * event table isolation mode configuration(static configuration)
         * 0:normal mode
         * There is no event id isolation. All of event id can be allocated to all of cacheline.
         * 1:isolation mode( only enabled in event table cache mode)
         * 65536 event id are divided into 16 equal parts, each part can only be allcated to a fixed cacheline.
         */
        unsigned int starsEventTableIsoMode  : 1;       /* [1] */
        /*
         * addr check enable between sft rd and reqs in sft wr buffer
         * 0:disable
         * 1:enable
         */
        unsigned int starsEventTableSftConflictEn : 1;       /* [2] */
        unsigned int reserved                : 29;      /* [31:3] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} StarsEventTableCtrl;
/* Define the union StarsEventSecLock */
typedef union {
    /* Define the struct bits */
    struct {
        /*
         * security lock for registers whose field attribute are RW_FIELD_LOCKED。
         * If this register is not default value,
         * registers whose field attribute are RW_FIELD_LOCKED are locked and
         * can be read by non-secure/secure operation；otherwise
         * they are unlocked and can be read/written by non-secure/secure operation
         */
        unsigned int eventBaseAddrLocker     : 32;      /* [31:0] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} StarsEventSecLock;

/*
 * DEFINE EVENT TABLE INFO STRUCT
 */
typedef struct {
    StarsEventTable                      starsEventTable[4096];
    unsigned int                         reserved0[4096];
    StarsCrossSocEventTable              starsCrossSocEventTable[4096];
    unsigned int                         reserved1[4096];
} StarsEventGroupTableInfo;

/*
 * DEFINE GLOBAL STRUCT
 */
typedef struct {
    StarsEventGroupTableInfo             StarsEventGroupTable[16];
    StarsEventTableBaseAddrLowNs         starsEventTableBaseAddrLowNs;        /* 100000 */
    StarsEventTableBaseAddrHighNs        starsEventTableBaseAddrHighNs;       /* 100004 */
    StarsEventTableBaseAddrIsVirtualNs   starsEventTableBaseAddrIsVirtualNs;  /* 100008 */
    StarsEventTableStreamIdNs            starsEventTableStreamIdNs;           /* 10000c */
    StarsEventTableCtrl                  starsEventTableCtrl;                 /* 100010 */
    StarsEventSecLock                    starsEventSecLock;                   /* 100014 */
} StarsEventTableNsRegsType;
#endif /* __STARS_EVENT_TABLE_NS_C_UNION_DEFINE_H__ */

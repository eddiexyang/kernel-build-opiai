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

#ifndef HI_DVPP_SYS_H
#define HI_DVPP_SYS_H

#include "HiDvppCommon.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HI_ERR_SYS_ILLEGAL_PARAM 0xA0028003
#define HI_ERR_SYS_EXIST         0xA0028004
#define HI_ERR_SYS_UNEXIST       0xA0028005
#define HI_ERR_SYS_NULL_PTR      0xA0028006
#define HI_ERR_SYS_NOT_SUPPORT   0xA0028008
#define HI_ERR_SYS_NOT_PERM      0xA0028009
#define HI_ERR_SYS_NOMEM         0xA002800C
#define HI_ERR_SYS_NOTREADY      0xA0028010
#define HI_ERR_SYS_BUSY          0xA0028012
#define HI_ERR_SYS_ERR           0xA0028015

// Deprecated by 2021-10-22
enum DvppVersion {
    VERSION_100 = 100, // T version
    VERSION_200 = 200, // F version
    VERSION_300 = 300  // M version
};

typedef enum hiDVPP_EPOLL_CTL_OP_E {
    DVPP_EPOLL_CTL_ADD = 1,
    DVPP_EPOLL_CTL_MOD = 2,
    DVPP_EPOLL_CTL_DEL = 3,
    DVPP_EPOLL_CTL_BUTT
} DVPP_EPOLL_CTL_OP_E;

typedef enum hiDVPP_EPOLL_EVENT_TYPE_E {
    DVPP_EPOLL_IN = 1u,
    DVPP_EPOLL_OUT = 1u << 1u,
    DVPP_EPOLL_ET = (HI_U32)1u << 31u
} DVPP_EPOLL_EVENT_TYPE_E;

typedef struct hiDVPP_EPOLL_EVENT_S {
    HI_U32 u32Events;
    HI_VOID* pData;
} DVPP_EPOLL_EVENT_S;

/*
* Deprecated by 2021-10-22
*
* @brief : get dvpp version
* @return : DvppVersion
*/
enum DvppVersion GetVersion(HI_VOID);

/*
 * @brief init mpp system
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_SYS_Init(HI_VOID);

/*
 * @brief exit mpp system
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_SYS_Exit(HI_VOID);

/*
 * Deprecated by 2021-10-22
 *
 * @brief map physical address memory
 * @param [in] u64PhyAddr: physical address
 * @param [in] u32Size: size of memory
 * @return success: return valid virtual address
 *         fail: return 0
 */
HI_VOID *HI_MPI_SYS_Mmap(HI_U64 u64PhyAddr, HI_U32 u32Size);

/*
 * Deprecated by 2021-10-22
 *
 * @brief unmap memory
 * @param [in] pVirAddr: virtual address
 * @param [in] u32Size: size of memory
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_SYS_Munmap(HI_VOID *pVirAddr, HI_U32 u32Size);

/**
 * @brief Dvpp Epoll create interface
 * @param [in] s32Size: Specify the number of dvpp channel descriptors to monitor,
 *                      currently ignored, and must be positive
 * @param [out] ps32EpollFd: return the descriptor referring to the new dvpp epoll instance
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_SYS_CreateEpoll(HI_S32 s32Size, HI_S32 *ps32EpollFd);

/**
 * @brief Dvpp Epoll control interface
 * @param [in] s32EpollFd: descriptor of dvpp epoll instance
 * @param [in] s32Operation: operation type for the target dvpp channel descriptor referred by @s32Fd
 * @param [in] s32Fd: descriptor of target dvpp channel
 * @param [in] pEvent: describes the event type wants to monitor, and also the data go with the channel descriptor
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_SYS_CtlEpoll(HI_S32 s32EpollFd, HI_S32 s32Operation, HI_S32 s32Fd, DVPP_EPOLL_EVENT_S *pEvent);

/**
 * @brief Dvpp Epoll wait interface
 * @param [in] s32EpollFd: descriptor of dvpp epoll instance
 * @param [out] pEvents: return events that happened
 * @param [in] s32MaxEvents: maxinum numer of events can return
 * @param [in] s32Timeout: milliseconds the caller can block
 * @param [out] ps32EventNum: return the numer of events saved in @pEvents
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_SYS_WaitEpoll(HI_S32 s32EpollFd, DVPP_EPOLL_EVENT_S *pEvents, HI_S32 s32MaxEvents, HI_S32 s32Timeout,
                            HI_S32 *ps32EventNum);

/**
 * @brief Dvpp Epoll close interface
 * @param [in] ps32EpollFd:  descriptor of dvpp epoll instance
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_SYS_CloseEpoll(HI_S32 s32EpollFd);

/*
 * @brief set csc matrix coefficient
 * @param [in] mode: dvpp module
 * @param [in] chn: chn id
 * @param [in] cscMatrix: csc matrix mode
 * @param [in] cscCoefficient: csc matrix coefficient when use user mode
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_SYS_SetChnCscMatrix(MOD_ID_E mode, HI_S32 chn, CSC_MATRIX_E cscMatrix, CSC_COEFFICIENT_S *cscCoefficient);

/*
 * @brief get csc matrix coefficient
 * @param [in] mode: dvpp module
 * @param [in] chn: chn id
 * @param [out] cscMatrix: csc matrix mode
 * @param [out] cscCoefficient: csc matrix coefficient for all mode
 * @return success: return 0
 *         fail: return error number
 */
HI_S32 HI_MPI_SYS_GetChnCscMatrix(MOD_ID_E mode, HI_S32 chn, CSC_MATRIX_E *cscMatrix,
                                  CSC_COEFFICIENT_S *cscCoefficient);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // #ifndef HI_DVPP_SYS_H

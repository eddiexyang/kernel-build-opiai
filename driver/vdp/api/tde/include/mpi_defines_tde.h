/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: tde api define
 * Author: Hisilicon multimedia software group
 * Create: 2023/3/10
 */

#ifndef MPI_DEFINES_TDE_H
#define MPI_DEFINES_TDE_H

#include "vdp_comm_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define tde_mpi_fatal(fmt, ...) VDP_EMERG_TRACE("[lib_tde][%s]:" fmt, __FUNCTION__, ##__VA_ARGS__)
#define tde_mpi_error(fmt, ...) VDP_ERR_TRACE("[lib_tde][%s]:" fmt, __FUNCTION__, ##__VA_ARGS__)
#define tde_mpi_warning(fmt, ...) VDP_WARN_TRACE("[lib_tde][%s]:" fmt, __FUNCTION__, ##__VA_ARGS__)
#define tde_mpi_info(fmt, ...) VDP_DEBUG_TRACE("[lib_tde][%s]:" fmt, __FUNCTION__, ##__VA_ARGS__)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* MPI_DEFINES_TDE_H */

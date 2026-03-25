/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASCEND_XPC_KERNEL_INTER_H__
#define __ASCEND_XPC_KERNEL_INTER_H__

#include <linux/errno.h>
#include <linux/types.h>

#define XPC_CHANNEL_NAME_MAX 32

#define CHL_O_CREATE 0

#define XPC_CHL_STATE_OPEN 1

#define XPC_ERR_POLL_TIMEOUT (-ETIMEDOUT)
#define XPC_ERR_POLL_CANCELED_BY_OTHERS (-ECANCELED)
#define XPC_ERR_CHL_EMPTY (-ENODATA)

struct chl_info {
	char name[XPC_CHANNEL_NAME_MAX];
	u32 flags;
	u32 tx_size;
	u32 rx_size;
};

struct chl_poll_ret {
	int id;
	u32 events;
};

static inline int xpcshm_open_channel(const struct chl_info *info, int mode)
{
	return -ENOTSUPP;
}

static inline int xpcshm_close_channel(int chl_id)
{
	return 0;
}

static inline int xpcshm_get_chl_state(int chl_id)
{
	return 0;
}

static inline int xpcshm_poll(const int *chl_id, int chl_num, struct chl_poll_ret *poll_ret, int *res_num, int timeout)
{
	return -ENOTSUPP;
}

static inline int xpcshm_read(int chl_id, void *buf, int *len, int *pack_type)
{
	return -ENOTSUPP;
}

static inline int xpcshm_write(int chl_id, const void *buf, int len)
{
	return -ENOTSUPP;
}

#endif

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
*/
#ifdef LPM_FAULT_HEARTBEAT
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/timer.h>
#include <linux/time64.h>
#include <linux/version.h>
#include "devdrv_manager_comm.h"
#include "dms_interface.h"
#include "dms_sensor_type.h"
#include "devdrv_dfm.h"
#include "lpm_init.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_debugfs_base.h"
#include "lpm_fault_debugfs.h"
#endif
#include "lpm_fault_sharemem.h"
#include "lpm_fault_sharemem_v2.h"
#include "lpm_fault_report.h"
#include "lpm_fault_config.h"
#include "lpm_fault_ipc.h"
#include "lpm_fault_heartbeat.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_heartbeat_debugfs_init(void);
#endif

STATIC struct lpm_fault_heartbeat_priv *lpm_heartbeat_priv_info(void)
{
	static struct lpm_fault_heartbeat_priv lpm_hb_priv;
	return &lpm_hb_priv;
}

STATIC void lpm_heartbeat_report_fault(uint32_t dev_id, bool is_alarm)
{
#if !defined(CFG_SOC_PLATFORM_MINI)
	int32_t ret;
	struct lpm_inner_fault_event event = {0};

	event.dev_id      = dev_id;
	event.node_type   = DMS_DEV_TYPE_LPM;
	event.sensor_type = DMS_SEN_TYPE_HEARTBEAT;
	event.event_type  = LPM_FAULT_EVENT_HEARTBEAT_LOST;
	event.assertion   = is_alarm ? (uint32_t)DMS_EVENT_TYPE_OCCUR : (uint32_t)DMS_EVENT_TYPE_RESUME;
	ret = lpm_handle_fault(&event);
	if (ret != 0) {
		lpm_log_err("lpm heartbeat report alarm failed, dev_id=%u, ret=%d, is_alarm=%u\n",
			dev_id, ret, (uint32_t)is_alarm);
	}
#else
	// mini not support report alarm
	(void)dev_id;
	(void)is_alarm;
#endif
}

STATIC struct lpm_heartbeat_cfg *lpm_heartbeat_dev_priv_info(uint32_t dev_id)
{
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();
	// internal function, the caller guarantees the legality of the parameters
	return &hb_priv->hb_cfg[dev_id];
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC bool lpm_heartbeat_check_fault_flag(enum lpm_heartbeat_debug_type debug_type, uint32_t dev_id)
{
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	// fault inject return true, not inject return false
	if (hb_priv->hb_debug_cfg.fault_inject[(uint32_t)debug_type].inject_flag) {
		if ((hb_priv->hb_debug_cfg.fault_inject[(uint32_t)debug_type].dev_id == LPM_HB_INJECT_ALL_DEV) ||
			(hb_priv->hb_debug_cfg.fault_inject[(uint32_t)debug_type].dev_id == dev_id)) {
			return true;
		}
	}
	return false;
}

STATIC void lpm_heartbeat_init_fault_flag(void)
{
	uint32_t i;
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	for (i = 0; i < (uint32_t)LPM_HB_FAULT_TYPE_MAX; i++) {
		hb_priv->hb_debug_cfg.fault_inject[i].dev_id      = LPM_DMS_NODE_MAX_NUM;
		hb_priv->hb_debug_cfg.fault_inject[i].inject_flag = false;
	}
}
#endif

STATIC void lpm_heartbeat_init_priv_info(uint32_t dev_num)
{
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	(void)memset_s(hb_priv->hb_cfg, sizeof(hb_priv->hb_cfg), 0, sizeof(hb_priv->hb_cfg));

	hb_priv->dev_num = dev_num;
	hb_priv->common_cfg.is_resume_check = LPM_FAULT_HEART_BEAT_RESUME_CHECK;
	hb_priv->common_cfg.is_recovery_check = LPM_FAULT_HEART_BEAT_RECOVERY_CHECK;
	hb_priv->common_cfg.is_sharemem_check = LPM_FAULT_HEART_BEAT_SHAREMEM_CHECK;

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	lpm_heartbeat_init_fault_flag();
#endif
}

STATIC bool lpm_heartbeat_check_is_skip_ipc(uint32_t dev_id)
{
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	if (!hb_priv->hb_cfg[dev_id].hb_en) {
		return true;
	}
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	if (lpm_heartbeat_check_fault_flag(LPM_HB_UNHANDLE_IPC, dev_id)) {
		// fault inject, not handle ipc
		return true;
	}
#endif
	return false;
}

STATIC bool lpm_heartbeat_check_is_enable(uint32_t dev_id)
{
	uint32_t env_type = lpm_common_get_env_type(dev_id);
	if (env_type == LPM_FAULT_PLAT_TYPE_ESL) {
		lpm_log_info("dev_id=%u esl does not support ipc, run without sending heart beat to lp.\n", dev_id);
		return false;
	}

	if (!lpm_fault_query_lp_startup_status(dev_id)) {
		lpm_log_err("dev_id=%u lp startup unready, run without sending heart beat to lp.\n", dev_id);
		return false;
	}

	return true;
}

STATIC void lpm_heartbeat_start(struct lpm_heartbeat_cfg *hb_dev_priv)
{
	hb_dev_priv->stop = 0;
}

STATIC void lpm_heartbeat_stop(struct lpm_heartbeat_cfg *hb_dev_priv)
{
	hb_dev_priv->stop = 1;
}

static inline void lpm_heartbeat_reset_seq(struct lpm_heartbeat_seq *hb_seq)
{
	// default value
	hb_seq->ipc_counter = LPM_HEARTBEAT_SEQ_INVALID;
	hb_seq->sm_counter  = 0;
}

STATIC void lpm_heartbeat_clear_queue(struct lpm_heartbeat_cfg *hb_dev_priv)
{
	uint32_t i;

	// Attention:
	// the caller guarantees that the call to the modified function
	// has applied for a hb_spinlock
	for (i = 0; i < LPM_HEARTBEAT_QUEUE_MAX; i++) {
		lpm_heartbeat_reset_seq(&hb_dev_priv->queue[i]);
	}

	hb_dev_priv->queue_ptr = 0;
}

STATIC void lpm_heartbeat_status_change_handle(uint32_t dev_id, bool is_alarm)
{
	struct timespec64 os_time;
	excep_time timestamp;
	uint32_t excep_id = LPM_HEARTBEAT_LOST_BBOX_CODE;

	lpm_common_get_current_time(&os_time);
	timestamp.tv_sec = (unsigned long long)os_time.tv_sec;
	timestamp.tv_usec = (unsigned long long)os_time.tv_nsec / (unsigned long long)1000;

	if (is_alarm) {
		lpm_log_err("dev_id=%u detect broken time:%lld(s), %ld(ns).\n",
			dev_id, os_time.tv_sec, os_time.tv_nsec);
	} else {
		lpm_log_err("dev_id=%u detect recovery time:%lld(s), %ld(ns).\n",
			dev_id, os_time.tv_sec, os_time.tv_nsec);
		// bit 28-29: 01: recovery, 10: alarm
		excep_id = (excep_id & (uint32_t)0xcfffffffU) | ((uint32_t)0x1 << 0x1C);
	}

#if defined(CFG_SOC_PLATFORM_MINI)
	mntn_system_error(excep_id, timestamp, 0);
#else
	bbox_system_error(dev_id, excep_id, &timestamp, 0);
#endif
	lpm_heartbeat_report_fault(dev_id, is_alarm);
}

STATIC bool lpm_heartbeat_set_ipc_block_status(struct lpm_heartbeat_cfg *hb_dev_priv, uint32_t new_status)
{
	// record heartbeat ipc block occur or recover
	// new_status: 0x1 means ipc block occur, 0x0 means ipc block recover
	// return true: means ipc block status changed
	if (hb_dev_priv->ipc_block != new_status) {
		hb_dev_priv->ipc_block = (uint8_t)new_status;
		return true;
	}
	return false;
}

STATIC void lpm_heartbeat_record_ipc_block_status(
	uint32_t dev_id, uint32_t ipc_counter, bool status_change, bool is_recovery)
{
	if (status_change) {
		// record heartbeat ipc block recover info
		const char *status_desc[] = {"occur", "recovery"};
		struct timespec64 os_time;
		lpm_common_get_current_time(&os_time);
		lpm_log_warn("dev_id=%u detect heartbeat ipc block %s, ipc_counter=%u, time:%lld(s), %ld(ns).\n",
			dev_id, is_recovery ? status_desc[0x1] : status_desc[0x0], ipc_counter,
			os_time.tv_sec, os_time.tv_nsec);
	}
}

STATIC void lpm_heartbeat_record_ipc_ts(uint32_t dev_id, struct lpm_heartbeat_seq *last_seq)
{
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);
	uint8_t curr_ptr = hb_dev_priv->ipc_data_ptr;
	uint8_t prev_ptr;
	uint8_t pprev_ptr;
	uint32_t i;

	lpm_log_err("dev_id=%u lpm heart beat broken, last error heart beat number: ipc[%u] sm[%u].\n",
		dev_id, last_seq->ipc_counter, last_seq->sm_counter);

	// dfx:
	// Record the sending timestamp of broken ipc
	for (i = 0; i < LPM_HEARTBEAT_QUEUE_MAX; i++) {
		curr_ptr = (curr_ptr + (uint8_t)LPM_HEARTBEAT_QUEUE_MAX - (uint8_t)0x1) % (uint8_t)LPM_HEARTBEAT_QUEUE_MAX;
		if (hb_dev_priv->ipc_data[curr_ptr].count != last_seq->ipc_counter) {
			if (i == 0) {
				// record send timestamp of the last heartbeat ipc
				lpm_log_err("send timestamp of the last heartbeat ipc: %u(%u-%u-%u).\n",
					hb_dev_priv->ipc_data[curr_ptr].count, hb_dev_priv->ipc_data[curr_ptr].time_sec_high,
					hb_dev_priv->ipc_data[curr_ptr].time_sec_low, hb_dev_priv->ipc_data[curr_ptr].time_usec_low);
			}
			continue;
		}

		prev_ptr = (curr_ptr + (uint8_t)LPM_HEARTBEAT_QUEUE_MAX - (uint8_t)0x1) % (uint8_t)LPM_HEARTBEAT_QUEUE_MAX;
		pprev_ptr = (prev_ptr + (uint8_t)LPM_HEARTBEAT_QUEUE_MAX - (uint8_t)0x1) % (uint8_t)LPM_HEARTBEAT_QUEUE_MAX;
		lpm_log_err("heartbeat broken number timestamp: %u(%u-%u-%u), %u(%u-%u-%u), %u(%u-%u-%u).\n",
			hb_dev_priv->ipc_data[pprev_ptr].count, hb_dev_priv->ipc_data[pprev_ptr].time_sec_high,
			hb_dev_priv->ipc_data[pprev_ptr].time_sec_low, hb_dev_priv->ipc_data[pprev_ptr].time_usec_low,
			hb_dev_priv->ipc_data[prev_ptr].count, hb_dev_priv->ipc_data[prev_ptr].time_sec_high,
			hb_dev_priv->ipc_data[prev_ptr].time_sec_low, hb_dev_priv->ipc_data[prev_ptr].time_usec_low,
			hb_dev_priv->ipc_data[curr_ptr].count, hb_dev_priv->ipc_data[curr_ptr].time_sec_high,
			hb_dev_priv->ipc_data[curr_ptr].time_sec_low, hb_dev_priv->ipc_data[curr_ptr].time_usec_low);
		return;
	}
}

STATIC enum lpm_heartbeat_status lpm_heartbeat_check_broken(
	struct lpm_heartbeat_cfg *hb_dev_priv, struct lpm_heartbeat_seq *last_seq)
{
	bool ipc_continous = true; // true: 3 consecutive ipc did not receive ack
	bool sm_unchange = true; // true: 3 consecutive counter read from sharemem count did not change
	uint32_t i;
	struct lpm_heartbeat_seq *queue = hb_dev_priv->queue;
	struct lpm_heartbeat_seq *prev  = NULL;
	struct lpm_heartbeat_seq *curr  = NULL;

	if (hb_dev_priv->queue_ptr < LPM_HEARTBEAT_BROKEN_CNT) {
		// not need last_seq info
		return LPM_HEART_NORMAL;
	}

	prev = &queue[0];
	for (i = 1; i < hb_dev_priv->queue_ptr; i++) {
		curr = &queue[i];
		if ((prev->ipc_counter + 0x1U) != curr->ipc_counter) {
			// non-continuous ipc means that the heartbeat is not lost
			ipc_continous = false;
			break;
		}
		if (prev->sm_counter != curr->sm_counter) {
			sm_unchange = false;
		}
		prev = curr;
	}

	// record the last heartbeat counter
	last_seq->ipc_counter = curr->ipc_counter;
	last_seq->sm_counter  = curr->sm_counter;

	if (!ipc_continous) {
		// delete oldest ipc seq info
		for (i = 0x1; i < hb_dev_priv->queue_ptr; i++) {
			queue[i - 0x1U].ipc_counter = queue[i].ipc_counter;
			queue[i - 0x1U].sm_counter  = queue[i].sm_counter;
		}
		hb_dev_priv->queue_ptr = hb_dev_priv->queue_ptr - 0x1U;
		return LPM_HEART_NORMAL;
	}

	//  ipc block, delete all ipc seq info
	lpm_heartbeat_clear_queue(hb_dev_priv);

	// ipc block and heartbeat seq read from sharemem not change
	// it means that the heartbeat is lost
	return sm_unchange ? LPM_HEART_BROKEN : LPM_HEART_IPC_BLOCK;
}

STATIC int32_t lpm_heartbeat_broken_judge(uint32_t dev_id)
{
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);
	unsigned long flags;
	struct lpm_heartbeat_seq last_seq = {0};
	enum lpm_heartbeat_status hb_status = LPM_HEART_NORMAL;

	spin_lock_irqsave(&hb_dev_priv->hb_spinlock, flags);
	hb_status = lpm_heartbeat_check_broken(hb_dev_priv, &last_seq);
	if (hb_status == LPM_HEART_NORMAL) {
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
		return 0;
	} else if (hb_status == LPM_HEART_IPC_BLOCK) {
		bool status_change = lpm_heartbeat_set_ipc_block_status(hb_dev_priv, 0x1);
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
		// reocrd log if ipc block status changed
		lpm_heartbeat_record_ipc_block_status(dev_id, last_seq.ipc_counter, status_change, false);
		return 0;
	} else { // broken
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
		hb_dev_priv->broken = 1;
		lpm_heartbeat_record_ipc_ts(dev_id, &last_seq); // reocrd log
		lpm_heartbeat_status_change_handle(dev_id, true); // report heartbeat broken alarm to bbox and FM
		return -ETIMEDOUT;
	}
}

STATIC int32_t lpm_heartbeat_recovery_judge(uint32_t dev_id)
{
	unsigned long flags;
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);

	// the last heartbeat ipc sending failed, continue sending heartbeat ipc
	// or if ipc has not been sent yet, continue to send ipc
	if (hb_dev_priv->send_ipc_status != LPM_HEARTBEAT_IPC_SEND_SUCC) {
		return 0;
	}

	// the latest heartbeat ipc is sent successfully,
	// and the heartbeat ipc ack is received,
	// the heartbeat is considered to be restored
	// report heartbeat broken recovery and change broken status
	spin_lock_irqsave(&hb_dev_priv->hb_spinlock, flags);
	if (hb_dev_priv->queue_ptr == 0) {
		// report heartbeat broken
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
		hb_dev_priv->broken = 0;
		lpm_heartbeat_status_change_handle(dev_id, false);
	} else {
		// no heartbeat ipc ack received,
		// clear the queue and continue sending heartbeat ipc
		lpm_heartbeat_clear_queue(hb_dev_priv);
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
	}

	return 0;
}

STATIC int32_t lpm_heartbeat_judge(uint32_t dev_id)
{
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);

	// return value:
	// -ETIMEDOUT: means heartbeat broken
	if (hb_dev_priv->broken == 0) {
		return lpm_heartbeat_broken_judge(dev_id);
	} else {
		return lpm_heartbeat_recovery_judge(dev_id);
	}
}

STATIC void lpm_heartbeat_record_ipc_data(
	uint32_t dev_id, struct lpm_heartbeat_ipc_data *hb_data)
{
	uint8_t ptr;
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);

	// dfx:
	// Record the sending timestamp of ipc and
	// and the sequence number of the ipc message
	ptr = hb_dev_priv->ipc_data_ptr;
	hb_dev_priv->ipc_data[ptr].count         = hb_data->count;
	hb_dev_priv->ipc_data[ptr].time_sec_high = hb_data->time_sec_high;
	hb_dev_priv->ipc_data[ptr].time_sec_low  = hb_data->time_sec_low;
	hb_dev_priv->ipc_data[ptr].time_usec_low = hb_data->time_usec_low;
	hb_dev_priv->ipc_data_ptr = (ptr + (uint8_t)1) % (uint8_t)LPM_HEARTBEAT_QUEUE_MAX;
}

STATIC int32_t lpm_heartbeat_send_ipc(uint32_t dev_id, uint8_t hb_seq_count)
{
	struct lpm_heartbeat_ipc_data hb_data = {0};
	struct lpm_fault_ipc_msg ipc_msg;
	struct timespec64 interval = {0};
	struct timespec64 os_time = {0};
	long seconds;
	int32_t ret;

	// interval = os time + time zone
	lpm_common_get_current_time(&os_time);
	lpm_common_get_time_interval(&interval);
	seconds = os_time.tv_sec + interval.tv_sec;
	seconds = (seconds < 0) ? 0 : seconds;

	hb_data.count          = hb_seq_count;
	hb_data.time_sec_low   = (uint32_t)((uint64_t)seconds & 0xFFFFFFFFU);
	hb_data.time_sec_high  = (uint32_t)((uint64_t)seconds >> 0x20) & 0xFFFFFFFFU;
	hb_data.time_usec_low  = (uint32_t)((uint64_t)os_time.tv_nsec / (uint64_t)LPM_NS_TO_US) & 0xFFFFFFFFU;

	ipc_msg.send_type = LPM_IPC_NOTIFY_HB;
	ipc_msg.in        = &hb_data;
	ipc_msg.in_len    = (uint32_t)sizeof(hb_data);
	ipc_msg.out       = NULL;
	ipc_msg.out_len   = 0;

	ret = lpm_fault_ipc_send_async_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("ipc send heartbeat msg failed, ret=%d, "
			"dev_id=%u, count=%u, os_sec=%lld, os_nsec=%ld, interval_sec=%lld\n",
			ret, dev_id, hb_data.count, os_time.tv_sec, os_time.tv_nsec, interval.tv_sec);
		return ret;
	}

	// record ipc send timestamp
	lpm_heartbeat_record_ipc_data(dev_id, &hb_data);

	return 0;
}

STATIC void lpm_heartbeat_check_time_cycle(
	uint32_t dev_id, unsigned long current_timestamp, unsigned long last_timestamp)
{
	uint32_t time_cycle;

	// first heartbeat cycle, no need check
	if (last_timestamp == 0) {
		return;
	}

	time_cycle = jiffies_to_msecs(current_timestamp - last_timestamp);
	if (time_cycle > LPM_HEARTBEAT_TIME_THRESHOLD_WARN) {
		lpm_log_warn("dev_id=%u heart beat time interval over %u ms, time_cycle: %u ms, last_timestamp: %lu\n",
			dev_id, LPM_HEARTBEAT_TIME_THRESHOLD_WARN, time_cycle, last_timestamp);
	}
}

STATIC bool lpm_heartbeat_check_is_stop(int32_t result)
{
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	// if the heartbeat recovery detection is supported,
	// there is no need to stop heartbeat ipc sending
	if (hb_priv->common_cfg.is_recovery_check) {
		return false;
	}

	// if the heartbeat recovery detection is not supported,
	// the heartbeat ipc sending needs to be stopped when the heartbeat broken
	if (result == -ETIMEDOUT) {
		return true;
	}

	// if the heartbeat recovery detection is not supported,
	// it is necessary to continue sending heartbeat ipc when there is no heartbeat broken
	return false;
}

STATIC bool lpm_heartbeat_check_resume_done(uint32_t dev_id)
{
#if defined(LPM_SHAREMEM_V2)
	int32_t ret;
	bool in_suspend = false;
	static bool read_fail = false; // true: read sharemem fail
	static bool suspend_status = false;

	if (read_fail) {
		// There used to be a failure to read the sharemem
		// probably because the version does not match
		// no longer read, to avoid log swiping
		return true;
	}

	ret = lpm_get_suspend_status_from_sharemem(dev_id, &in_suspend);
	if (ret != 0) {
		lpm_log_err("read suspend status from sharemem failed, dev_id=%u, ret=%d\n", dev_id, ret);
		read_fail = true;
		// read sharemem fail, send ipc
		return true;
	}
	if (in_suspend) {
		if (!suspend_status) {
			suspend_status = true;
			// record log
			lpm_log_info("dev_id=%u lp not completed resume, not send ipc.\n", dev_id);
		}
		// lp not completed resume, not send ipc
		return false;
	}

	if (suspend_status) {
		suspend_status = false;
		// record log
		lpm_log_info("dev_id=%u lp completed resume, start send ipc.\n", dev_id);
	}
#endif
	return true;
}

STATIC bool lpm_heartbeat_check_is_unsend_ipc(uint32_t dev_id)
{
	if (!lpm_heartbeat_check_resume_done(dev_id)) {
		// lp not completed resume, not send ipc
		return true;
	}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	if (lpm_heartbeat_check_fault_flag(LPM_HB_UNSEND_IPC, dev_id)) {
		// fault inject, not send ipc
		return true;
	}
#endif
	return false;
}

STATIC uint32_t lpm_read_heartbeat_counter_from_sharemem(uint32_t dev_id)
{
	struct lpm_heartbeat_sm_data sm_data = {0};

#if defined(LPM_SHAREMEM_V1) || defined(LPM_SHAREMEM_V2)
	static uint32_t log_cnt = 0; // log suppression, each bit means a dev_id
	int32_t ret;
	ret = lpm_sharemem_get_value(
		dev_id, LPM_SHAREMEM_HEARTBEAT_OFFSET, (uint8_t *)&sm_data, (unsigned long)sizeof(sm_data));
	if (ret != 0) {
		lpm_log_err("dev_id=%u read heartbeat counter from sharemem failed, ret=%d\n", dev_id, ret);
		return 0;
	}
	if (sm_data.magic_num != LPM_SHAREMEM_HEARTBEAT_MAGIC) {
		if ((log_cnt & ((uint32_t)0x1 << dev_id)) == 0) {
			// log suppression
			lpm_log_err("dev_id=%u read heartbeat counter from sharemem error, magic_num=0x%x\n",
				dev_id, sm_data.magic_num);
			log_cnt |= ((uint32_t)0x1 << dev_id);
		}
		return 0;
	}
	log_cnt &= ~((uint32_t)0x1 << dev_id);
#endif
	return sm_data.sm_counter;
}

static uint32_t lpm_heartbeat_get_sm_counter(uint32_t dev_id)
{
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	if (!hb_priv->common_cfg.is_sharemem_check) {
		// no check heartbeat counter read from sharemem
		// sm_counter always set 0
		return 0;
	}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	if (lpm_heartbeat_check_fault_flag(LPM_HB_UNUPDATE_SM, dev_id)) {
		// fault inject, not update counter read from sharemem
		// sm_counter always set 0
		return 0;
	}
#endif
	return lpm_read_heartbeat_counter_from_sharemem(dev_id);
}

STATIC void lpm_heartbeat_increase_counter(uint32_t dev_id, uint8_t *count)
{
	uint8_t ptr;
	unsigned long flags;
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);

	// Update the inc_counter number first
	// to avoid not finding the inc_counter number when receiving ack
	spin_lock_irqsave(&hb_dev_priv->hb_spinlock, flags);
	*count = hb_dev_priv->inc_counter;

	ptr = hb_dev_priv->queue_ptr;
	hb_dev_priv->queue[ptr].ipc_counter = hb_dev_priv->inc_counter;
	hb_dev_priv->queue[ptr].sm_counter  = lpm_heartbeat_get_sm_counter(dev_id);
	hb_dev_priv->queue_ptr = (ptr + (uint8_t)1) % (uint8_t)LPM_HEARTBEAT_QUEUE_MAX;
	hb_dev_priv->inc_counter++;
	spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
}

STATIC void lpm_heartbeat_decrease_counter(uint32_t dev_id, uint8_t count)
{
	unsigned long flags;
	uint8_t last_queue_ptr;
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);

	// Return the inc_counter number when ipc fails to send
	spin_lock_irqsave(&hb_dev_priv->hb_spinlock, flags);
	if (hb_dev_priv->queue_ptr == 0) {
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
		lpm_log_warn("dev_id=%u lpm heartbeat not find count=%u when roll back seq.\n",
			dev_id, count);
		return;
	}

	last_queue_ptr = hb_dev_priv->queue_ptr - (uint8_t)0x1;
	if (hb_dev_priv->queue[last_queue_ptr].ipc_counter == count) {
		lpm_heartbeat_reset_seq(&hb_dev_priv->queue[last_queue_ptr]);
		hb_dev_priv->queue_ptr = last_queue_ptr;
		hb_dev_priv->inc_counter = (hb_dev_priv->inc_counter == 0) ?
			(uint8_t)LPM_HEARTBEAT_MAX_COUNTER : (hb_dev_priv->inc_counter - (uint8_t)0x1);
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
	} else {
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
		lpm_log_warn("dev_id=%u lpm heartbeat count=%u mismatch queue[%u]=ipc[%u] sm[%u] when roll back seq.\n",
			dev_id, count, last_queue_ptr, hb_dev_priv->queue[last_queue_ptr].ipc_counter,
			hb_dev_priv->queue[last_queue_ptr].sm_counter);
	}
}

// return value :
// false: no need restart timer, true: need restart timer
STATIC bool lpm_heartbeat_do_check(uint32_t dev_id)
{
	int32_t ret;
	unsigned long timestamp;
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);
	uint8_t hb_seq_count;

	if (hb_dev_priv->work == 0) {
		return false;
	}

	if (hb_dev_priv->stop == 1) {
		return true;
	}

	if (lpm_heartbeat_check_is_unsend_ipc(dev_id)) {
		return true;
	}

	ret = lpm_heartbeat_judge(dev_id);
	if (lpm_heartbeat_check_is_stop(ret)) {
		lpm_heartbeat_stop(hb_dev_priv);
		hb_dev_priv->work = 0;
		lpm_log_err("lpm heart beat broken, stop sending heart beat, dev_id=%u.\n", dev_id);
		return false;
	}

	hb_dev_priv->send_ipc_status = LPM_HEARTBEAT_IPC_SEND_SUCC;
	// to avoid false detection in the scenario where
	// the inc_counter has not been update when the heartbeat ack has arrived
	lpm_heartbeat_increase_counter(dev_id, &hb_seq_count);
	ret = lpm_heartbeat_send_ipc(dev_id, hb_seq_count);
	if (ret != 0) {
		lpm_heartbeat_decrease_counter(dev_id, hb_seq_count);
		hb_dev_priv->send_ipc_status = LPM_HEARTBEAT_IPC_SEND_FAIL;
		lpm_log_err("lpm heart beat send ipc message failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return true;
	}

	timestamp = (unsigned long)jiffies;
	lpm_heartbeat_check_time_cycle(dev_id, timestamp, hb_dev_priv->last_timestamp);
	hb_dev_priv->last_timestamp = timestamp;

	return true;
}

STATIC void lpm_heartbeat_event_pro(uint32_t dev_id)
{
	bool need_restart = true;
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);

	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("lpm heartbeat event pro failed, dev_id=%u is out of range\n", dev_id);
		return;
	}

	need_restart = lpm_heartbeat_do_check(dev_id);
	if (need_restart) {
		hb_dev_priv->hb_timer.expires =
			jiffies + ((unsigned long)LPM_FAULT_HEART_BEAT_CYCLE * (unsigned long)HZ);
		add_timer_on(&hb_dev_priv->hb_timer, LPM_TIMER_BIND_CPU);
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
STATIC void lpm_heartbeat_event(unsigned long data)
{
	lpm_heartbeat_event_pro((uint32_t)data);
}
#else
STATIC void lpm_heartbeat_event(struct timer_list *t)
{
	struct lpm_heartbeat_cfg *hb_dev_priv = NULL;

	hb_dev_priv = from_timer(hb_dev_priv, t, hb_timer);
	lpm_heartbeat_event_pro(hb_dev_priv->dev_id);
}
#endif

STATIC void lpm_heartbeat_cfg_init(uint32_t dev_id)
{
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);

	hb_dev_priv->hb_en           = true;
	hb_dev_priv->work            = 1;
	hb_dev_priv->stop            = 0;
	hb_dev_priv->broken          = 0;
	hb_dev_priv->inc_counter     = 0;
	hb_dev_priv->queue_ptr       = 0;
	hb_dev_priv->last_timestamp  = 0;
	hb_dev_priv->send_ipc_status = LPM_HEARTBEAT_IPC_NOT_SEND;
	spin_lock_init(&hb_dev_priv->hb_spinlock);
	lpm_heartbeat_clear_queue(hb_dev_priv);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
	setup_timer(&hb_dev_priv->hb_timer, lpm_heartbeat_event, (unsigned long)((uintptr_t)dev_id));
#else
	timer_setup(&hb_dev_priv->hb_timer, lpm_heartbeat_event, 0);
#endif

	hb_dev_priv->hb_timer.expires = jiffies + ((unsigned long)LPM_FAULT_HEART_BEAT_CYCLE * (unsigned long)HZ);
	add_timer_on(&hb_dev_priv->hb_timer, LPM_TIMER_BIND_CPU);
}

STATIC void lpm_heartbeat_cfg_uninit(uint32_t dev_id)
{
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);

	hb_dev_priv->work = 0;
	hb_dev_priv->stop = 1;
	(void)del_timer_sync(&hb_dev_priv->hb_timer);
}

void lpm_hb_task_suspend_prepare(void)
{
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();
	struct lpm_heartbeat_cfg *hb_dev_priv = NULL;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);
		if (hb_dev_priv->hb_en) {
			lpm_heartbeat_stop(hb_dev_priv);
		}
	}

	lpm_log_info("lpm heartbeat suspend success\n");
}

void lpm_hb_task_post_suspend(void)
{
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();
	struct lpm_heartbeat_cfg *hb_dev_priv = NULL;
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();
	unsigned long flags;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);
		if (hb_dev_priv->hb_en) {
			hb_dev_priv->send_ipc_status = LPM_HEARTBEAT_IPC_NOT_SEND;
			// clear queue
			// avoid false detection of heartbeat broken
			// when repeatedly suspend and resume
			spin_lock_irqsave(&hb_dev_priv->hb_spinlock, flags);
			lpm_heartbeat_clear_queue(hb_dev_priv);
			spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
			lpm_heartbeat_start(hb_dev_priv);
		}

		if (hb_priv->common_cfg.is_resume_check) {
			lpm_log_info("lpm heartbeat resume check\n");
			(void)lpm_heartbeat_do_check(dev_id);
		}
	}

	lpm_log_info("lpm heartbeat resume success\n");
}

STATIC uint8_t lpm_heartbeat_find_ipc(struct lpm_heartbeat_cfg *hb_dev_priv, uint32_t hb_ipc_seq)
{
	uint8_t i;
	struct lpm_heartbeat_seq *queue = hb_dev_priv->queue;

	if ((hb_dev_priv->queue_ptr == 0) || (hb_dev_priv->queue_ptr > LPM_HEARTBEAT_QUEUE_MAX)) {
		// mismatch
		return LPM_HEARTBEAT_QUEUE_MAX;
	}
	for (i = 0; i < hb_dev_priv->queue_ptr; i++) {
		if (queue[i].ipc_counter == hb_ipc_seq) {
			return i;
		}
	}
	// mismatch
	return LPM_HEARTBEAT_QUEUE_MAX;
}

STATIC void lpm_heartbeat_del_ipc(struct lpm_heartbeat_cfg *hb_dev_priv, uint8_t hb_ipc_idx)
{
	uint8_t ptr;
	struct lpm_heartbeat_seq *queue = hb_dev_priv->queue;

	// delete the matching ipc counter
	for (ptr = hb_ipc_idx; ptr < (hb_dev_priv->queue_ptr - (uint8_t)0x1); ptr++) {
		queue[ptr].ipc_counter = queue[ptr + (uint8_t)0x1].ipc_counter;
		queue[ptr].sm_counter  = queue[ptr + (uint8_t)0x1].sm_counter;
	}

	lpm_heartbeat_reset_seq(&queue[ptr]);
	hb_dev_priv->queue_ptr = ptr;
}

STATIC void lpm_heartbeat_ipc_handle(uint32_t dev_id, uint32_t hb_ipc_seq)
{
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);
	unsigned long flags;
	uint8_t ptr;
	bool status_change = false;

	spin_lock_irqsave(&hb_dev_priv->hb_spinlock, flags);
	status_change = lpm_heartbeat_set_ipc_block_status(hb_dev_priv, 0x0);
	ptr = lpm_heartbeat_find_ipc(hb_dev_priv, hb_ipc_seq);
	if (ptr < LPM_HEARTBEAT_QUEUE_MAX) {
		lpm_heartbeat_del_ipc(hb_dev_priv, ptr);
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
	} else {
		spin_unlock_irqrestore(&hb_dev_priv->hb_spinlock, flags);
		lpm_log_warn("dev_id=%u cann't find match heart beat seq, hb_ipc_seq=%u, queue_ptr=%u\n",
			dev_id, hb_ipc_seq, hb_dev_priv->queue_ptr);
	}

	// show log if ipc block status changed
	lpm_heartbeat_record_ipc_block_status(dev_id, hb_ipc_seq, status_change, true);
}

STATIC bool lpm_heartbeat_check_ipc(struct lpm_fault_ipc_ack_msg *rx_msg)
{
	if ((rx_msg->cmd == (uint8_t)LPM_FAULT_CMD_INQUIRY) &&
		(rx_msg->sub_cmd == (uint8_t)LPM_FAULT_SUB_CMD_SET_HB)) {
		return true;
	}
	return false;
}

STATIC void lpm_heartbeat_receive_ipc(uint32_t dev_id, uint8_t *ipc_data)
{
	struct lpm_fault_ipc_ack_msg *rx_msg = (struct lpm_fault_ipc_ack_msg *)ipc_data;
	struct lpm_heartbeat_ipc_data *hb_ipc_ack = (struct lpm_heartbeat_ipc_data *)(void *)&rx_msg->data[0];

	if (!lpm_heartbeat_check_ipc(rx_msg)) {
		return;
	}
	if (lpm_heartbeat_check_is_skip_ipc(dev_id)) {
		return;
	}
	lpm_heartbeat_ipc_handle(dev_id, hb_ipc_ack->count);
}

STATIC void lpm_heartbeat_uninit_dev_priv(uint32_t dev_num)
{
	int32_t ret;
	uint32_t dev_id;
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		if (!hb_priv->hb_cfg[dev_id].hb_en) {
			continue;
		}
		lpm_heartbeat_cfg_uninit(dev_id);
		ret = lpm_ipc_unregister_notify_func(dev_id, LPM_IPC_RX_HB);
		if (ret != 0) {
			// try best to unregister
			lpm_log_err("lpm heartbeat unregister notify failed, dev_id=%u\n", dev_id);
		}
	}
}

STATIC int32_t lpm_heartbeat_init_dev_priv(uint32_t dev_num)
{
	int32_t ret;
	uint32_t dev_id;
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		hb_priv->hb_cfg[dev_id].dev_id = dev_id;

		// not support heatbeat detect, no need register ipc receiver
		if (!lpm_heartbeat_check_is_enable(dev_id)) {
			hb_priv->hb_cfg[dev_id].hb_en = false;
			hb_priv->hb_cfg[dev_id].work  = 0;
			hb_priv->hb_cfg[dev_id].stop  = 1;
			lpm_log_info("lpm dev_id=%u close heart beat between lp and driver.\n", dev_id);
			continue;
		}

		ret = lpm_ipc_register_rx_notify_func(dev_id, LPM_IPC_RX_HB, lpm_heartbeat_receive_ipc);
		if (ret != 0) {
			lpm_log_err("lpm heartbeat register notify failed, dev_id=%u\n", dev_id);
			goto register_ipc_failed;
		}
		lpm_heartbeat_cfg_init(dev_id);
	}
	return 0;

register_ipc_failed:
	lpm_heartbeat_uninit_dev_priv(dev_id);
	return -1;
}


int32_t lpm_fault_heartbeat_init(uint32_t dev_num)
{
	int32_t ret;

	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm channel init failed, dev_num=%u is out of range\n",
			dev_num);
		return -1;
	}

	lpm_heartbeat_init_priv_info(dev_num);

	ret = lpm_heartbeat_init_dev_priv(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm heartbeat init dev priv failed, dev_num=%u\n", dev_num);
		return ret;
	}
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	ret = lpm_heartbeat_debugfs_init();
	if (ret != 0) {
		lpm_heartbeat_uninit_dev_priv(dev_num);
		return ret;
	}
#endif
	lpm_log_info("lpm heartbeat probe success, dev_num=%u\n", dev_num);
	return 0;
}

int32_t lpm_fault_heartbeat_exit(uint32_t dev_num)
{
	lpm_heartbeat_uninit_dev_priv(dev_num);

	lpm_log_info("lpm heartbeat remove success, dev_num=%u\n", dev_num);
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
static bool lpm_hb_debug_check_inject_dev(uint32_t dev_id)
{
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	// dev_id == 0xfe: faults into all devices
	// dev_id < 0xfe: inject faults into the specified device
	if (dev_id != LPM_HB_INJECT_ALL_DEV) {
		// inject faults into the specified device
		if (!lpm_common_check_dev_id(dev_id)) {
			lpm_log_err("lpm heartbeat inject fault failed, dev_id=%u is out of range.\n", dev_id);
			return false;
		}

		if (!hb_priv->hb_cfg[dev_id].hb_en) {
			lpm_log_err("lpm heartbeat inject fault failed, dev_id=%u heartbeat not enable\n", dev_id);
			return false;
		}
	}
	return true;
}

static void lpm_hb_debug_inject_fault(enum lpm_heartbeat_debug_type op_type, uint32_t dev_id, bool inject_flag)
{
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();
	const char *fault_desc[] = {"unhandle ipc", "unsend ipc", "unupdate sharemem"};

	lpm_log_info("lpm heartbeat inject %s fault, old:dev_id=%u, inject_flag=%u, new:dev_id=%u, inject_flag=%u\n",
		fault_desc[op_type], hb_priv->hb_debug_cfg.fault_inject[op_type].dev_id,
		hb_priv->hb_debug_cfg.fault_inject[op_type].inject_flag, dev_id, (uint32_t)inject_flag);

	hb_priv->hb_debug_cfg.fault_inject[op_type].dev_id = dev_id;
	hb_priv->hb_debug_cfg.fault_inject[op_type].inject_flag = inject_flag;
}

STATIC void lpm_hb_debug_set_status(uint32_t op_type, uint32_t dev_id, bool fault_inject)
{
	if (!lpm_hb_debug_check_inject_dev(dev_id)) {
		return;
	}

	switch (op_type) {
	case 0x0: // heartbeat broken
		// does not update heartbeat counter read from sharemem
		lpm_hb_debug_inject_fault(LPM_HB_UNHANDLE_IPC, dev_id, fault_inject);
		lpm_hb_debug_inject_fault(LPM_HB_UNUPDATE_SM, dev_id, fault_inject);
		break;
	case 0x1:
		// not send heartbeat ipc
		lpm_hb_debug_inject_fault(LPM_HB_UNSEND_IPC, dev_id, fault_inject);
		break;
	case 0x2: // heartbeat ipc block
		// does not handle heartbeat ipc ack
		lpm_hb_debug_inject_fault(LPM_HB_UNHANDLE_IPC, dev_id, fault_inject);
		if (!fault_inject) {
			// ipc recover means lpmcu recover, sharemem will recover at same
			lpm_hb_debug_inject_fault(LPM_HB_UNUPDATE_SM, dev_id, fault_inject);
		}
		break;
	default:
		lpm_log_err("lpm heartbeat inject fault failed, op_type=%u illegal\n", op_type);
		break;
	}
}

static void lpm_hb_debug_show_last_ipc_ts(uint32_t dev_id, struct seq_file *seq)
{
	struct lpm_heartbeat_cfg *hb_dev_priv = lpm_heartbeat_dev_priv_info(dev_id);
	uint8_t curr_ptr = hb_dev_priv->ipc_data_ptr;
	uint8_t prev_ptr;
	uint8_t pprev_ptr;

	curr_ptr  = (curr_ptr + (uint8_t)LPM_HEARTBEAT_QUEUE_MAX - (uint8_t)0x1) % (uint8_t)LPM_HEARTBEAT_QUEUE_MAX;
	prev_ptr  = (curr_ptr + (uint8_t)LPM_HEARTBEAT_QUEUE_MAX - (uint8_t)0x1) % (uint8_t)LPM_HEARTBEAT_QUEUE_MAX;
	pprev_ptr = (prev_ptr + (uint8_t)LPM_HEARTBEAT_QUEUE_MAX - (uint8_t)0x1) % (uint8_t)LPM_HEARTBEAT_QUEUE_MAX;

	seq_printf(seq, "last 3 heartbeat ipc timestamp: %u(%u-%u-%u), %u(%u-%u-%u), %u(%u-%u-%u).\n",
		hb_dev_priv->ipc_data[pprev_ptr].count, hb_dev_priv->ipc_data[pprev_ptr].time_sec_high,
		hb_dev_priv->ipc_data[pprev_ptr].time_sec_low, hb_dev_priv->ipc_data[pprev_ptr].time_usec_low,
		hb_dev_priv->ipc_data[prev_ptr].count, hb_dev_priv->ipc_data[prev_ptr].time_sec_high,
		hb_dev_priv->ipc_data[prev_ptr].time_sec_low, hb_dev_priv->ipc_data[prev_ptr].time_usec_low,
		hb_dev_priv->ipc_data[curr_ptr].count, hb_dev_priv->ipc_data[curr_ptr].time_sec_high,
		hb_dev_priv->ipc_data[curr_ptr].time_sec_low, hb_dev_priv->ipc_data[curr_ptr].time_usec_low);
}

static void lpm_hb_debug_show_common_cfg(struct seq_file *seq)
{
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();
	seq_printf(seq, "[common config]\n");
	seq_printf(seq, " is_resume_check=%s\n", hb_priv->common_cfg.is_resume_check ? "true" : "false");
	seq_printf(seq, " is_recovery_check=%s\n", hb_priv->common_cfg.is_recovery_check ? "true" : "false");
	seq_printf(seq, " is_sharemem_check=%s\n", hb_priv->common_cfg.is_sharemem_check ? "true" : "false");
}

static void lpm_hb_debug_show_fault_inject_cfg(struct seq_file *seq)
{
	uint32_t i;
	const char *fault_desc[] = {"unhandle ipc", "unsend ipc", "unupdate sharemem"};
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();

	seq_printf(seq, "[debugfs config]\n");
	for (i = 0; i < (uint32_t)LPM_HB_FAULT_TYPE_MAX; i++) {
		seq_printf(seq, " %s: dev_id=%u, inject_flag=%u\n",
			fault_desc[i], hb_priv->hb_debug_cfg.fault_inject[i].dev_id,
			(uint32_t)hb_priv->hb_debug_cfg.fault_inject[i].inject_flag);
	}
}

static void lpm_hb_debug_show_dev_cfg(struct seq_file *seq)
{
	uint32_t dev_id;
	uint32_t sm_counter;
	struct lpm_fault_heartbeat_priv *hb_priv = lpm_heartbeat_priv_info();
	struct lpm_heartbeat_cfg *hb_cfg = NULL;

	seq_printf(seq, "[dev config]\n");
	for (dev_id = 0; dev_id < hb_priv->dev_num; dev_id++) {
		hb_cfg = &hb_priv->hb_cfg[dev_id];
		sm_counter  = lpm_heartbeat_get_sm_counter(dev_id);
		seq_printf(seq, " dev_id=%u, hb_en=%u, stop=%u, work=%u, broken=%u, ipc_block=%u, "
			"inc_counter=%u, sm_counter=%u, queue_ptr=%u, send_ipc_status=%u\n",
			dev_id, hb_cfg->hb_en, hb_cfg->stop, hb_cfg->work, hb_cfg->broken, hb_cfg->ipc_block,
			hb_cfg->inc_counter, sm_counter, hb_cfg->queue_ptr, hb_cfg->send_ipc_status);

		lpm_hb_debug_show_last_ipc_ts(dev_id, seq);
	}
}

STATIC void lpm_hb_debug_get_status(struct seq_file *seq)
{
	lpm_hb_debug_show_common_cfg(seq);
	lpm_hb_debug_show_fault_inject_cfg(seq);
	lpm_hb_debug_show_dev_cfg(seq);
}

STATIC ssize_t lpm_fault_debugfs_hb_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	uint32_t user_cfg[LPM_DEBUGFS_MAX_USER_INPUT_NUM] = {0};

	// param0:fault type:
	//    0: does not handle heartbeat ipc ack
	//    1: not send heartbeat ipc
	// param1:dev_id
	// param2:is inject fault
	ret = lpm_debugfs_get_input(buf, len, user_cfg, 0x3);
	if (ret != 0) {
		lpm_log_err("debugfs get heartbeat input failed, ret=%d\n", ret);
		return -1;
	}

	lpm_hb_debug_set_status(user_cfg[0], user_cfg[1], (user_cfg[2] != 0) ? true : false);

	(void)pos;
	(void)file;
	return (ssize_t)len;
}

STATIC int32_t lpm_fault_debugfs_hb_show(struct seq_file *seq, void *v)
{
	(void)v;
	lpm_hb_debug_get_status(seq);
	return 0;
}

STATIC int32_t lpm_fault_debugfs_hb_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_fault_debugfs_hb_show, inode->i_private);
}
STATIC int32_t lpm_heartbeat_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "heartbeat",
		.fn_open = lpm_fault_debugfs_hb_open,
		.fn_write = lpm_fault_debugfs_hb_write
	};
	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm heartbeat register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}
#endif

#endif
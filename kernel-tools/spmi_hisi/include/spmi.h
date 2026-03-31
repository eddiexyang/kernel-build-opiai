/* Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM spmi

#if !defined(_TRACE_SPMI_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_SPMI_H

#include <linux/hisi-spmi.h>
#include <linux/tracepoint.h>
#include <linux/securec.h>

TRACE_EVENT(spmi_write_begin,
	TP_PROTO(u8 opcode, u8 sid, u16 addr, u8 len, const u8 *buf),
	TP_ARGS(opcode, sid, addr, len, buf),

	TP_STRUCT__entry(
		__field(u8, sid)
		__field(u8, opcode)
		__field(u16, addr)
		__field(u8, len)
		__dynamic_array(u8, buf, len + 1)
	),

	TP_fast_assign(
		__entry->sid    = sid;
		__entry->opcode = opcode;
		__entry->addr   = addr;
		__entry->len    = len + 1;
		memcpy_s(__get_dynamic_array(buf), len + 1, buf, len + 1);
	),

	TP_printk("sid=%02d opc=%d addr=0x%04x len=%d buf=0x[%*phD]",
		  (int)__entry->sid, (int)__entry->opcode, (int)__entry->addr,
		  (int)__entry->len, (int)__entry->len, __get_dynamic_array(buf))
);

TRACE_EVENT(spmi_write_end,
	TP_PROTO(u8 opcode, u8 sid, u16 addr, int ret),
	TP_ARGS(opcode, sid, addr, ret),

	TP_STRUCT__entry(
		__field(u8, sid)
		__field(u8, opcode)
		__field(u16, addr)
		__field(int, ret)
	),

	TP_fast_assign(
		__entry->sid    = sid;
		__entry->opcode = opcode;
		__entry->addr   = addr;
		__entry->ret    = ret;
	),

	TP_printk("sid=%02d opc=%d addr=0x%04x ret=%d",
		  (int)__entry->sid, (int)__entry->opcode, (int)__entry->addr, __entry->ret)
);

TRACE_EVENT(spmi_read_begin,
	TP_PROTO(u8 opcode, u8 sid, u16 addr),
	TP_ARGS(opcode, sid, addr),

	TP_STRUCT__entry(
		__field(u8, sid)
		__field(u8, opcode)
		__field(u16, addr)
	),

	TP_fast_assign(
		__entry->sid    = sid;
		__entry->opcode = opcode;
		__entry->addr   = addr;
	),

	TP_printk("sid=%02d opc=%d addr=0x%04x",
		  (int)__entry->sid, (int)__entry->opcode, (int)__entry->addr)
);

TRACE_EVENT(spmi_read_end,
	TP_PROTO(u8 opcode, u8 sid, u16 addr, int ret, u8 len, const u8 *buf),
	TP_ARGS(opcode, sid, addr, ret, len, buf),

	TP_STRUCT__entry(
		__field(u8, sid)
		__field(u8, opcode)
		__field(u16, addr)
		__field(int, ret)
		__field(u8, len)
		__dynamic_array(u8, buf, len + 1)
	),

	TP_fast_assign(
		__entry->sid    = sid;
		__entry->opcode = opcode;
		__entry->addr   = addr;
		__entry->ret    = ret;
		__entry->len    = len + 1;
		memcpy_s(__get_dynamic_array(buf), len + 1, buf, len + 1);
	),

	TP_printk("sid=%02d opc=%d addr=0x%04x ret=%d len=%02d buf=0x[%*phD]",
		  (int)__entry->sid, (int)__entry->opcode, (int)__entry->addr,
		  __entry->ret, (int)__entry->len, (int)__entry->len,
		  __get_dynamic_array(buf))
);

#endif /* _TRACE_SPMI_H */

/* This part must be outside of protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH /mnt/sda1/yihao/opiai/kernel-build-opiai/kernel-tools/spmi_hisi/include/trace/events
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE spmi
#include <trace/define_trace.h>

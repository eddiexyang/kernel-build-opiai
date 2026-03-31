/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/


#ifndef __DMS_EVENT_BBOX_MAP_H__
#define __DMS_EVENT_BBOX_MAP_H__

#define DMS_DEVICE_HEAT_BEAT_LOST 0x68020002

struct dms_event_bbox_map {
    unsigned int event_id;
    unsigned char assertion;
    unsigned int bbox_code;
};

struct dms_event_bbox_map g_event_bbox_map[] = {
    {
        .event_id = 0x80E01801,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA8028802
    },
    {
        .event_id = 0x80E01801,
        .assertion = DMS_EVENT_TYPE_RESUME,
        .bbox_code = 0x98028802
    },
    {
        .event_id = 0x80E18402,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA8028801
    },
    {
        .event_id = 0x80E18401,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA4028801
    },
#ifndef CFG_SOC_PLATFORM_MINIV2
    {
        .event_id = 0x80fa4e00,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA607FFFFU
    },
#endif
    {
        .event_id = 0x40F84E00,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = DMS_DEVICE_HEAT_BEAT_LOST
    },
    {
        .event_id = 0x8C0A4E00,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA4025081 /* dmp_daemon heartbeat lost */
    },
    {
        .event_id = 0x8C0A4E00,
        .assertion = DMS_EVENT_TYPE_RESUME,
        .bbox_code = 0x94025081 /* dmp_daemon heartbeat recover */
    },
    {
        .event_id = 0x8C204E00,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA60250A1 /* tsd_daemon heartbeat lost */
    },
    {
        .event_id = 0x8C204E00,
        .assertion = DMS_EVENT_TYPE_RESUME,
        .bbox_code = 0x960250A1 /* tsd_daemon heartbeat recover */
    },
    {
        .event_id = 0x8C084E00,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA4025021 /* adda heartbeat lost */
    },
    {
        .event_id = 0x8C084E00,
        .assertion = DMS_EVENT_TYPE_RESUME,
        .bbox_code = 0x94025021 /* adda heartbeat recover */
    },
    {
        .event_id = 0x8C0E4E00,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA4025041 /* slogd heartbeat lost */
    },
    {
        .event_id = 0x8C0E4E00,
        .assertion = DMS_EVENT_TYPE_RESUME,
        .bbox_code = 0x94025041 /* slogd heartbeat recover */
    },
    {
        .event_id = 0x8C104E00,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA40250E1 /* log-daemon heartbeat lost */
    },
    {
        .event_id = 0x8C104E00,
        .assertion = DMS_EVENT_TYPE_RESUME,
        .bbox_code = 0x940250E1 /* log-daemon heartbeat recover */
    },
    {
        .event_id = 0x8C0C4E00,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA4025061 /* sklogd heartbeat lost */
    },
    {
        .event_id = 0x8C0C4E00,
        .assertion = DMS_EVENT_TYPE_RESUME,
        .bbox_code = 0x94025061 /* sklogd heartbeat recover */
    },
    {
        .event_id = 0x8C124E00,
        .assertion = DMS_EVENT_TYPE_OCCUR,
        .bbox_code = 0xA4025101 /* hdcd heartbeat lost */
    },
    {
        .event_id = 0x8C124E00,
        .assertion = DMS_EVENT_TYPE_RESUME,
        .bbox_code = 0x94025101 /* hdcd heartbeat recover */
    }
};

#endif

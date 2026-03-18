/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */
#ifndef __HDC_USER_INTERFACE_H__
#define __HDC_USER_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
* @ingroup driver
* @brief Get the base trusted path sent to the specified node device, get trusted path, used to combine dst_path
* parameters of drvHdcSendFile
* @attention host call is valid, used to obtain the basic trusted path sent to the device side using the drvHdcSendFile
* interface
* @param [in]  signed int user_mode   :	0xFF for HwDmUser(upgrade), 0 for BaseUser;
* @param [in]  int peer_node          :	Node number of the node where the Device is located
* @param [in]  int peer_devid         :	Device's unified ID in the host
* @param [in]  unsigned int path_len  :	base_path space size
* @param [out] char *base_path		  :	Obtained trusted path
* @return   DRV_ERROR_NONE, DRV_ERROR_INVALID_VALUE
*/
drvError_t drvHdcGetTrustedBasePathEx(signed int user_mode, int peer_node, int peer_devid, char *base_path,
    unsigned int path_len);

/**
* @ingroup driver
* @brief Send file to the specified path on the specified device
* @attention null
* @param [in]  signed int user_mode :	0xFF for HwDmUser(upgrade), 0 for BaseUser;
* @param [in]  int peer_node        :	Node number of the node where the Device is located
* @param [in]  int peer_devid       :	Device's unified ID in the host
* @param [in]  const char *file		:	Specify the file name of the sent file
* @param [in]  const char *dst_path	:	Specifies the path to send the file to the receiver. If the path is directory,
* the file name remains unchanged after it is sent to the peer; otherwise, the file name is changed to the part of the
* path after the file is sent to the receiver.
* @param [out] void (*progress_notifier)(struct drvHdcProgInfo *) :	  Specify the user's callback handler function;
* when progress of the file transfer increases by at least one percent,file transfer protocol will call this interface.
* @return   DRV_ERROR_NONE, DRV_ERROR_INVALID_VALUE
*/
drvError_t drvHdcSendFileEx(signed int user_mode, int peer_node, int peer_devid, const char *file,
    const char *dst_path, void (*progress_notifier)(struct drvHdcProgInfo *));
#ifdef __cplusplus
}
#endif

#endif /* __HDC_USER_INTERFACE_H__ */

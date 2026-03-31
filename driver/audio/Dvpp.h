 /*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 * Description: Dvpp Head File.
 */

#ifndef DVPP_DVPP_H
#define DVPP_DVPP_H

#include "ExportMacro.h"
#include "DvppCapability.h"
#include "Jpeg.h"
#include "Png.h"
#include "Vdec.h"
#include "Venc.h"
#include "Vpc.h"

DVPP_EXPORT int32_t CreateDvppApi(IDVPPAPI*& pIDVPPAPI);
DVPP_EXPORT int32_t DvppCtl(IDVPPAPI*& pIDVPPAPI, int32_t CMD, dvppapi_ctl_msg* MSG);
DVPP_EXPORT int32_t DestroyDvppApi(IDVPPAPI*& pIDVPPAPI);
DVPP_EXPORT int32_t DvppGetOutParameter(void* in, void* out, int32_t cmd);
#endif // DVPP_DVPP_H
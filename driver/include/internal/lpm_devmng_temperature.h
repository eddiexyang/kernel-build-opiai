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
*/
#ifndef LPM_DEVMNG_TEMPERATURE_H
#define LPM_DEVMNG_TEMPERATURE_H

#ifdef LPM_TEMPERATURE

int32_t lpm_temperature_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_temperature_remove(uint64_t *param, uint32_t param_num);

int32_t lpm_temperature_query_soc(char *in, uint32_t in_len, char *out, uint32_t out_len);

#else
static inline int32_t lpm_temperature_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_temperature_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}
#endif

#endif

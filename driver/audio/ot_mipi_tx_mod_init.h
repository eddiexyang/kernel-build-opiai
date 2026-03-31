/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: define mipi tx init header file
 * Author: Hisilicon multimedia software group
 * Create: 2020/08/10
 */

#ifndef __OT_MIPI_TX_MOD_INIT_H__
#define __OT_MIPI_TX_MOD_INIT_H__

#define MIPI_BOOTDOT_BLOCK_ID      (23U)
#define MIPI_BOOTDOT_MAGIC_NUM     (1U)

#define MIPI_INIT_FAIL             0xA83C1005U

#define INIT_START                 (0U)
#define INIT_END                   (1U)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

void mipi_tx_set_irq_num(unsigned int irq_num);
void mipi_tx_set_regs(const void *regs);

int mipi_tx_module_init(int smooth);
void mipi_tx_module_exit(void);

int mipi_tx_get_smooth(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef __OT_MIPI_TX_MOD_INIT_H__ */

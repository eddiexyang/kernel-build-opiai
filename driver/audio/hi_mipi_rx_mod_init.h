/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: define mipi rx init header file
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */

#ifndef HI_MIPI_RX_MOD_INIT_H
#define HI_MIPI_RX_MOD_INIT_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

void mipi_rx_set_irq_num(unsigned int irq_num);
void slvs_set_irq_num(unsigned int irq_num);
void mipi_rx_set_regs(const void *regs);
void slvs_ec_set_regs(const void *regs);
int mipi_rx_mod_init(void);
void mipi_rx_mod_exit(void);

int mipi_suspend(void);
int mipi_resume(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef HI_MIPI_RX_MOD_INIT_H */
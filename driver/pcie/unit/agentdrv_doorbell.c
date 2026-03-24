/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include "agentdrv_doorbell.h"
#include "agentdrv_msg_p2p.h"
#include "agentdrv_mdc_p2p.h"
#include "agentdrv_unit.h"
#include "resource_drv.h"

u32 g_agentdrv_doorbell_irq_cnt = 0;
EXPORT_SYMBOL(g_agentdrv_doorbell_irq_cnt);

void (*test_interrupt_func[MAX_AGENTDEV_CNT][AGENTDRV_TEST_USED_INTR_NUM])(int interrupt_num) = {
    { NULL, NULL },
};

void agentdrv_test_doorbell_interrupt(int dev_id, int interrupt_num)
{
    if ((dev_id >= 0) && (dev_id < MAX_AGENTDEV_CNT) && (interrupt_num < AGENTDRV_TEST_USED_INTR_NUM) &&
        (test_interrupt_func[dev_id][interrupt_num] != NULL)) {
        test_interrupt_func[dev_id][interrupt_num](interrupt_num);
    }
}

int agentdrv_reg_test_interrupt_func(int interrupt_num, void (*func)(int interrupt_num))
{
    int dev_id = 0;
    if ((interrupt_num < AGENTDRV_TEST_USED_INTR_NUM) && (interrupt_num >= 0)) {
        test_interrupt_func[dev_id][interrupt_num] = func;
        return 0;
    }
    return -EINVAL;
}
EXPORT_SYMBOL(agentdrv_reg_test_interrupt_func);

void agentdrv_doorbell_interrupt_sq_proc(struct agentdrv_devctrl *agent_dev,
    u32 irq_vector, u32 func_id)
{
    struct agentdrv_msg_dev *msg_dev = NULL;
    struct agentdrv_msg_chan *chan = NULL;
    u32 db_ids[AGENTDRV_QCNT_EACH_IRQ] = {0};
    u32 i, db_id, db_num, db_id_in_func;
    u32 msg_chan_num;
    u32 p2p_db_start, p2p_db_end;

    /* Get SQ queue, then get msg_chan to call proc function */
    devdrv_get_nvme_irq_sq_db(agent_dev->sdi_base[func_id], irq_vector, db_ids, func_id, &db_num);
    devdrv_debug("Get SQ db_num. (db_num=%u)\n", db_num);
    for (i = 0; i < db_num; i++) {
        db_id = db_ids[i];
        agentdrv_res_db_queue_id2func_id(db_id, &db_id_in_func);
        msg_dev = agent_dev->p_agentdrv_msg_dev[func_id];
        agentdrv_get_dma_max_msg_chan(func_id, &msg_chan_num);
        agentdrv_get_p2p_msg_db_range(func_id, &p2p_db_start, &p2p_db_end);
        devdrv_debug("Get IRQ sq information. (idx=%u; queue_id=%u; func_id=%u; db_id_in_func=%u)\n",
                     i, db_id, func_id, db_id_in_func);
        if (db_id_in_func < msg_chan_num) {
            chan = &msg_dev->io_chan[db_id_in_func];
            agentdrv_msg_chan_sq_doorbell_proc(chan);
        } else if ((db_id_in_func >= p2p_db_start) &&
                   (db_id_in_func < p2p_db_end)) {
            agentdrv_p2p_msg_chan_doorbell_proc(&msg_dev->p2p_chan[db_id_in_func - p2p_db_start]);
        } else if ((db_id_in_func >= AGENTDRV_GENERAL_INTR_USED_DB_START) &&
                   (db_id_in_func < AGENTDRV_GENERAL_INTR_USED_DB_END)) {
            agentdrv_mdc_sq_doorbell_interrupt(msg_dev->dev_id, db_id_in_func);
        } else if (db_id_in_func == AGENTDRV_TEST_USED_DB_ID) {
            agentdrv_test_doorbell_interrupt(msg_dev->dev_id, 0);
        }
    }
}

void agentdrv_doorbell_interrupt_cq_proc(struct agentdrv_devctrl *agent_dev,
    u32 irq_vector, u32 func_id)
{
    struct agentdrv_msg_dev *msg_dev = NULL;
    struct agentdrv_msg_chan *chan = NULL;
    u32 db_ids[AGENTDRV_QCNT_EACH_IRQ] = {0};
    u32 i, db_id, db_num, db_id_in_func;
    u32 msg_chan_num;

    /* CQ queue, then get msg_chan to call proc function */
    devdrv_get_nvme_irq_cq_db(agent_dev->sdi_base[func_id], irq_vector, db_ids, func_id, &db_num);
    devdrv_debug("Get CQ db_num. (db_num=%u)\n", db_num);
    for (i = 0; i < db_num; i++) {
        /* get msg_dev from db_id */
        db_id = db_ids[i];
        agentdrv_res_db_queue_id2func_id(db_id, &db_id_in_func);
        msg_dev = agent_dev->p_agentdrv_msg_dev[func_id];
        agentdrv_get_dma_max_msg_chan(func_id, &msg_chan_num);
        devdrv_debug("Get IRQ cq information. (idx=%u; queue_id=%u; func_id=%u; db_id_in_func=%u)\n",
                     i, db_id, func_id, db_id_in_func);
        if (db_id_in_func < msg_chan_num) {
            chan = &msg_dev->io_chan[db_id_in_func];
            agentdrv_msg_chan_cq_doorbell_proc(chan);
        } else if ((db_id_in_func >= AGENTDRV_GENERAL_INTR_USED_DB_START) &&
                   (db_id_in_func < AGENTDRV_GENERAL_INTR_USED_DB_END)) {
            agentdrv_mdc_cq_doorbell_interrupt(msg_dev->dev_id, db_id_in_func);
        } else if (db_id_in_func == AGENTDRV_TEST_USED_DB_ID) {
            agentdrv_test_doorbell_interrupt(msg_dev->dev_id, 1);
        }
    }
}

int agentdrv_get_irq_num_and_func_id(int irq, void *data, u32 *irq_num, u32 *func_id)
{
    struct agentdrv_devctrl *agent_dev = (struct agentdrv_devctrl *)data;
    u32 func_totl = agentdrv_res_get_func_total();
    u32 irq_index = 0;
    u32 func_index = 0;

    if (agentdrv_get_soc_doorbell_capability() == false) {
        irq_index = irq - agent_dev->msi_irq_base[0][0];
        (void)agentdrv_nvme_irq_num2func(irq_index, &func_index);
        *irq_num = irq_index;
        *func_id = func_index;
        return 0;
    }

    for (func_index = 0; func_index < func_totl; func_index++) {
        for (irq_index = 0; irq_index < AGENTDRV_SOC_DB_IRQ_NUM; irq_index++) {
            if ((int)agent_dev->msi_irq_base[func_index][irq_index] == irq) {
                *irq_num = irq_index;
                *func_id = func_index;
                return 0;
            }
        }
    }

    devdrv_debug("Get doorbell irq (%d) fail\n", irq);

    return -1;
}

irqreturn_t agentdrv_doorbell_interrupt(int irq, void *data)
{
    struct agentdrv_devctrl *agent_dev = (struct agentdrv_devctrl *)data;
    u32 irq_num = 0;
    u32 irq_vector = 0;
    u32 func_id = 0;
    int ret;

    /* transfer irq number input to doorbell vector */
    ret = agentdrv_get_irq_num_and_func_id(irq, data, &irq_num, &func_id);
    if (ret != 0) {
        return IRQ_HANDLED;
    }

    ret = devdrv_nvme_agent_irq_num2vector(irq_num, &irq_vector);
    devdrv_debug("Get IRQ doorbell vector. (func_id=%u; irq_num=%u; irq_vector=%u)\n", func_id, irq_num, irq_vector);
    if (ret != 0) {
        return IRQ_HANDLED;
    }
    /* dfx */
    g_agentdrv_doorbell_irq_cnt++;

    /* mask before process */
    devdrv_set_nvme_irq_mask(agent_dev->sdi_base[func_id], func_id, irq_vector);
    agentdrv_doorbell_interrupt_sq_proc(agent_dev, irq_vector, func_id);
    agentdrv_doorbell_interrupt_cq_proc(agent_dev, irq_vector, func_id);
    /* unmask after process */
    devdrv_set_nvme_irq_unmask(agent_dev->sdi_base[func_id], func_id, irq_vector);

    return IRQ_HANDLED;
}

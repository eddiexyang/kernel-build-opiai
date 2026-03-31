/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: tsch_defines.h
 * Create: 2020-01-01
 */
#ifndef TS_TSCH_DEFINES_H
#define TS_TSCH_DEFINES_H

#ifndef DAVINCI_MDC_VOS
#if defined(__COMPILER_HUAWEILITEOS__)
#include <los_typedef.h>
#elif defined(__KERNEL__)
#else
#include <stdint.h>
#endif
#else
#include "ee_platform_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(WIN32) && !defined(__cplusplus)

#define inline __inline

#endif

#ifndef char_t
typedef char char_t;
#endif

/**
 * @ingroup tsch
 * @brief the size of each task command is 64 byte
 */
#define TS_TASK_COMMAND_SIZE (64U)

/**
 * @ingroup tsch
 * @brief the size of each task report msg to host is 4 byte
 */
#define TS_TASK_REPORT_MSG_SIZE (12U)

/**
 * @ingroup tsch
 * @brief the count of task report queue's slot
 */
#define TS_TASK_REPORT_QUEUE_SLOTS_COUNT (1024U)

/**
* @ingroup tsch
* @brief when the number of report queue's msg node is large than the task report queue's threshold,

*        then trigger the interrupt to the host
*/
#define TS_TASK_REPORT_THRESHOLD (512U)

/**
 * @ingroup tsch
 * @brief  the duration of report timer is x milliseconds
 */
#define TS_TASK_REPORT_TIMER_DURATION (10U)

/**
 * @ingroup tsch
 * @brief there are 8 priority levels
 */
#define TS_TASK_CMD_QUEUE_PRIORITIES_LEVEL (8U)

/**
 * @ingroup tsch
 * @brief the total count of those task command queue's slot is 1024
 */
#define TS_TASK_CMD_QUEUE_SLOTS_COUNT (1024U)

/**
 * @ingroup tsch
 * @brief the size of each task command queue
 */
#define TS_SIZE_OF_PER_TASK_CMD_QUEUE (TS_TASK_CMD_QUEUE_SLOTS_COUNT / TS_TASK_CMD_QUEUE_PRIORITIES_LEVEL)

/**
 * @ingroup tsch
 * @brief the number of task commands for one prefatch from SQ queue
 */
#define ONE_SQ_PREFECH_NUM (4U)

/**
 * @ingroup tsch
 * @brief the report number of one CQ,1024 reports
 */
#define CQ_LEN (1024U)

/**
 * @ingroup tsch
 * @brief cq flow control
 */
#define FLOW_CONTROL_HIGH_LIMITS (900U)
#define FLOW_CONTROL_LOW_LIMITS (700U)
#define FLOW_CONTROL_FREE_LIMITS (CQ_LEN - FLOW_CONTROL_HIGH_LIMITS)
/**
 * @ingroup tsch
 * @brief the intial flag of BS
 */
#define BS_INITIALIZED (0x2U)

/**
 * @ingroup tsch
 * @brief max event num of dha_dfx
 */
#define DHA_DFX_EVENT_MAX_NUM (8U)

#define DHA_DFX_EVENT_COMMAND_BIT_LEN (8U)

#define MATA_MATA_EVENT_TYPE_NUM (4U)


/**
 * @ingroup tsch
 * @brief aicore 10 group 16 cycle for bs9sx1a, 8 group 8cycle for pg1
 */
#define CUBE_ACTIVE_CFG_EN_VAL  1U
#define CUBE_DUMMY_START_EN_VAL 1U
#define CUBE_DUMMY_NOP_EN_VAL   1U
#define CUBE_DUMMY_NOP_CYCLE_VAL 1U
#define CUBE_DUMMY_CFG_CYCLE_VAL 1U
#define CUBE_DUMMY_NOP_CYCLE_BS9SX1A_VAL 2U
#define CUBE_DUMMY_CFG_CYCLE_BS9SX1A_VAL 2U
#define AIC_SYS_CNT_GROUP       0xA0U
#define AIC_SYS_CNT_16_CYCLE    0x10U
#define AIC_SYS_CNT_1_CYCLE     0x1U
#define AIC_SYS_CNT_OFFSET      16U

enum tag_ts_prof_type_e {
    TS_PROF_TYPE_TASK_BASE = 0,
    TS_PROF_TYPE_SAMPLE_BASE,
};

enum tag_ts_profil_command_type {
    TS_PROFILE_COMMAND_TYPE_ACK = 0,
    TS_PROFILE_COMMAND_TYPE_PROFILING_ENABLE = 1,
    TS_PROFILE_COMMAND_TYPE_PROFILING_DISABLE = 2,
    TS_PROFILE_COMMAND_TYPE_BUFFERFULL = 3,
    TS_PROFILE_COMMAND_TYPE_PRO_RPTR_UPDATE = 4,
    TS_PROFILE_COMMAND_TYPE_PRO_WPTR_UPDATE = 5,
};

typedef struct tag_stream_proc_info {
    uint16_t last_rcv_type;
    uint16_t last_rcv_id;
    uint16_t last_rcv_phase;
    uint16_t last_run_type;
    uint16_t last_run_id;
    uint16_t last_run_phase;
    uint16_t last_snd_type;
    uint16_t last_snd_id;
    uint16_t stream_recyle_flag;
    uint16_t sq_recyle_flag;
    uint8_t stream_phase;
    uint8_t reserved;
} stream_proc_info_t;

typedef struct tag_ts_ts_cpu_profile_config {
    uint32_t period;
    uint32_t event_num;
    uint32_t event[0];
} ts_ts_cpu_profile_config_t;

typedef struct tag_ts_ai_cpu_profile_config {
    uint32_t type;  // 0-task base, 1-sample base
    uint32_t period;
    uint32_t event_num;
    uint32_t event[0];
} ts_ai_cpu_profile_config_t;

typedef struct tag_ts_ai_core_profile_config {
    uint32_t type;                   // 0-task base, 1-sample base
    uint32_t almost_full_threshold;  // sample base
    uint32_t period;                 // sample base
    uint32_t core_mask;              // sample base
    uint32_t event_num;              // public
    uint32_t event[8];               // 8:max event num
    uint32_t tag;                    // bit0==0-enable immediately, bit0==1-enable delay
} ts_ai_core_profile_config_t;

typedef struct tag_ts_ai_vector_profile_config {
    uint32_t type;                   // 0-task base, 1-sample base
    uint32_t almost_full_threshold;  // sample base
    uint32_t period;                 // sample base
    uint32_t core_mask;              // sample base
    uint32_t event_num;              // public
    uint32_t event[0];               // public
} ts_ai_vector_profile_config_t;

typedef struct tag_ts_ts_fw_profile_config {
    uint32_t period;
    uint32_t ts_task_track;     // 1-enable,2-disable
    uint32_t ts_cpu_usage;      // 1-enable,2-disable
    uint32_t ai_core_status;    // 1-enable,2-disable
    uint32_t ts_timeline;       // 1-enable,2-disable
    uint32_t ai_vector_status;  // 1-enable,2-disable
    uint32_t ts_keypoint;       // 1-enable,2-disable
    uint32_t ts_taskstep;       // 1-enable,2-disable
} ts_ts_fw_profile_config_t;

typedef struct tag_ts_l2_profile_config_t {
    uint32_t event_num;
    uint32_t event[DHA_DFX_EVENT_MAX_NUM];
} ts_l2_profile_config_t;

typedef struct tag_ts_stars_soc_log_config {
    uint32_t acsq_task;         // 1-enable,2-disable
    uint32_t acc_pmu;           // 1-enable,2-disable
    uint32_t cdqm_req;          // 1-enable,2-disable
    uint32_t dvpp_vpc_block;    // 1-enable,2-disable
    uint32_t dvpp_jpegd_block;  // 1-enable,2-disable
    uint32_t dvpp_jpege_block;  // 1-enable,2-disable
    uint32_t ffts_thread_task;  // 1-enable,2-disable
    uint32_t ffts_block;        // 1-enable,2-disable
    uint32_t sdma_dmu;          // 1-enable,2-disable
    uint32_t tag;               // bit0==0-enable immediately, bit0==1-enable delay
} ts_stars_soc_log_config_t;

typedef struct tag_ts_ffts_profile_config {
    uint32_t type;              // bit0-task base, bit1-sample base, bit2 blk task, bit3 sub task
    uint32_t period;            // sample base
    uint32_t core_mask;         // sample base
    uint32_t event_num;         // public
    uint16_t event[DHA_DFX_EVENT_MAX_NUM];    // public
} ts_ffts_profile_config_t;

typedef struct tag_ts_stars_ffts_profile_config {
    uint32_t cfg_mode;                 // 0-none,1-aic,2-aiv,3-aic&aiv
    ts_ffts_profile_config_t aic_cfg;
    ts_ffts_profile_config_t aiv_cfg;
    uint32_t tag;                      // bit0==0-enable immediately, bit0==1-enable delay
} ts_stars_ffts_profile_config_t;

typedef struct  tag_stars_profile_config {
    uint32_t inner_switch;   // 1-enable,2-disable
    uint32_t period;        // ms
}ts_stars_profile_config_t;

typedef struct tag_stars_soc_profile_config {
    ts_stars_profile_config_t acc_pmu;
    ts_stars_profile_config_t on_chip;
    ts_stars_profile_config_t inter_die;
    ts_stars_profile_config_t inter_chip;
    ts_stars_profile_config_t low_power;
    ts_stars_profile_config_t stars_info;
}ts_stars_soc_profile_config_t;

typedef struct tag_ts_stars_biu_perf_config {
    uint32_t cycles;
} ts_stars_biu_perf_config_t;

typedef struct tag_ts_hwts_log_config {
    uint32_t tag;          // bit0==0-enable immediately, bit0==1-enable delay
} ts_hwts_log_config_t;

typedef struct tag_ts_profile_command {
    uint32_t cmd_verify;
    uint32_t channel_id;
    uint32_t cmd_type;
    uint32_t buffer_len;   // length of chl buf
    uint64_t buffer_addr;  // chl buf, for ts <--> drv data exchange
    uint32_t buffer_num;
    uint32_t fid;
    uint64_t com_buf_addr;  // 1980 only
    uint32_t com_buf_len;   // 1980 only
    uint32_t sub_channel_id;
    uint32_t data_len;
    uint32_t pid;
    union {
        uint8_t reserved_payload[72];
        ts_ts_cpu_profile_config_t ts_cpu_profile_cfg;
        ts_ai_cpu_profile_config_t ai_cpu_profile_cfg;
        ts_ai_core_profile_config_t ai_core_profile_cfg;
        ts_ai_vector_profile_config_t ai_vector_profile_cfg;
        ts_ts_fw_profile_config_t ts_fw_profile_cfg;
        ts_l2_profile_config_t ts_l2_profile_cfg;
        ts_stars_soc_log_config_t ts_soc_profile_cfg;
        ts_stars_ffts_profile_config_t ts_ffts_profile_cfg;
        ts_stars_soc_profile_config_t ts_stars_profile_cfg;
        ts_stars_biu_perf_config_t ts_biu_perf_profile_cfg;
        ts_hwts_log_config_t hwts_log_cfg;
    } u;
} ts_profile_command_t;

// define for error message
typedef struct ts_ttlv_msg {
    uint16_t tag;
    uint16_t type;
    uint16_t length;
    uint8_t value[];
} ts_ttlv_msg_t;

// define for error message
typedef struct ts_ttv_msg {
    uint16_t tag;
    uint16_t type;
    uint8_t value[];
} ts_ttv_msg_t;

/**
* @ingroup tsch
* @brief error code and exception code encoding scheme
Loc information 2 bits (31~30) ------ 0x01:device
Code category 2 bits (29~28) ------------- 0x01:error code
Error level 3 bits (27~25) --------------- 0b100:CRITICAL 0b011:MAJOR 0b010:MINOR 0b001:SUGGESTION 0b000:UNKNOW
Module ID 8 bits (24~17) ----------------- 0x03:TS
*/
#define TS_STD_ERROR_CODE_BASIC                          (0x90060000U)
#define TS_STD_ERROR_LVL_CRITICAL                        (0X08000000U)
#define TS_STD_ERROR_LVL_MAJOR                           (0X06000000U)
#define TS_STD_ERROR_LVL_MINOR                           (0X04000000U)
#define TS_STD_ERROR_LVL_SUGGESTION                      (0X02000000U)
#define TS_STD_ERROR_CODE_CRITICAL(errcode)     (TS_STD_ERROR_CODE_BASIC | TS_STD_ERROR_LVL_CRITICAL | (errcode))
#define TS_STD_ERROR_CODE_MAJOR(errcode)        (TS_STD_ERROR_CODE_BASIC | TS_STD_ERROR_LVL_MAJOR | (errcode))
#define TS_STD_ERROR_CODE_MINOR(errcode)        (TS_STD_ERROR_CODE_BASIC | TS_STD_ERROR_LVL_MINOR | (errcode))
#define TS_STD_ERROR_CODE_SUGGESTION(errcode)   (TS_STD_ERROR_CODE_BASIC | TS_STD_ERROR_LVL_SUGGESTION | (errcode))
#define TS_STD_ERROR_CODE_DEFAULT(errcode)      (TS_STD_ERROR_CODE_BASIC | (errcode))

/**
 * @ingroup tsch
 * @brief the reture value of task scheduler module
 */
typedef enum tag_ts_error {
    TS_SUCCESS = 0x0,                                    /**< success */
    TS_ERROR_UNKNOWN = 0x1,                              /**< unknown error */
    TS_ERROR_NULL_PTR = 0x2,                             /**< NULL ptr */
    TS_ERROR_ILLEGAL_AI_CORE_ID = 0x3,                   /**< illegal ai core id */
    TS_ERROR_ILLEGAL_PARAM = 0x4,                        /**< illegal param */
    TS_ERROR_TASK_CMD_QUEUE_FULL = 0x5,                  /**< task cmd queue is full */
    TS_ERROR_TASK_CMD_QUEUE_EMPTY = 0x6,                 /**< task cmd queue is empty */
    TS_ERROR_TASK_REPORT_QUEUE_FULL = 0x7,               /**< task report queue is full */
    TS_ERROR_TASK_REPORT_QUEUE_EMPTY = 0x8,              /**< task report queue is empty */
    TS_ERROR_TASK_NODE_BUFF_ALL_OCCUPYED = 0x9,          /**< all the node of public task buff are occupyed */
    TS_ERROR_TASK_NODE_BUFF_ALL_FREED = 0xA,             /**< all the node of public task buff are free */
    TS_ERROR_L2_MEM_INSUFFICIENT_SPACE = 0xB,            /**< there is not enough space on l2 memory */
    TS_ERROR_L2_MALLOC_FAILED = 0xC,                     /**< malloc l2 failed */
    TS_ERROR_DMA_CHANNEL_ALL_OCCUPYED = 0xD,             /**< all the dma channel are occupyed */
    TS_ERROR_MEMCPY_OP_FAILED = 0xE,                     /**< memcpy failed */
    TS_ERROR_BS_SLOT_ALL_OCCUPYED = 0xF,                 /**< there is not free bs slot for new task */
    TS_ERROR_TBS_SLOT_REPEAT_FREE = 0x10,                /**< one free bs slot cannot be freed again */
    TS_ERROR_PRIORITY_TASK_LIST_FULL = 0x11,             /**< the priority list is full */
    TS_ERROR_PRIORITY_TASK_LIST_EMPTY = 0x12,            /**< the priority list is empty */
    TS_ERROR_NO_STREAM_LIST_NEED_TO_BE_PROCESSED = 0x13, /**< there is no stream list need service */
    TS_ERROR_REPEAT_MARK_STREAM_NEED_SERVICE = 0x14,     /**< repeat mark the stream list need service */
    TS_ERROR_SYS_DMA_CHANNEL_ALL_OCCUPAPYED = 0x15,      /**< system dma channel all occupapyed */
    TS_ERROR_NO_HBML2TASKNODE_FOUND = 0x16,              /**< no hbm l2 task node found */
    TS_ERROR_SQNODE_NODE_SLOT_ALL_OCCUPAPYED = 0x17, /**< all the node of the current sq are occupyed */
    TS_ERROR_CQNODE_NODE_SLOT_ALL_OCCUPAPYED = 0x18, /**< all the node of the current sq are occupyed */
    TS_ERROR_SQNODE_NOT_ENOUGH = 0x19,               /**< the sq node is not enough for data transfer */
    TS_ERROR_SQNODE_SLOT_REPEAT_FREE = 0x1A,         /**< sq node slot repeat free */
    TS_ERROR_CQNODE_SLOT_REPEAT_FREE = 0x1B,         /**< cq node slot repeat free */
    TS_ERROR_CQ_REPORT_FAILED = 0x1C,                /**< cq report failed */
    TS_SYS_DMA_RESET_SUCCESS = 0x1D,                 /**< sys dma reset success */
    TS_SYS_DMA_RESET_FAILED = 0x1E,                  /**< sys dma reset failed */
    TS_SYS_DMA_TRNSFER_FAILED = 0x1F,                /**< sys dma transfer failed */
    TS_SYS_DMA_MEMADDRALIGN_FAILED = 0x20,           /**< sys dma mem address align failed */
    TS_SYS_DMA_ERROR_QUEUE_FULL = 0x21,              /**< sys dma queue full */
    TS_SYS_DMA_ERROR_QUEUE_EMPTY = 0x22,             /**< sys dma queue empty */
    TS_ERROR_TIMER_EVENT_FULL = 0x23,                /**< timer event full */
    TS_TASK_L2_DESC_ENTRY_NOT_ENOUGH = 0x24,         /**< l2 description enter not enough */
    TS_ERROR_AICORE_TIMEOUT = 0x25,                  /**< aicore timeout */
    TS_ERROR_AICORE_EXCEPTION = 0x26,                /**< aicore exception */
    TS_ERROR_AICORE_TRAP_EXCEPTION = 0x27,           /**< aicore trap exception */
    TS_ERROR_AICPU_TIMEOUT = 0x28,                   /**< aicpu timeout */
    TS_ERROR_SDMA_L2_TO_DDR_MALLOC_FAIL = 0x29,      /**< sdma move l2 to ddr malloc failed */
    TS_ERROR_AICPU_EXCEPTION = 0x2A,                 /**< aicpu exception */
    TS_ERROR_AICPU_DATADUMP_RSP_ERR = 0x2B,          /**< aicpu datadump response error */
    TS_ERROR_AICPU_MODEL_RSP_ERR = 0x2C,             /**< aicpu model operate response error */
    TS_ERROR_REPEAT_ACTIVE_MODEL_STREAM = 0x2D,      /**< repeat active model stream */
    TS_ERROR_REPEAT_NOTIFY_WAIT = 0x2E,              /**< repeat notify wait */
    TS_ERROR_AICORE_OVERFLOW = 0x2F,                 /**< aicore over flow error */
    TS_ERROR_VECTOR_CORE_TIMEOUT = 0x30,             /**< vector core timeout */
    TS_ERROR_VECTOR_CORE_EXCEPTION = 0x31,           /**< vector core exception */
    TS_ERROR_VECTOR_CORE_TRAP_EXCEPTION = 0x32,      /**< vector core trap exception */
    TS_ERROR_AIVEC_OVERFLOW = 0x33,                  /**< aivec over flow error */
    TS_ERROR_AICPU_OVERFLOW = 0x34,                  /**< aicpu over flow error */
    TS_ERROR_SDMA_OVERFLOW = 0x35,                   /**< sdma over flow error */
    TS_ERROR_NORMAL_SQCQ_EXISTED = 0x36,             /**< sq or cq id is existed*/
    TS_ERROR_SQ_CQ_INVALID = 0x37,                   /**< sq or cq invalid */
    TS_ERROR_SQ_RANGE_INVALID = 0x38,                /**< sq range invalid */
    TS_ERROR_IRQ_INVALID = 0x39,                     /**< irq id invalid */
    TS_ERROR_LOGIC_CQID_INVALID = 0x40,              /**< logic cqid invalid */
    TS_ERROR_APPFLAG_INVALID = 0x41,                 /**< app flag invalid */
    TS_ERROR_PHY_CQID_INVALID = 0x42,                /**< physical cq id invalid */
    TS_ERROR_VFID_NOT_ZERO = 0x43,                   /**< vfid must be 0(physical vm) */
    TS_ERROR_VFID_NOT_RANGE = 0x44,                  /**< vfid not range(virtual vm) */
    TS_ERROR_STREAMID_INVALID = 0x45,                /**< stream id invalid */
    TS_ERROR_VFGID_INVALID = 0x46,                   /**< vfgroup id invalid */
    TS_ERROR_AICORENUM_INVALID = 0x47,               /**< aicore num invalid(virtual vm) */
    TS_ERROR_VM_IS_WORKING = 0x48,                   /**< vm is working(virtual vm) */
    TS_ERROR_VM_IS_IDLE = 0x49,                      /**< vm is idle(virtual vm) */
    TS_ERROR_VM_RESOURCE_CONFLICT = 0x50,            /**< vm resource confict(virtual vm) */
    TS_ERROR_OPER_TYPE_INVALID = 0x51,               /**< operation type invalid(must be 1 or 0) */
    TS_ERROR_RESOURCE_TYPE_NOT_SUPPORT = 0x52,       /**< resource_type not support */
    TS_ERROR_VIR_ID_INVALID = 0x53,                  /**< vir id invalid */
    TS_ERROR_PHY_ID_INVALID = 0x54,                  /**< phy id invalid */
    TS_ERROR_SQ_CTRL_FLAG_INVALID = 0x55,            /**< sq ctrl flag invalid */
    TS_ERROR_APP_COUNT_INVALID = 0x56,               /**< app count invalid */

    TS_ERROR_DEBUG_INVALID_SQCQ = 0x70,              /**< debug invalid sqcq */
    TS_ERROR_DEBUG_WRONG_COMMAND_TYPE = 0x71,        /**< debug wrong command type */
    TS_ERROR_DEBUG_CMD_PROCESS = 0x72,               /**< dbg cmd process */
    TS_ERROR_DEBUG_INVALID_DEVICE_STATUS = 0x73,     /**< debug invalid device status */
    TS_ERROR_DEBUG_NOT_IN_DEBUG_STATUS = 0x74,       /**< debug not in debug status */
    TS_ERROR_DEBUG_INVALID_TASK_STATUS = 0x75,       /**< debug invalid task status */
    TS_ERROR_DEBUG_TASK_EMPTY = 0x76,                /**< debug task empty */
    TS_ERROR_DEBUG_TASK_FULL = 0x77,                 /**< debug task full */
    TS_ERROR_DEBUG_TASK_NOT_EXIST = 0x78,            /**< debug task not exist */
    TS_ERROR_DEBUG_AI_CORE_FULL = 0x79,              /**< debug ai core full */
    TS_ERROR_DEBUG_AI_CORE_NOT_EXIST = 0x7A,         /**< debug ai core not exist */
    TS_ERROR_DEBUG_AI_CORE_EXCEPTION = 0x7B,         /**< debug ai core exception */
    TS_ERROR_DEBUG_AI_CORE_TIMEOUT = 0x7C,           /**< debug ai core timeout */
    TS_ERROR_DEBUG_BREAKPOINT_FULL = 0x7D,           /**< debug breakpoint full */
    TS_ERROR_DEBUG_READ_ERROR = 0x7E,                /**< debug read error */
    TS_ERROR_DEBUG_WRITE_FAIL = 0x7F,                /**< debug write fail */
    TS_QUEUE_FULL = 0x80,                            /**< ts queue full */
    TS_QUEUE_EMPTY = 0x81,                           /**< ts queue empty */
    TS_QUEUE_ALLOC_MEM_FAIL = 0x82,                  /**< ts queue alloc mem fail */
    TS_QUEUE_DATA_SIZE_UNMATCH = 0x83,               /**< ts queue data size unmatch */
    TS_PCIE_DMA_INVLD_CPY_TYPE = 0x84,               /**< pcie dma invalid copy type */
    TS_INVLD_CPY_DIR = 0x85,                         /**< invalid copy dir */
    TS_PCIE_DMA_INVLD_CQ_DES = 0x86,                 /**< pcie dma invalid cq des */
    TS_PCIE_DMA_CPY_ERR = 0x87,                      /**< pcie dma copy error */
    TS_PCIE_DMA_LNK_CHN_BUSY = 0x88,                 /**< pcie dima link channal busy */
    TS_ERROR_PROFILE_BUFF_FULL = 0x89,               /**< profile buff full */
    TS_ERROR_PROFILE_MODE_CONFLICT = 0x8A,           /**< profile mode conflict */
    TS_ERROR_PROFILE_OTHER_PID_ON = 0x8B,            /**< profile other pid on */
    TS_ERROR_SCHD_AIC_TASK_PRELOAD_FAILED = 0x8C,    /**< sched aic task preload failed */
    TS_ERROR_TSCPU_CLOSE_FAILED = 0x8D,              /**< tscpu close failed */
    TS_ERROR_EXPECT_FAIL = 0x8E,                     /**< expect failed */
    TS_ERROR_REPEAT_MODEL_STREAM = 0x8F,             /**< repeat model stream */
    TS_ERROR_STREAM_MODEL_UNBIND = 0x90,             /**< the model stream unbind failed */
    TS_MODEL_STREAM_EXE_FAILED = 0x91,               /**< the model stream execute failed */
    TS_ERROR_IPC_SEND_FAILED = 0x92,                 /**< ipc send failed */
    TS_ERROR_IPC_PROC_REG_FAILED = 0x93,             /**< ipc process register failed */
    TS_STREAM_SQ_FULL = 0x94,                        /**< stream sq full */
    TS_ERROR_END_OF_SEQUENCE = 0x95,                 /**< end of sequence */
    TS_ERROR_SWITCH_STREAM_LABEL = 0x96,             /**< switch stream label failed */
    TS_ERROR_TASK_TYPE_NOT_SUPPORT = 0x97,           /**< task type not support */
    TS_ERROR_TRANS_SQE_FAIL = 0x98,                  /**< trans sqe fail */
    TS_MODEL_ABORT_NORMAL = 0x99,                    /**< model exit */
    TS_ERROR_RINGBUFFER_NOT_FIND = 0x100,            /**< not find the ringbuffer */
    TS_ERROR_RINGBUFFER_MANAGE_IS_NOT_FREE = 0x101,  /**< not find the idle ringbuffer manage */
    TS_ERROR_RINGBUFFER_IS_FULL = 0x102,             /**< the current ringbuffer is full */
    TS_ERROR_TASK_EXCEPTION = 0x103,                 /**< task exception */
    TS_ERROR_TASK_TRAP = 0x104,                      /**< task trap exception */
    TS_ERROR_TASK_TIMEOUT = 0x105,                   /**< task timeout */
    TS_ERROR_TASK_SQE_ERROR = 0x106,                 /**< task sqe error */
    TS_ERROR_VF_SLOT_ALLOC_FAIL = 0x107,             /**< vf slot alloc fail */
    TS_PCIE_DMA_LIMIT_CONTROL = 0x108,               /**< vf mode pcie copy limit control */
    TS_ERROR_DEBUG_REGISTER_CONFLICT = 0x109,        /**< debug register conflict */
    TS_ERROR_AIC_TRAP_RD_OVERFLOW = 0x10A,           /**< aicore trap read overflow */
    TS_ERROR_AIC_TRAP_WR_OVERFLOW = 0x10B,           /**< aicore trap write overflow */
    TS_ERROR_AIV_TRAP_RD_OVERFLOW = 0x10C,           /**< aivec trap read overflow */
    TS_ERROR_AIV_TRAP_WR_OVERFLOW = 0x10D,           /**< aivec trap write overflow */
    TS_ERROR_MODEL_ABORTED = 0x10E,                  /**< model abort */
    TS_ERROR_FFTSPLUS_TASK_EXCEPTION = 0x10F,        /* ftsplus exception */
    TS_ERROR_FFTSPLUS_TASK_TRAP = 0x110,             /* fftsplus trap exception */
    TS_ERROR_FFTSPLUS_TASK_TIMEOUT = 0x111,          /* fftsplus timeout */
    // the following error codes are define for error msg
    TS_ERR_MSG_EXCEED_MAX_SIZE = 0x200,
    TS_ERROR_MALLOC_MEM_FAILED = 0X201,              /**< malloc memory fail, system error */
    TS_ERROR_DELETE_NODE_FAILED = 0X202,             /* delete node from list error */
    TS_ERROR_ONLINE_PROF_ADDR_CANNOT_BE_ZERO = 0X203, /* online profiling addr cannot be zero */
    TS_ERROR_GET_HWTS_TASK_LOG_ERROR = 0X204,        /* not get hwts task log */
    TS_ERROR_KERNEL_DATA_FULL = 0X205,               /* ts kernel data full */
    TS_ERROR_SHARED_BUFF_INVALID = 0x206,            /* shared buffer invalid */
    TS_ERROR_READ_REGISTER_FAILED = 0x207,           /* read register failed */
    TS_ERROR_TS_STATUS_POWER_OFF = 0x208,            /* ts status power off */
    TS_ERROR_PRIVATE_BUFFER_FULL = 0x209,            /* ts private buffer full */
    TS_ERROR_TRANS_ADDR_FAILED = 0x20A,              /* ts trans addr failed */
    TS_ERROR_SDMA_ERROR = 0x20B,
    TS_ERROR_SDMA_TIMEOUT = 0x20C,                   /* hwts sdma timeout */
    TS_ERROR_AICORE_RESET_FAILED = 0x20D,            /* aicore reset failed */
    TS_ERROR_AIV_RESET_FAILED = 0x20E,               /* aiv reset failed */
    TS_ERROR_HWTS_SW_STATUS_ERROR = 0x20F,            /* hwts sw status failed */
    TS_ERROR_HWTS_EXCEPTION = 0x210,                  /* hwts exception */
    TS_ERROR_MEMSET_FAILED = 0x211,                   /* memset failed */
    TS_ERROR_AICPU_BS_DONE_PRE_CHECK_FAILED = 0x212,  /* aicpu bs done pre check failed */
    TS_ERROR_GOING_DOWN = 0x213,
    TS_ERROR_HWTS_POOL_CONFLICT = 0x214,              /* hwts pool conflict */
    TS_ERROR_GET_SQ_ID_FAILED = 0x215,                /* get stream sq id failed */
    TS_ERROR_MODEL_END_GRAPH_EXCEPTION = 0x216,       /* end graph exception */
    TS_ERROR_SDMA_COPY_ERROR = 0x217,
    TS_ERROR_SDMA_REDUCE_ERROR = 0x218,
    TS_ERROR_ALLOC_STREAM_ID_FAILED = 0x219,          /* alloc stream id failed */
    // the following error codes are ts inner codes, no need return to runtime
    TS_EXEC_AGAIN = 0x900,                           /**< task exec again */
    TS_PROFILE_CMD_COMPLETED = 0x901,                /**< profiling config completed */
    TS_PROFILE_CMD_FAILED = 0x902,                   /**< profiling config failed */
    TS_EXEC_NEW_SCHE = 0x903,                        /**< task need use new slot sche */
    TS_APP_EXIT_UNFINISHED = 0x904,                  /**< app exit is still processing */
    TS_ERROR_STREAM_NOT_IN_MODEL = 0x905,            /* target stream is not in model */
    TS_ERROR_MODEL_EXEC_REC_ACT_STREAM = 0x906,      /* model is running, but active stream receive */
    TS_APP_EXIT_NO_SEND_CQ = 0x907,                  /**< app exit no send cq report */
    TS_STREAM_RECYCLED = 0x908,                      /**< stream is recycled */
    TS_ERROR_RESERVED,                               /**< unknow error */
} ts_error_t;

// stars model exe result: 4bits, valid value is 0~15
typedef enum tag_ts_stars_model_exe_result {
    TS_STARS_MODEL_EXE_SUCCESS = 0,
    TS_STARS_MODEL_STREAM_EXE_FAILED = 1,
    TS_STARS_MODEL_END_OF_SEQ = 2,
    TS_STARS_MODEL_EXE_ABORT = 3,
    TS_STARS_MODEL_AICPU_TIMEOUT = 4,
    TS_STARS_MODEL_EXE_RES5 = 5,
    TS_STARS_MODEL_EXE_RES6 = 6,
    TS_STARS_MODEL_EXE_RES7 = 7,
    TS_STARS_MODEL_EXE_RES8 = 8,
    TS_STARS_MODEL_EXE_RES9 = 9,
    TS_STARS_MODEL_EXE_RES10 = 10,
    TS_STARS_MODEL_EXE_RES11 = 11,
    TS_STARS_MODEL_EXE_RES12 = 12,
    TS_STARS_MODEL_EXE_RES13 = 13,
    TS_STARS_MODEL_EXE_RES14 = 14,
    TS_STARS_MODEL_EXE_RES15 = 15
} ts_stars_model_exe_result;

typedef enum tag_debug_exception {
    /* BIU */
    DFX_ERR = 0x00000000ULL,
    L2_WRITE_OOB,
    L2_READ_OOB,
    /* CCU */
    DIV0_CCU = 0x10000000ULL,
    NEG_SQRT_CCU,
    ILLEGAL_INSTR,
    CALL_DEPTH_OVRFLW,
    LOOP_ERR,
    LOOP_CNT_ERR,
    DATA_EXCP_CCU,
    SBUF_ECC = 0x10000009ULL,
    CCU_INF_NAN,
    UB_ECC_CCU,
    /* IFU */
    BUS_ERR = 0X20000000ULL,
    /* MTE */
    UB_ECC_MTE = 0X30000000ULL,
    DATA_EXCP_MTE,
    DECOMP,
    COMP,
    UNZIP,
    CIDX_OVERFLOW,
    ILLEGAL_STRIDE,
    ILLEGAL_FM_SIZE,
    ILLEGAL_L1_3D_SIZE,
    BAS_RADDR_OBOUND,
    FMAP_LESS_KERNEL,
    FPOS_LARGER_FSIZE,
    F1WPOS_LARGER_FSIZE,
    FMAPWH_LARGER_L1SIZE,
    PANDDING_CFG,
    WRITE_3D_OVERFLOW,
    READ_OVERFLOW, /* 0X30000010ULL */
    WRITE_OVERFLOW,
    GDMA_READ_OVERFLOW,
    GDMA_WRITE_OVERFLOW,
    GDMA_ILLEGAL_BURST_NUM,
    GDMA_ILLEGAL_BURST_LEN,
    L1_ECC,
    TLU_ECC,
    ROB_ECC,
    BIU_RDWR_RESP,
    AIPP_ILLEGAL_PARAM,
    KTABLE_RD_ADDR_OVERFLOW,
    TIMEOUT = 0x3000001EULL,
    MTE_ILLEGAL_SCHN_CFG,
    ILLEGAL_SCHN_CFG,
    ATM_ADD,
    KTABLE_WR_ADDR_OVERFLOW,
    /* VEC */
    UB_ECC_VEC = 0X40000000ULL,
    DATA_EXCP_VEC,
    ILLEGAL_MASK,
    SAME_BLK_ADDR,
    NEG_SQRT_VEC,
    NEG_LN,
    INF_NAN,
    DIV0_VEC,
    L0C_ECC_VEC,
    UB_SELF_RDWR_CFLT,
    COL2IMG_ILLEGAL_K_SIZE,
    COL2IMG_ILLEGAL_FETCH_POS,
    COL2IMG_ILLEGAL_1ST_WIN_POS,
    COL2IMG_ILLEGAL_STRIDE,
    COL2IMG_ILLEGAL_FM_SIZE,
    COL2IMG_RD_DFM_ADDR_OVFLOW,
    COL2IMG_RD_FM_ADDR_OVFLOW, /* 0X40000010ULL */
    UB_WRAP_AROUND,
    /* CUBE */
    L0A_ECC = 0X50000001ULL,
    L0B_ECC,
    L0C_ECC_CUBE,
    L0C_SELF_RDWR_CFLT,
    INVLD_INPUT,
    L0A_WRAP_AROUND,
    L0B_WRAP_AROUND,
    L0C_WRAP_AROUND,
    /* CFLT */
    L0A_RDWR_CFLT = 0X60000000ULL,
    L0B_RDWR_CFLT,
    L0C_RDWR_CFLT,
    UB_WR_CFLT,
    UB_RD_CFLT,
    /* unknown */
    UNKNOWN_EXCEPTION = 0xF0000000ULL
} ts_dbg_except_t;
/**
 * @ingroup tsch
 * @brief the bool value defination
 */
#define TS_FALSE ((ts_bool_t)0U)
#define TS_TRUE ((ts_bool_t)1U)

#define TS_DISABLE ((ts_bool_t)0U)
#define TS_ENABLE  ((ts_bool_t)1U)

#define TS_EFFECTIVE_LATER ((ts_bool_t)0U)
#define TS_EFFECTIVE_IMMEDIATELY ((ts_bool_t)1U)

#define TS_HUGE_STREAM_FLAG (0x4U)
enum tag_switch_condition {
    TS_EQUAL = 0,
    TS_NOT_EQUAL,
    TS_GREATER,
    TS_GREATER_OR_EQUAL,
    TS_LESS,
    TS_LESS_OR_EQUAL
};

enum tag_switch_datatype {
    TS_SWITCH_INT32 = 0,
    TS_SWITCH_INT64 = 1
};

// defined for error message
// stream status
enum tag_ts_stream_state {
    STREAM_STATE_INIT = 0,
    STREAM_STATE_CREATE = 1,
    STREAM_STATE_BIND_MODEL = 2,
    STREAM_STATE_ADD_TASK = 3,
    STREAM_STATE_ACTIVE = 4,
    STREAM_STATE_AICPU_ACTIVE = 5,
    STREAM_STATE_SCHEDULE = 6,
    STREAM_STATE_DEACTIVE = 7,  // ignore 1910, reset hwts sq
    STREAM_STATE_UNBIND_MODEL = 8,
    STREAM_STATE_DESTROY = 9,   // get maintaince task, and clean data
    STREAM_STATE_RECYCLE = 10,    // send driver for recycling
    STREAM_STATE_RESERVED = 0XFF,
};

typedef enum tag_ts_task_phase {
    TASK_PHASE_INIT = 0,
    TASK_PHASE_WAIT_EXEC = 1, // task is wait to exec
    TASK_PHASE_RUN = 2,
    TASK_PHASE_COMPLETE = 3,
    TASK_PHASE_PENDING = 4,   // task process stream wait event
    TASK_PHASE_AICPU_TASK_WAIT = 5,  // aicpu second phase
    TASK_PHASE_TASK_PROCESS_MEMCPY = 6,
    TASK_PHASE_AICORE_DONE = 7,
    TASK_PHASE_AIV_DONE = 8,
    TASK_PHASE_AICPU_TIMEOUT_PROC = 9,
    TASK_PHASE_RESERVED = 0XFF,
} ts_task_phase_t;

enum ts_error_type {
    TS_ERROR_TYPE_SYSTEM,
    TS_ERROR_TYPE_TASK,
    TS_ERROR_TYPE_STREAM,
    TS_ERROR_TYPE_MODEL,
    TS_ERROR_TYPE_PID,
    // can not modify
    TS_ERROR_TYPE_RESERVED = 0xff
};

// define for error msg encode&decode
enum ts_err_msg_data_type {
    TS_ERR_MSG_UINT8 = 0,
    TS_ERR_MSG_INT8 = 1,
    TS_ERR_MSG_UINT16 = 2,
    TS_ERR_MSG_INT16 = 3,
    TS_ERR_MSG_UINT32 = 4,
    TS_ERR_MSG_INT32 = 5,
    TS_ERR_MSG_UINT64 = 6,
    TS_ERR_MSG_INT64 = 7,
    TS_ERR_MSG_BASIC_TYPE_MAX_ID = 0x7E,
    TS_ERR_MSG_STRUCT = 0x7F,
    TS_ERR_MSG_UINT8_ARRAY = 0x80, // set high bit means array
    TS_ERR_MSG_CHAR_ARRAY = 0x81,
    TS_ERR_MSG_UINT16_ARRAY = 0x82,
    TS_ERR_MSG_INT16_ARRAY = 0x83,
    TS_ERR_MSG_UINT32_ARRAY = 0x84,
    TS_ERR_MSG_INT32_ARRAY = 0x85,
    TS_ERR_MSG_UINT64_ARRAY = 0x86,
    TS_ERR_MSG_INT64_ARRAY = 0x87,
    // can not modify
    TS_ERR_MSG_DATA_TYPE_RESERVED,
};

enum ts_err_msg_code_tag {
    // error message basic information
    TAG_TS_ERR_MSG_FUNC_CODE = 0,
    TAG_TS_ERR_MSG_LINE_CODE = 1,
    TAG_TS_ERR_MSG_TS_ERR_CODE = 2,
    // define for ts_err_code_msg_t
    TAG_TS_ERR_MSG_CODE_STRUCT = 3, // ts_err_code_t
    TAG_TS_ERR_MSG_TIMESTAMP_SEC = 4,
    TAG_TS_ERR_MSG_TIMESTAMP_USEC = 5,
    TAG_TS_ERR_MSG_CURRENT_TIME = 6,
    TAG_TS_ERR_MSG_STRING = 7,
    TAG_TS_ERR_MSG_ERROR_CODE = 8,
    TAG_TS_ERR_MSG_MAX_ID = 0x100,

    // other extension information
    TAG_TS_ERR_MSG_TASK_ID = 0x101,
    TAG_TS_ERR_MSG_TASK_TYPE = 0x102,
    TAG_TS_ERR_MSG_TASK_PHASE = 0x103,
    TAG_TS_ERR_MSG_LAST_RECEIVE_TASK_ID = 0x104,
    TAG_TS_ERR_MSG_LAST_SEND_TASK_ID = 0x105,
    TAG_TS_ERR_MSG_STREAM_ID = 0x106,
    TAG_TS_ERR_MSG_STREAM_PHASE = 0x107,
    TAG_TS_ERR_MSG_MODEL_ID = 0x108,
    TAG_TS_ERR_MSG_PID = 0x109,

    // if tag > TAG_TS_ERR_MSG_FIRST_SENTENCE_ID, means add a new sentence
    TAG_TS_ERR_MSG_FIRST_SENTENCE_ID = 0xF000,
    // define for task_track_msg_t
    TAG_TS_ERR_MSG_TASK_TRACK_MSG = 0xF001, // highest bit means sentence
    // define for stream_control_msg_t
    TAG_TS_ERR_MSG_STREAM_DESC_MSG = 0xF002,
    // define for model_desc_msg_t
    TAG_TS_ERR_MSG_MODEL_DESC_MSG = 0xF003,
    // define for pid_desc_msg_t
    TAG_TS_ERR_MSG_PID_DESC_MSG = 0xF004,
    // define for ts_sys_err_desc_msg_t
    TAG_TS_ERR_MSG_SYS_MSG = 0xF005,
    TAG_TS_ERR_MSG_UNKNOWN_SENTENCE = 0xF006,
    // can not modify
    TAG_TS_ERR_MSG_CODE_RESERVED = 0xFFFF,
};

typedef uint8_t ts_bool_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TS_TSCH_DEFINES_H */

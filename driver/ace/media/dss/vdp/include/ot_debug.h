#ifndef OT_DEBUG_H
#define OT_DEBUG_H

#include "../../../../dvpp/dvpp/HiDvpp/cbb/include/hi_debug.h"

#ifdef hi_chk_log_level
#undef hi_chk_log_level
#endif
#define hi_chk_log_level(level, mod) HI_TRUE

#ifndef HI_DBG_EMERG
#define HI_DBG_EMERG HI_DVPP_DBG_EMERG
#endif

#ifndef HI_DBG_ALERT
#define HI_DBG_ALERT HI_DVPP_DBG_ALERT
#endif

#ifndef HI_DBG_CRIT
#define HI_DBG_CRIT HI_DVPP_DBG_CRIT
#endif

#ifndef HI_DBG_ERR
#define HI_DBG_ERR HI_DVPP_DBG_ERR
#endif

#ifndef HI_DBG_WARN
#define HI_DBG_WARN HI_DVPP_DBG_WARN
#endif

#ifndef HI_DBG_NOTICE
#define HI_DBG_NOTICE HI_DVPP_DBG_NOTICE
#endif

#ifndef HI_DBG_INFO
#define HI_DBG_INFO HI_DVPP_DBG_INFO
#endif

#ifndef HI_DBG_DEBUG
#define HI_DBG_DEBUG HI_DVPP_DBG_DEBUG
#endif

#ifndef OT_TRACE
#define OT_TRACE(level, mod, fmt, ...) HI_PRINTK_##level(mod, fmt, ##__VA_ARGS__)
#endif

#endif

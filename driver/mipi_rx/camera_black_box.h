#ifndef CAMERA_BLACK_BOX_H
#define CAMERA_BLACK_BOX_H

/*
 * The original SDK ships a camera_black_box.h wrapper for MIPI Rx bootdot
 * tracing. In this tree the only required pieces are already provided by
 * hi_mipi_rx.h and ascend_kernel_hal.h / linux/bootdot.h, so keep this as a
 * compatibility shim to preserve the include structure used by the driver.
 */

#ifndef MEIDA_BLOCK_BOX_BLOCK_ID
#define MEIDA_BLOCK_BOX_BLOCK_ID 0U
#endif

#ifndef CAMERA_MODID_MIPI_START_FAIL
#define CAMERA_MODID_MIPI_START_FAIL 0U
#endif

#endif

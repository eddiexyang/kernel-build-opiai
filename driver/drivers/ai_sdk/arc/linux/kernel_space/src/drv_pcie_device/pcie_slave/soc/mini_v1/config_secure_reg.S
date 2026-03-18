#include <linux/linkage.h>
#include <linux/version.h>

	/* void smc_jump_to_atf(struct tf_smc_param *param); */
	.globl	smc_jump_to_atf
#define SMC_PARAM_X0_OFFS	0
#define SMC_PARAM_X2_OFFS	16
#define SMC_PARAM_X4_OFFS	32
#define SMC_PARAM_X6_OFFS	48

        /* void smc_jump_to_atf(struct tf_smc_param *param); */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
SYM_FUNC_START(smc_jump_to_atf)
#else
ENTRY(smc_jump_to_atf)
#endif
	stp	x28, x30, [sp, #-16]!
	mov	x28, x0
	ldp	x0, x1, [x28, #SMC_PARAM_X0_OFFS]
	ldp	x2, x3, [x28, #SMC_PARAM_X2_OFFS]
	ldp	x4, x5, [x28, #SMC_PARAM_X4_OFFS]
	ldp	x6, x7, [x28, #SMC_PARAM_X6_OFFS]
	smc	#0
	stp	x0, x1, [x28, #SMC_PARAM_X0_OFFS]
	stp	x2, x3, [x28, #SMC_PARAM_X2_OFFS]
	ldp	x28, x30, [sp], #16
	ret
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
SYM_FUNC_END(smc_jump_to_atf)
#else
ENDPROC(smc_jump_to_atf)
#endif
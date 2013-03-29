/*
 * asm/io.h compatibility header
 */

#ifndef __COMPAT_ASM_IO_H_
#define __COMPAT_ASM_IO_H_

#include <linux/config.h>

#include_next <asm/io.h>

#ifndef mmiowb			/* Defined in 2.6.10 */

#if defined(CONFIG_MIPS)

/* Depends on MIPS II instruction set */
#define mmiowb() asm volatile ("sync" ::: "memory")

#elif defined(CONFIG_IA64)

/* IA64 */
#ifdef CONFIG_IA64_SGI_SN2
#include <asm/sn/io.h>
#define platform_mmiowb sn_mmiob
#endif

#ifndef platform_mmiowb
#define platform_mmiowb ia64_mfa
#endif

#define mmiowb() platform_mmiowb()

#else

/* Other architectures */
#define mmiowb()

#endif

#endif /* mmiowb */

#endif

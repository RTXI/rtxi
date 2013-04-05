/*
 * asm/types.h compatibility header
 */

#ifndef __COMPAT_ASM_TYPES_H_
#define __COMPAT_ASM_TYPES_H_

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,18)
typedef unsigned long dma_addr_t;
#endif

#include_next <asm/types.h>

#endif // __COMPAT_ASM_TYPES_H_

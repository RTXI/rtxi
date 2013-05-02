/*
 * asm/dma.h compatibility header
 */

#ifndef __COMPAT_ASM_DMA_H_
#define __COMPAT_ASM_DMA_H_

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,0)
#include <asm/system.h>
static __inline__ unsigned long claim_dma_lock(void)
{
	unsigned long flags;
	save_flags(flags);
	cli();
	return flags;
}

#define release_dma_lock(x)	restore_flags(x)
#endif

#include_next <asm/dma.h>

#endif

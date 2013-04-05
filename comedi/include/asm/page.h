/*
 * asm/page.h compatibility header
 */

#ifndef __COMPAT_PAGE_TYPES_H_
#define __COMPAT_PAGE_TYPES_H_

#include <linux/version.h>
#include_next <asm/page.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
#define virt_to_page(addr) MAP_NR(addr)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,1)
/* Pure 2^n version of get_order */
static __inline__ int get_order(unsigned long size)
{
	int order;

	size = (size - 1) >> (PAGE_SHIFT - 1);
	order = -1;
	do {
		size >>= 1;
		order++;
	} while (size);
	return order;
}
#endif

#endif // __COMPAT_ASM_PAGE_H_

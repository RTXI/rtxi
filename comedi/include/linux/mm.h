/*
 * linux/mm.h compatibility header
 */
/*
    Copyright (C) 2004-2006 Frank Mori Hess <fmhess@users.sourceforge.net>
    Copyright (C) 2006 Ian Abbott

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _COMPAT_MM_H
#define _COMPAT_MM_H

#include <linux/version.h>

#include_next <linux/mm.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,10) \
			  && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,3)
#include <asm/pgalloc.h>
#include <asm/tlb.h>		/* look for tlb_vma() macro for "statm" patch */
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,3) && !defined(tlb_vma)
#define REMAP_PAGE_RANGE(a,b,c,d,e) remap_page_range(b,c,d,e)
#else
#define REMAP_PAGE_RANGE(a,b,c,d,e) remap_page_range(a,b,c,d,e)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
static inline int remap_pfn_range(struct vm_area_struct *vma,
	unsigned long from, unsigned long pfn, unsigned long size,
	pgprot_t prot)
{
	return REMAP_PAGE_RANGE(vma, from, pfn << PAGE_SHIFT, size, prot);
};

#endif

#endif /* _COMPAT_MM_H */

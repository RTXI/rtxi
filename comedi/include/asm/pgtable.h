/*
    asm/pgtable.h compatibility header

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __COMPAT_ASM_PGTABLE_H_
#define __COMPAT_ASM_PGTABLE_H_

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
#define pud_t pgd_t
#define pud_offset(pgd, start)	(pgd)
#endif

#include_next <asm/pgtable.h>

#if !defined(pte_offset_kernel) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
#define pte_offset_kernel(dir,address) pte_offset(dir,address)
#endif

#endif /* __COMPAT_ASM_PGTABLE_H_ */

/*    

Copyright (C) 2005 Frank Mori Hess <fmhess@users.sourceforge.net>
2.4 compat code is based on kernel's lib/kref.c which was:
Copyright (C) 2004 Greg Kroah-Hartman <greg@kroah.com>
Copyright (C) 2004 IBM Corp.
Copyright (C) 2002-2003 Patrick Mochel <mochel@osdl.org>
   
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

#ifndef _KREF_COMPAT_H_
#define _KREF_COMPAT_H_

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)

#include <linux/types.h>
#include <asm/atomic.h>

struct kref {
	atomic_t refcount;
};

static inline void kref_init(struct kref *kref)
{
	atomic_set(&kref->refcount, 1);
}

static inline void kref_get(struct kref *kref)
{
	atomic_inc(&kref->refcount);
}

static inline int kref_put(struct kref *kref,
	void (*release) (struct kref * kref))
{
	if (atomic_dec_and_test(&kref->refcount)) {
		release(kref);
		return 1;
	}
	return 0;
}

#else

#include_next <linux/kref.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
#include <asm/bug.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)

/* Dummy release function should never be called. */
static void comedi_dummy_kref_release(struct kref *kref)
{
	BUG();
}

/* Redefine kref_init to remove 'release' parameter. */
static inline void comedi_internal_kref_init(struct kref *kref)
{
	kref_init(kref, comedi_dummy_kref_release);
}
#undef kref_init
#define kref_init(kref) comedi_internal_kref_init(kref)

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12)

/* Redefine kref_put to add 'release' parameter and return a result. */
static inline int comedi_internal_kref_put(struct kref *kref,
	void (*release) (struct kref * kref))
{
	if (atomic_dec_and_test(&kref->refcount)) {
		release(kref);
		return 1;
	}
	return 0;
}
#undef kref_put
#define kref_put(kref, release) comedi_internal_kref_put(kref, release)

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12) */

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5) */

#endif /* _KREF_COMPAT_H_ */

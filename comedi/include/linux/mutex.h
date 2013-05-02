/*
 * linux/mutex.h compatibility header
 */
/*
    Copyright (C) 2007 Ian Abbott <abbotti@mev.co.uk>

    Based on "FUSE: Filesystem in Userspace",
    Copyright (C) 2001-2007 Miklos Szeredi

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

#ifndef __COMPAT_LINUX_MUTEX_H_
#define __COMPAT_LINUX_MUTEX_H_

#include <linux/config.h>

#ifdef CONFIG_COMEDI_HAVE_MUTEX_H

#include_next <linux/mutex.h>

#ifndef CONFIG_DEBUG_MUTEXES
#ifndef mutex_destroy
/* Some Redhat kernels include a backported mutex.h, lacking mutex_destroy */
#define mutex_destroy(m) do; while (0)
#endif
#endif

#else /* HAVE_MUTEX_H */

#include <asm/semaphore.h>

#define DEFINE_MUTEX(m) DECLARE_MUTEX(m)
#define mutex_init(m) init_MUTEX(m)
#define mutex_destroy(m) do; while (0)
#define mutex_lock(m) down(m)
#define mutex_lock_interruptible(m) down_interruptible(m)
#define mutex_trylock(m) (!down_trylock(m))
#define mutex_unlock(m) up(m)
/* There is some unfortunate name-space pollution in the following macro, so any
 * code using 'mutex' as an identifier has to be careful with include order. */
#define mutex semaphore		/* "struct mutex" becomes "struct semaphore" */

#endif /* HAVE_MUTEX_H */

#endif /* __COMPAT_LINUX_MUTEX_H_ */

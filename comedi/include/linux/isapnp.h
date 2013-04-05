/*
    Kernel compatibility header file

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

#ifndef _ISAPNP_COMPAT_H
#define _ISAPNP_COMPAT_H

#include_next <linux/isapnp.h>

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#include <linux/pnp.h>
#define pnp_find_dev(...) NULL
#define pnp_find_dev(...) NULL
#endif

#endif // _ISAPNP_COMPAT_H

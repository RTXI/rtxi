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

#ifndef _MOD_DEVICETABLE_COMPAT_H
#define _MOD_DEVICETABLE_COMPAT_H

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#define PNP_ID_LEN 8

typedef unsigned long kernel_ulong_t;

struct pnp_device_id {
	__u8 id[PNP_ID_LEN];
	kernel_ulong_t driver_data;
};

#else
#include_next <linux/mod_devicetable.h>
#endif

#endif // _MOD_DEVICETABLE_COMPAT_H

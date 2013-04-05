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

#ifndef _KERNEL_COMPAT_H
#define _KERNEL_COMPAT_H

#include_next <linux/kernel.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#define container_of(ptr, type, member) ({ \
	const typeof( ((type *)0)->member ) *__mptr = (ptr); \
	(type *)( (char *)__mptr - offsetof(type,member) );})

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25)
/* Add missing strict_strtox functions. */
#include <linux/string.h>
#include <linux/errno.h>

static inline int comedi_strict_strtoul(const char *cp, unsigned int base,
	unsigned long *res)
{
	char *tail;
	unsigned long val;
	size_t len;

	*res = 0;
	len = strlen(cp);
	if (len == 0)
		return -EINVAL;

	val = simple_strtoul(cp, &tail, base);
	if (tail == cp)
		return -EINVAL;
	if ((*tail == '\0') ||
		((len == (size_t)(tail - cp) + 1) && (*tail == '\n'))) {
		*res = val;
		return 0;
	}

	return -EINVAL;
}

#undef strict_strtoul
#define strict_strtoul(cp, base, res) comedi_strict_strtoul(cp, base, res)

static inline int comedi_strict_strtol(const char *cp, unsigned int base,
	long *res)
{
	int ret;
	if (*cp == '-') {
		ret = comedi_strict_strtoul(cp + 1, base, (unsigned long *)res);
		if (!ret)
			*res = -(*res);
	} else {
		ret = comedi_strict_strtoul(cp, base, (unsigned long *)res);
	}

	return ret;
}

#undef strict_strtol
#define strict_strtol(cp, base, res) comedi_strict_strtoul(cp, base, res)

static inline int comedi_strict_strtoull(const char *cp, unsigned int base,
	unsigned long long *res)
{
	char *tail;
	unsigned long long val;
	size_t len;

	*res = 0;
	len = strlen(cp);
	if (len == 0)
		return -EINVAL;

	val = simple_strtoull(cp, &tail, base);
	if (tail == cp)
		return -EINVAL;
	if ((*tail == '\0') ||
		((len == (size_t)(tail - cp) + 1) && (*tail == '\n'))) {
		*res = val;
		return 0;
	}

	return -EINVAL;
}

#undef strict_strtoull
#define strict_strtoull(cp, base, res) comedi_strict_strtoul(cp, base, res)

static inline int comedi_strict_strtoll(const char *cp, unsigned int base,
	long long *res)
{
	int ret;
	if (*cp == '-') {
		ret = comedi_strict_strtoull(cp + 1, base, (unsigned long long *)res);
		if (!ret)
			*res = -(*res);
	} else {
		ret = comedi_strict_strtoull(cp, base, (unsigned long long *)res);
	}

	return ret;
}

#undef strict_strtoll
#define strict_strtoll(cp, base, res) comedi_strict_strtoul(cp, base, res)

#endif

#endif // _KERNEL_COMPAT_H

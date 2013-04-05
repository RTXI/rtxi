/***************************************************************************
                                  asm/div64.h
                             -------------------

    copyright            : (C) 2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DIV64_COMPAT_H
#define _DIV64_COMPAT_H

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 0)

static inline u64 my_ull_div(u64 numerator, unsigned long denominator)
{
	u32 value;
	u64 remainder, temp, answer = 0;
	unsigned int shift;
	static const u32 max_u32 = 0xffffffff;

	remainder = numerator;

	while (remainder >= denominator) {
		shift = 0;
		numerator = remainder;
		// shift most significant bits into 32 bit variable
		while (numerator > max_u32) {
			numerator >>= 1;
			shift++;
		}
		if (numerator < denominator) {
			shift--;
			value = 1;
		} else {
			value = numerator;
			value /= denominator;
		}
		temp = ((u64) value) << shift;
		answer += temp;
		remainder -= temp * denominator;
	}

	return answer;
}

#define do_div(n, base) ((n) = my_ull_div(n, base))

#else

#include_next <asm/div64.h>

#endif

#endif // _DIV64_COMPAT_H


#ifndef __COMPAT_ASM_UACCESS_H
#define __COMPAT_ASM_UACCESS_H

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,0)
/* unknown, approx 2.1.4 */

#include <asm/segment.h>

static inline int copy_to_user(void *to, const void *from,
	unsigned long n_bytes)
{
	int i;

	if ((i = verify_area(VERIFY_WRITE, to, n_bytes)) != 0)
		return i;
	memcpy_tofs(to, from, n_bytes);
	return 0;
}

static inline int copy_from_user(void *to, const void *from,
	unsigned long n_bytes)
{
	int i;
	if ((i = verify_area(VERIFY_READ, from, n_bytes)) != 0)
		return i;
	memcpy_fromfs(to, from, n_bytes);
	return 0;
}

static inline int clear_user(void *mem, unsigned long len)
{
	char *cmem = mem;

	if (verify_area(VERIFY_WRITE, mem, len))
		return len;
	/* this is slow, but I'm lazy */
	while (len--) {
		put_user(0, cmem);
		cmem++;
	}
	return 0;
}

#else

#include_next <asm/uaccess.h>

#endif

#endif

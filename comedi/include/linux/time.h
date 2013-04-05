
#ifndef __COMPAT_LINUX_TIME_H
#define __COMPAT_LINUX_TIME_H

#include <linux/version.h>

#include_next <linux/time.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,29) || ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7) )

static inline unsigned int jiffies_to_msecs(const unsigned long j)
{
	/* This works for jiffies values up to ((ULONG_MAX + 1 - HZ) / 1000) */
	return (j * 1000 + HZ - 1) / HZ;
}

static inline unsigned long msecs_to_jiffies(const unsigned int m)
{
	/* This works for msecs values up to ((ULONG_MAX - 999) / HZ) */
	return (m * HZ + 999) / 1000;
};

#endif

#endif

/*
 * asm/semaphore.h compatibility header
 */

#ifndef __COMPAT_ASM_SEMAPHORE_H_
#define __COMPAT_ASM_SEMAPHORE_H_

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,18)
#define init_MUTEX(x)				*(x)=MUTEX
#define init_MUTEX_LOCKED(x)			*(x)=MUTEX_LOCKED
#define DECLARE_MUTEX(name)			struct semaphore name=MUTEX
#define DECLARE_MUTEX_LOCKED(name)		struct semaphore name=MUTEX_LOCKED
/* N.B. Cannot define missing sema_init() portably, so best not use it! */
#endif

#include_next <asm/semaphore.h>

#endif // __COMPAT_ASM_SEMAPHORE_H_

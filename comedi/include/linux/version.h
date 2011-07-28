#ifndef __MULTI_VERSION_H_
#define __MULTI_VERSION_H_

#include <config.h>
#include_next <linux/version.h>

#ifdef UTS_RELEASE_OVERRIDE
#ifdef UTS_RELEASE
#undef UTS_RELEASE
#endif
#define UTS_RELEASE UTS_RELEASE_OVERRIDE
#endif

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

#endif

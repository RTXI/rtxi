/*
 * module.h compatibility header
 */

#ifndef _COMPAT_MODULE_H
#define _COMPAT_MODULE_H

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,10)
#define MODULE_LICENSE(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,11)
#define EXPORT_SYMBOL_GPL(x) EXPORT_SYMBOL(x)
#endif

#include_next <linux/module.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,48)
static inline int try_module_get(struct module *module)
{
	if (!module)
		return 1;
	__MOD_INC_USE_COUNT(module);
	return 1;
}
static inline void module_put(struct module *module)
{
	if (!module)
		return;
	__MOD_DEC_USE_COUNT(module);
}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,69)
static inline void __module_get(struct module *module)
{
	(void)try_module_get(module);
}
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,59)
#define MOD_IN_USE (0)
#endif

#ifndef module_param
#include <linux/moduleparam.h>
#endif

#endif /* _COMPAT_MODULE_H */

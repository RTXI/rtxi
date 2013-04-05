/*
 * pcmcia/cistpl.h compatibility header
 */

#ifndef __COMPAT_PCMCIA_CISTPL_H_
#define __COMPAT_PCMCIA_CISTPL_H_

#include_next <pcmcia/cistpl.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)

/* Redefine pcmcia_parse_tuple() macro to have only 2 params, like 2.6.28. */
#undef pcmcia_parse_tuple
#define pcmcia_parse_tuple(tuple, parse) pccard_parse_tuple(tuple, parse)

#endif

#endif // __COMPAT_PCMCIA_CISTPL_H_

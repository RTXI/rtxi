/*
    Kernel compatibility header file

	portions of this file were copied from the Linux kernel source file
	include/linux/pnp.h  (copyright by Adam Belay <ambx1@neo.rr.com>).

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

#ifndef _PNP_COMPAT_H
#define _PNP_COMPAT_H

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#include <linux/mod_devicetable.h>
#include <linux/device.h>

struct pnp_protocol;
struct pnp_card;
struct pnp_irq;
struct pnp_dma;
struct pnp_port;
struct pnp_mem;
struct pnp_resource_table;
struct pnp_id;
struct pnp_card_link;
struct pnp_card_driver;
struct pnp_dev {
	struct pnp_card *card;
};
struct pnp_driver {
	char *name;
	const struct pnp_device_id *id_table;
	unsigned int flags;
	int (*probe) (struct pnp_dev * dev,
		const struct pnp_device_id * dev_id);
	void (*remove) (struct pnp_dev * dev);
	struct device_driver driver;
};

/* device management */
static inline int pnp_register_protocol(struct pnp_protocol *protocol)
{
	return -ENODEV;
}
static inline void pnp_unregister_protocol(struct pnp_protocol *protocol)
{
}
static inline int pnp_init_device(struct pnp_dev *dev)
{
	return -ENODEV;
}
static inline int pnp_add_device(struct pnp_dev *dev)
{
	return -ENODEV;
}
static inline int pnp_device_attach(struct pnp_dev *pnp_dev)
{
	return -ENODEV;
}
static inline void pnp_device_detach(struct pnp_dev *pnp_dev)
{;
}

/* multidevice card support */
static inline int pnp_add_card(struct pnp_card *card)
{
	return -ENODEV;
}
static inline void pnp_remove_card(struct pnp_card *card)
{;
}
static inline int pnp_add_card_device(struct pnp_card *card,
	struct pnp_dev *dev)
{
	return -ENODEV;
}
static inline void pnp_remove_card_device(struct pnp_dev *dev)
{;
}
static inline int pnp_add_card_id(struct pnp_id *id, struct pnp_card *card)
{
	return -ENODEV;
}
static inline struct pnp_dev *pnp_request_card_device(struct pnp_card_link
	*clink, const char *id, struct pnp_dev *from)
{
	return NULL;
}
static inline void pnp_release_card_device(struct pnp_dev *dev)
{;
}
static inline int pnp_register_card_driver(struct pnp_card_driver *drv)
{
	return -ENODEV;
}
static inline void pnp_unregister_card_driver(struct pnp_card_driver *drv)
{;
}

/* resource management */
static inline struct pnp_option *pnp_register_independent_option(struct pnp_dev
	*dev)
{
	return NULL;
}
static inline struct pnp_option *pnp_register_dependent_option(struct pnp_dev
	*dev, int priority)
{
	return NULL;
}
static inline int pnp_register_irq_resource(struct pnp_option *option,
	struct pnp_irq *data)
{
	return -ENODEV;
}
static inline int pnp_register_dma_resource(struct pnp_option *option,
	struct pnp_dma *data)
{
	return -ENODEV;
}
static inline int pnp_register_port_resource(struct pnp_option *option,
	struct pnp_port *data)
{
	return -ENODEV;
}
static inline int pnp_register_mem_resource(struct pnp_option *option,
	struct pnp_mem *data)
{
	return -ENODEV;
}
static inline void pnp_init_resource_table(struct pnp_resource_table *table)
{
}
static inline int pnp_manual_config_dev(struct pnp_dev *dev,
	struct pnp_resource_table *res, int mode)
{
	return -ENODEV;
}
static inline int pnp_auto_config_dev(struct pnp_dev *dev)
{
	return -ENODEV;
}
static inline int pnp_validate_config(struct pnp_dev *dev)
{
	return -ENODEV;
}
static inline int pnp_activate_dev(struct pnp_dev *dev)
{
	return -ENODEV;
}
static inline int pnp_disable_dev(struct pnp_dev *dev)
{
	return -ENODEV;
}
static inline void pnp_resource_change(struct resource *resource,
	unsigned long start, unsigned long size)
{
}

/* protocol helpers */
static inline int pnp_is_active(struct pnp_dev *dev)
{
	return 0;
}
static inline int compare_pnp_id(struct pnp_id *pos, const char *id)
{
	return -ENODEV;
}
static inline int pnp_add_id(struct pnp_id *id, struct pnp_dev *dev)
{
	return -ENODEV;
}
static inline int pnp_register_driver(struct pnp_driver *drv)
{
	return -ENODEV;
}
static inline void pnp_unregister_driver(struct pnp_driver *drv)
{;
}

#define pnp_port_valid(dev, bar) (0)
#define pnp_irq_valid(dev, bar) (0)
#define pnp_irq(dev, bar) (0)
#define pnp_port_start(dev,bar) (0)

#else
#include_next <linux/pnp.h>
#endif

#endif // _PNP_COMPAT_H

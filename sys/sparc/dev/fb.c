/*
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)fb.c	7.1 (Berkeley) 07/13/92
 *
 * from: $Header: fb.c,v 1.6 92/06/17 05:35:46 torek Exp $
 */

/*
 * /dev/fb (indirect frame buffer driver).  This is gross; we should
 * just build cdevsw[] dynamically.
 */

#include "sys/param.h"
#include "sys/conf.h"
#include "sys/device.h"
#include "sys/proc.h"
#include "sys/fbio.h"

#include "machine/fbvar.h"

static struct fbdevice *devfb;

void
fb_unblank()
{

	if (devfb)
		(*devfb->fb_driver->fbd_unblank)(devfb->fb_device);
}

void
fb_attach(fb)
	struct fbdevice *fb;
{

if (devfb) panic("multiple /dev/fb declarers");
	devfb = fb;
}

int
fbopen(dev, flags, mode, p)
	dev_t dev;
	int flags, mode;
	struct proc *p;
{

	if (devfb == NULL)
		return (ENXIO);
	return (cdevsw[devfb->fb_major].d_open(dev, flags, mode, p));
}

int
fbclose(dev, flags, mode, p)
	dev_t dev;
	int flags, mode;
	struct proc *p;
{

	return (cdevsw[devfb->fb_major].d_close(dev, flags, mode, p));
}

int
fbioctl(dev, cmd, data, flags, p)
	dev_t dev;
	int cmd;
	caddr_t data;
	int flags;
	struct proc *p;
{

	return (cdevsw[devfb->fb_major].d_ioctl(dev, cmd, data, flags, p));
}

int
fbmap(dev, off, prot)
	dev_t dev;
	int off, prot;
{
	int (*map)() = cdevsw[devfb->fb_major].d_mmap;

	if (map == NULL)
		return (-1);
	return (map(dev, off, prot));
}

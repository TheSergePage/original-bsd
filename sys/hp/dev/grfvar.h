/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 *
 * from: Utah $Hdr: grfvar.h 1.9 91/01/21$
 *
 *	@(#)grfvar.h	7.3 (Berkeley) 05/07/91
 */

/* internal structure of lock page */
#define GRFMAXLCK	256
struct	grf_lockpage {
	u_char	gl_locks[GRFMAXLCK];
};
#define gl_lockslot gl_locks[0]

/* per display info */
struct	grf_softc {
	int	g_flags;		/* software flags */
	int	g_type;			/* type of display */
	caddr_t	g_regkva;		/* KVA of registers */
	caddr_t	g_fbkva;		/* KVA of framebuffer */
	struct	grfinfo g_display;	/* hardware description (for ioctl) */
	struct	grf_lockpage *g_lock;	/* lock page associated with device */
	struct	proc *g_lockp;		/* process holding lock */
	short	*g_pid;			/* array of pids with device open */
	int	g_lockpslot;		/* g_pid entry of g_lockp */
	caddr_t	g_data;			/* device dependent data */
};

/* flags */
#define	GF_ALIVE	0x01
#define GF_OPEN		0x02
#define GF_EXCLUDE	0x04
#define GF_WANTED	0x08
#define GF_BSDOPEN	0x10
#define GF_HPUXOPEN	0x20

/* display types - indices into grfdev */
#define	GT_TOPCAT	0
#define	GT_GATORBOX	1
#define	GT_RENAISSANCE	2
#define GT_LRCATSEYE	3
#define GT_HRCCATSEYE	4
#define GT_HRMCATSEYE	5
#define GT_DAVINCI	6

struct	grfdev {
	int	gd_hardid;	/* secondary id returned by hardware */
	int	gd_softid;	/* id returned by HP-UX */
	int	(*gd_init)();	/* boot time initialization */
	int	(*gd_mode)();	/* misc functions */
	char	*gd_desc;	/* text description */
};

/* hardware ids */
#define GID_GATORBOX	1
#define	GID_TOPCAT	2
#define GID_RENAISSANCE	4
#define GID_LRCATSEYE	5
#define GID_HRCCATSEYE	6
#define GID_HRMCATSEYE	7
#define GID_DAVINCI	8

/* software ids defined in grfioctl.h */

/* requests to mode routine */
#define GM_GRFON	1
#define GM_GRFOFF	2
#define GM_GRFOVON	3
#define GM_GRFOVOFF	4

struct	grfreg {
	char	gr_pad0;
	u_char	gr_id;		/* +0x01 */
	char	gr_pad1[0x13];
	u_char	gr_id2;		/* +0x15 */
	char	gr_pad2[0x47];
	u_char	gr_fbomsb;	/* +0x5d */
	char	gr_pad3;
	u_char	gr_fbolsb;	/* +0x5f */
};
/* bitmapped display hardware id */
#define GRFHWID		0x39

/* internal bitmapped display address */
#define GRFIADDR	0x560000

/* minor device interpretation */
#define GRFOVDEV	0x10	/* overlay planes */
#define GRFIMDEV	0x20	/* images planes */
#define GRFUNIT(d)	((d) & 0x7)

#ifdef KERNEL
extern	struct grf_softc grf_softc[];
#endif

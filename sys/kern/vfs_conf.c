/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)vfs_conf.c	7.15 (Berkeley) 07/12/92
 */

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/vnode.h>

#ifdef FFS
#include <ufs/ffs/ffs_extern.h>

/*
 * This specifies the filesystem used to mount the root.
 * This specification should be done by /etc/config.
 */
int (*mountroot)() = ffs_mountroot;
#endif

/*
 * These define the root filesystem and device.
 */
struct mount *rootfs;
struct vnode *rootdir;

/*
 * Set up the filesystem operations for vnodes.
 * The types are defined in mount.h.
 */
#ifdef FFS
extern	struct vfsops ufs_vfsops;
#define	UFS_VFSOPS	&ufs_vfsops
#else
#define	UFS_VFSOPS	NULL
#endif

#ifdef LFS
extern	struct vfsops lfs_vfsops;
#define	LFS_VFSOPS	&lfs_vfsops
#else
#define	LFS_VFSOPS	NULL
#endif

#ifdef MFS
extern	struct vfsops mfs_vfsops;
#define	MFS_VFSOPS	&mfs_vfsops
#else
#define	MFS_VFSOPS	NULL
#endif

#ifdef NFS
extern	struct vfsops nfs_vfsops;
#define	NFS_VFSOPS	&nfs_vfsops
#else
#define	NFS_VFSOPS	NULL
#endif

#ifdef LOFS
extern	struct vfsops lofs_vfsops;
#define	LOFS_VFSOPS	&lofs_vfsops
#else
#define	LOFS_VFSOPS	NULL
#endif

#ifdef FDESC
extern	struct vfsops fdesc_vfsops;
#define	FDESC_VFSOPS	&fdesc_vfsops
#else
#define	FDESC_VFSOPS	NULL
#endif

#ifdef PORTAL
extern	struct vfsops portal_vfsops;
#define	PORTAL_VFSOPS	&portal_vfsops
#else
#define	PORTAL_VFSOPS	NULL
#endif

#ifdef NULLFS
extern	struct vfsops null_vfsops;
#define NULL_VFSOPS	&null_vfsops
#else
#define NULL_VFSOPS	NULL
#endif

#ifdef UMAPFS
extern	struct vfsops umap_vfsops;
#define UMAP_VFSOPS	&umap_vfsops
#else
#define UMAP_VFSOPS	NULL
#endif


struct vfsops *vfssw[] = {
	NULL,			/* 0 = MOUNT_NONE */
	UFS_VFSOPS,		/* 1 = MOUNT_UFS */
	NFS_VFSOPS,		/* 2 = MOUNT_NFS */
	MFS_VFSOPS,		/* 3 = MOUNT_MFS */
	NULL,			/* 4 = MOUNT_PC */
	LFS_VFSOPS,		/* 5 = MOUNT_LFS */
	LOFS_VFSOPS,		/* 6 = MOUNT_LOFS */
	FDESC_VFSOPS,		/* 7 = MOUNT_FDESC */
	PORTAL_VFSOPS,		/* 8 = MOUNT_PORTAL */
	NULL_VFSOPS,		/* 9 = MOUNT_NULL */
	UMAP_VFSOPS,		/* 10 = MOUNT_UMAP */
	0
};


/*
 *
 * vfs_opv_descs enumerates the list of vnode classes, each with it's own
 * vnode operation vector.  It is consulted at system boot to build operation
 * vectors.  It is NULL terminated.
 *
 */
extern struct vnodeopv_desc ffs_vnodeop_opv_desc;
extern struct vnodeopv_desc ffs_specop_opv_desc;
extern struct vnodeopv_desc ffs_fifoop_opv_desc;
extern struct vnodeopv_desc lfs_vnodeop_opv_desc;
extern struct vnodeopv_desc lfs_specop_opv_desc;
extern struct vnodeopv_desc lfs_fifoop_opv_desc;
extern struct vnodeopv_desc mfs_vnodeop_opv_desc;
extern struct vnodeopv_desc dead_vnodeop_opv_desc;
extern struct vnodeopv_desc fifo_vnodeop_opv_desc;
extern struct vnodeopv_desc spec_vnodeop_opv_desc;
extern struct vnodeopv_desc nfsv2_vnodeop_opv_desc;
extern struct vnodeopv_desc spec_nfsv2nodeop_opv_desc;
extern struct vnodeopv_desc fifo_nfsv2nodeop_opv_desc;
extern struct vnodeopv_desc lofs_vnodeop_opv_desc;
extern struct vnodeopv_desc fdesc_vnodeop_opv_desc;
extern struct vnodeopv_desc portal_vnodeop_opv_desc;
extern struct vnodeopv_desc null_vnodeop_opv_desc;
extern struct vnodeopv_desc umap_vnodeop_opv_desc;

struct vnodeopv_desc *vfs_opv_descs[] = {
	&ffs_vnodeop_opv_desc,
	&ffs_specop_opv_desc,
#ifdef FIFO
	&ffs_fifoop_opv_desc,
#endif
	&dead_vnodeop_opv_desc,
#ifdef FIFO
	&fifo_vnodeop_opv_desc,
#endif
	&spec_vnodeop_opv_desc,
#ifdef LFS
	&lfs_vnodeop_opv_desc,
	&lfs_specop_opv_desc,
#ifdef FIFO
	&lfs_fifoop_opv_desc,
#endif
#endif
#ifdef MFS
	&mfs_vnodeop_opv_desc,
#endif
#ifdef NFS
	&nfsv2_vnodeop_opv_desc,
	&spec_nfsv2nodeop_opv_desc,
#ifdef FIFO
	&fifo_nfsv2nodeop_opv_desc,
#endif
#endif
#ifdef LOFS
	&lofs_vnodeop_opv_desc,
#endif
#ifdef FDESC
	&fdesc_vnodeop_opv_desc,
#endif
#ifdef PORTAL
	&portal_vnodeop_opv_desc,
#endif
#ifdef NULLFS
	&null_vnodeop_opv_desc,
#endif
#ifdef UMAPFS
	&umap_vnodeop_opv_desc,
#endif
	NULL
};

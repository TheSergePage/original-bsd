/*-
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)spint.h	8.1 (Berkeley) 06/06/93
 */

/*
 * The 'spint' (spawn and interrupt) routines use this structure.
 *
 * Note that spint_asm.asm contains an Assembly language version of
 * the following, so keep changes in synch!
 */

typedef struct {
    union REGS		regs;
    struct SREGS	sregs;
    int			int_no;	/* Which interrupt to wait on */
    int			done;	/* Are we done, or just took an interrupt? */
    int			rc;	/* return code */
} Spint;

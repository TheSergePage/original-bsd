/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)abs.s	8.1 (Berkeley) 06/04/93"
#endif /* LIBC_SCCS and not lint */

#include "DEFS.h"

ENTRY(abs)
	movl	4(%esp),%eax
	cmpl	$0,%eax
	jge	1f
	negl	%eax
1:	ret

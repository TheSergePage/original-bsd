/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)
	.asciz "@(#)Ovfork.s	5.6 (Berkeley) 06/01/90"
#endif /* SYSLIBC_SCCS and not lint */

/*
 * @(#)vfork.s	4.1 (Berkeley) 12/21/80
 * C library -- vfork
 */

/*
 * pid = vfork();
 *
 * r1 == 0 in parent process, r1 == 1 in child process.
 * r0 == pid of child in parent, r0 == pid of parent in child.
 *
 * trickery here, due to keith sklower, uses ret to clear the stack,
 * and then returns with a jump indirect, since only one person can return
 * with a ret off this stack... we do the ret before we vfork!
 */

	.set	vfork,66
.globl	_vfork

_vfork:
	.word	0x0000
	movl	16(fp),r2
	movab	here,16(fp)
	ret
here:
	chmk	$vfork
	bcc	vforkok
	jmp	verror
vforkok:
	tstl	r1		# child process ?
	bneq	child	# yes
	bcc 	parent		# if c-bit not set, fork ok
.globl	_errno
verror:
	movl	r0,_errno
	mnegl	$1,r0
	jmp	(r2)
child:
	clrl	r0
parent:
	jmp	(r2)

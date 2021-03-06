divert(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#

#
#  This is a Berkeley-specific configuration file for a specific
#  machine in Electrical Engineering and Computer Sciences at Berkeley,
#  and should not be used elsewhere.   It is provided on the sendmail
#  distribution as a sample only.
#
#  This file is for the primary EECS mail server.
#

include(`../m4/cf.m4')
VERSIONID(`@(#)mail.eecs.mc	8.7 (Berkeley) 04/21/95')
OSTYPE(ultrix4)dnl
DOMAIN(EECS.Berkeley.EDU)dnl
MASQUERADE_AS(EECS.Berkeley.EDU)dnl
MAILER(local)dnl
MAILER(smtp)dnl
define(`confUSERDB_SPEC', `/usr/local/lib/users.eecs.db,/usr/local/lib/users.cs.db,/usr/local/lib/users.coe.db')dnl
DDBerkeley.EDU

# hosts for which we accept and forward mail (must be in .Berkeley.EDU)
CF EECS
FF/etc/sendmail.cw

LOCAL_RULE_0
R< @ $=F . $D . > : $*		$@ $>7 $2		@here:... -> ...
R$* $=O $* < @ $=F . $D . >	$@ $>7 $1 $2 $3		...@here -> ...

R$* < @ $=F . $D . >		$#local $: $1		use UDB

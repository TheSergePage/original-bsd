/*-
 * %sccs.include.proprietary.c%
 *
 *	@(#)xmail.h	8.1 (Berkeley) 06/06/93
 */

#include <stdio.h>
#include <mp.h>
extern MINT *x, *b, *one, *c64, *t45, *z, *q, *r, *two, *t15;
extern char buf[256];
#ifdef debug
#define nin(x, y) m_in(x, 8, y)
#define nout(x, y) m_out(x, 8, y)
#endif

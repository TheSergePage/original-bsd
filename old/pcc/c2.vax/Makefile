#	@(#)Makefile	4.14 (Berkeley) 11/13/91

PROG=	c2
SRCS=	c20.c c21.c c22.c
CFLAGS+=-DCOPYCODE -DC2 -I.
ASDIR=	${.CURDIR}/../../as.${MACHINE}
NOMAN=	noman
CLEANFILES=instrs.c2

c22.o:
	${CC} ${CFLAGS} -R -c ${.IMPSRC}

instrs.c2: ${ASDIR}/instrs
	(echo FLAVOR C2 ; cat ${ASDIR}/instrs) | \
	    awk -f ${ASDIR}/instrs > instrs.c2

.include <bsd.prog.mk>

.depend: instrs.c2

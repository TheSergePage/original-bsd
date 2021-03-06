/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)glob.h	8.1 (Berkeley) 06/02/93
 */

#ifndef _GLOB_H_
#define	_GLOB_H_

#include <sys/cdefs.h>

struct stat;
typedef struct {
	int gl_pathc;		/* Count of total paths so far. */
	int gl_matchc;		/* Count of paths matching pattern. */
	int gl_offs;		/* Reserved at beginning of gl_pathv. */
	int gl_flags;		/* Copy of flags parameter to glob. */
	char **gl_pathv;	/* List of paths matching pattern. */
				/* Copy of errfunc parameter to glob. */
	int (*gl_errfunc) __P((const char *, int));

	/*
	 * Alternate filesystem access methods for glob; replacement
	 * versions of closedir(3), readdir(3), opendir(3), stat(2)
	 * and lstat(2).
	 */
	void (*gl_closedir) __P((void *));
	struct dirent *(*gl_readdir) __P((void *));	
	void *(*gl_opendir) __P((const char *));
	int (*gl_lstat) __P((const char *, struct stat *));
	int (*gl_stat) __P((const char *, struct stat *));
} glob_t;

#define	GLOB_APPEND	0x0001	/* Append to output from previous call. */
#define	GLOB_DOOFFS	0x0002	/* Use gl_offs. */
#define	GLOB_ERR	0x0004	/* Return on error. */
#define	GLOB_MARK	0x0008	/* Append / to matching directories. */
#define	GLOB_NOCHECK	0x0010	/* Return pattern itself if nothing matches. */
#define	GLOB_NOSORT	0x0020	/* Don't sort. */

#ifndef _POSIX_SOURCE
#define	GLOB_ALTDIRFUNC	0x0040	/* Use alternately specified directory funcs. */
#define	GLOB_BRACE	0x0080	/* Expand braces ala csh. */
#define	GLOB_MAGCHAR	0x0100	/* Pattern had globbing characters. */
#define	GLOB_NOMAGIC	0x0200	/* GLOB_NOCHECK without magic chars (csh). */
#define	GLOB_QUOTE	0x0400	/* Quote special chars with \. */
#define	GLOB_TILDE	0x0800	/* Expand tilde names from the passwd file. */
#endif

#define	GLOB_NOSPACE	(-1)	/* Malloc call failed. */
#define	GLOB_ABEND	(-2)	/* Unignored error. */

__BEGIN_DECLS
int	glob __P((const char *, int, int (*)(const char *, int), glob_t *));
void	globfree __P((glob_t *));
__END_DECLS

#endif /* !_GLOB_H_ */

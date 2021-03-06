/*
 * Copyright (c) 1986, 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)process.c	8.1 (Berkeley) 06/04/93";
#endif /* not lint */

#include <sys/param.h>
#include <sys/time.h>

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bug.h"
#include "extern.h"

char	pfile[MAXPATHLEN];			/* permanent file name */

static int getnext __P((void));

/*
 * process --
 *	copy report to permanent file,
 *	update summary file.
 */
int
process()
{
	register int	rval;			/* read return value */
	struct timeval	tp;			/* time of day */
	int	lfd;				/* lock file descriptor */

	if (access(LOCK_FILE, R_OK) || (lfd = open(LOCK_FILE, O_RDONLY, 0)) < 0)
		error("can't find lock file %s.", LOCK_FILE);
	if (flock(lfd, LOCK_EX))
		error("can't get lock.", CHN);
	sprintf(pfile, "%s/%s/%d", dir, folder, getnext());
	fprintf(stderr, "\t%s\n", pfile);
	if (!(freopen(pfile, "w", stdout)))
		error("can't create %s.", pfile);
	rewind(stdin);
	while ((rval = read(fileno(stdin), bfr, sizeof(bfr))) != ERR && rval)
		if (write(fileno(stdout), bfr, rval) != rval)
			error("write to %s failed.", pfile);

	/* append information to the summary file */
	sprintf(bfr, "%s/%s", dir, SUMMARY_FILE);
	if (!(freopen(bfr, "a", stdout)))
		error("can't append to summary file %s.", bfr);
	if (gettimeofday(&tp, (struct timezone *)NULL))
		error("can't get time of day.", CHN);
	printf("\n%s\t\t%s\t%s\t%s\tOwner: Bugs Bunny\n\tStatus: Received\n",
	    pfile, ctime(&tp.tv_sec), mailhead[INDX_TAG].line,
	    mailhead[SUBJ_TAG].found ? mailhead[SUBJ_TAG].line : "Subject:\n");
	(void)flock(lfd, LOCK_UN);
	(void)fclose(stdout);
}

/*
 * getnext --
 *	get next file name (number)
 */
static int
getnext()
{
	register struct dirent *d;		/* directory structure */
	register DIR *dirp;			/* directory pointer */
	register int highval, newval;
	register char *p;

	(void)sprintf(bfr, "%s/%s", dir, folder);
	if (!(dirp = opendir(bfr)))
		error("can't read folder directory %s.", bfr);
	for (highval = -1; d = readdir(dirp);) {
		for (p = d->d_name; *p && isdigit(*p); ++p);
		if (!*p && (newval = atoi(d->d_name)) > highval)
			highval = newval;
	}
	closedir(dirp);
	return(++highval);
}

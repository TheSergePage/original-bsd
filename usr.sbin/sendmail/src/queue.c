# include "sendmail.h"
# include <sys/stat.h>
# include <ndir.h>
# include <signal.h>
# include <errno.h>

# ifndef QUEUE
SCCSID(@(#)queue.c	3.30		08/15/82	(no queueing));
# else QUEUE

SCCSID(@(#)queue.c	3.30		08/15/82);

/*
**  QUEUEUP -- queue a message up for future transmission.
**
**	The queued message should already be in the correct place.
**	This routine just outputs the control file as appropriate.
**
**	Parameters:
**		e -- the envelope to queue up.
**		queueall -- if TRUE, queue all addresses, rather than
**			just those with the QQUEUEUP flag set.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The current request (only unsatisfied addresses)
**			are saved in a control file.
*/

queueup(e, queueall)
	register ENVELOPE *e;
	bool queueall;
{
	char cf[MAXNAME];
	char buf[MAXNAME];
	register FILE *cfp;
	register HDR *h;
	register ADDRESS *q;
	extern char *mktemp();
	register int i;

	/*
	**  Create control file.
	*/

	(void) strcpy(cf, QueueDir);
	(void) strcat(cf, "/tfXXXXXX");
	(void) mktemp(cf);
	cfp = fopen(cf, "w");
	if (cfp == NULL)
	{
		syserr("queueup: cannot create control file %s", cf);
		return;
	}
	(void) chmod(cf, 0600);

# ifdef DEBUG
	if (tTd(40, 1))
		printf("queueing in %s\n", cf);
# endif DEBUG

	/*
	**  If there is no data file yet, create one.
	*/

	if (e->e_df == NULL)
	{
		register FILE *dfp;

		(void) strcpy(buf, QueueDir);
		(void) strcat(buf, "/dfXXXXXX");
		e->e_df = newstr(mktemp(buf));
		dfp = fopen(e->e_df, "w");
		if (dfp == NULL)
		{
			syserr("queueup: cannot create %s", e->e_df);
			(void) fclose(cfp);
			return;
		}
		(void) chmod(e->e_df, 0600);
		(*e->e_putbody)(dfp, Mailer[1], FALSE);
		(void) fclose(dfp);
	}

	/*
	**  Output future work requests.
	*/

	/* output name of data file */
	fprintf(cfp, "D%s\n", e->e_df);

	/* output name of sender */
	fprintf(cfp, "S%s\n", e->e_from.q_paddr);

	/* output timeout */
	fprintf(cfp, "T%ld\n", TimeOut);

	/* output message priority */
	fprintf(cfp, "P%ld\n", e->e_msgpriority);

	/* output message class */
	fprintf(cfp, "C%d\n", e->e_class);

	/* output macro definitions */
	for (i = 0; i < 128; i++)
	{
		register char *p = e->e_macro[i];

		if (p != NULL && i != (int) 'b')
			fprintf(cfp, "M%c%s\n", i, p);
	}

	/* output list of recipient addresses */
	for (q = e->e_sendqueue; q != NULL; q = q->q_next)
	{
# ifdef DEBUG
		if (tTd(40, 1))
		{
			printf("queueing ");
			printaddr(q, FALSE);
		}
# endif DEBUG
		if (queueall || bitset(QQUEUEUP, q->q_flags))
			fprintf(cfp, "R%s\n", q->q_paddr);
	}

	/* output headers for this message */
	for (h = e->e_header; h != NULL; h = h->h_link)
	{
		if (h->h_value == NULL || h->h_value[0] == '\0')
			continue;
		fprintf(cfp, "H");
		if (h->h_mflags != 0 && bitset(H_CHECK|H_ACHECK, h->h_flags))
			mfdecode(h->h_mflags, cfp);
		fprintf(cfp, "%s: %s\n", h->h_field, h->h_value);
	}

	/*
	**  Clean up.
	*/

	(void) fclose(cfp);
	(void) strcpy(buf, QueueDir);
	(void) strcat(buf, "/cfXXXXXX");
	(void) mktemp(buf);
	if (link(cf, buf) < 0)
		syserr("cannot link(%s, %s), df=%s", cf, buf, e->e_df);
	else
		(void) unlink(cf);

# ifdef LOG
	/* save log info */
	if (LogLevel > 9)
		syslog(LOG_INFO, "%s queueup: cf=%s, df=%s\n", MsgId, buf, e->e_df);
# endif LOG

	/* disconnect this temp file from the job */
	e->e_df = NULL;
}
/*
**  RUNQUEUE -- run the jobs in the queue.
**
**	Gets the stuff out of the queue in some presumably logical
**	order and processes them.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		runs things in the mail queue.
*/

bool	ReorderQueue;		/* if set, reorder the send queue */
int	QueuePid;		/* pid of child running queue */

runqueue(forkflag)
	bool forkflag;
{
	extern reordersig();

	/*
	**  See if we want to go off and do other useful work.
	*/

	if (forkflag)
	{
		QueuePid = dofork();
		if (QueuePid > 0)
		{
			/* parent */
			if (QueueIntvl != 0)
				setevent(QueueIntvl, reordersig, TRUE);
			return;
		}
	}

	/*
	**  Start making passes through the queue.
	**	First, read and sort the entire queue.
	**	Then, process the work in that order.
	**		But if you take too long, start over.
	**	There is a race condition at the end -- we could get
	**		a reorder signal after finishing the queue.
	**		In this case we will hang for one more queue
	**		interval -- clearly a botch, but rare and
	**		relatively innocuous.
	*/

	for (;;)
	{
		/* order the existing work requests */
		orderq();

		/* arrange to reorder later */
		ReorderQueue = FALSE;
		if (QueueIntvl != 0)
			setevent(QueueIntvl, reordersig, FALSE);

		/* process them once at a time */
		while (WorkQ != NULL)
		{
			WORK *w = WorkQ;

			WorkQ = WorkQ->w_next;
			dowork(w);
			free(w->w_name);
			free((char *) w);
			if (ReorderQueue)
				break;
		}

		/* if we are just doing one pass, then we are done */
		if (QueueIntvl == 0)
			finis();

		/* wait for work -- note (harmless) race condition here */
		while (!ReorderQueue)
		{
			if (forkflag)
				finis();
			pause();
		}
	}
}
/*
**  REORDERSIG -- catch the reorder signal and tell sendmail to reorder queue.
**
**	Parameters:
**		parent -- if set, called from parent (i.e., not
**			really doing the work).
**
**	Returns:
**		none.
**
**	Side Effects:
**		sets the "reorder work queue" flag.
*/

reordersig(parent)
	bool parent;
{
	if (!parent)
	{
		/* we are in a child doing queueing */
		ReorderQueue = TRUE;
	}
	else
	{
		/*
		**  In parent.  If the child still exists, we want
		**  to do nothing.  If the child is gone, we will
		**  start up a new one.
		**  If the child exists, it is responsible for
		**  doing a queue reorder.
		**  This code really sucks.
		*/

		if (kill(QueuePid, SIGALRM) < 0)
		{
			/* no child -- get zombie & start new one */
			static int st;

			(void) wait(&st);
			runqueue(TRUE);
		}
	}

	/*
	**  Arrange to get this signal again.
	*/

	setevent(QueueIntvl, reordersig, parent);
}
/*
**  ORDERQ -- order the work queue.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Sets WorkQ to the queue of available work, in order.
*/

# define WLSIZE		120	/* max size of worklist per sort */

orderq()
{
	register struct direct *d;
	register WORK *w;
	register WORK **wp;		/* parent of w */
	DIR *f;
	register int i;
	WORK wlist[WLSIZE];
	int wn = 0;
	extern workcmpf();
	extern char *QueueDir;

	/* clear out old WorkQ */
	for (w = WorkQ; w != NULL; )
	{
		register WORK *nw = w->w_next;

		WorkQ = nw;
		free(w->w_name);
		free((char *) w);
		w = nw;
	}

	/* open the queue directory */
	f = opendir(QueueDir);
	if (f == NULL)
	{
		syserr("orderq: cannot open %s", QueueDir);
		return;
	}

	/*
	**  Read the work directory.
	*/

	while (wn < WLSIZE && (d = readdir(f)) != NULL)
	{
		char cbuf[MAXNAME];
		char lbuf[MAXNAME];
		FILE *cf;
		register char *p;

		/* is this an interesting entry? */
		if (d->d_name[0] != 'c')
			continue;

		/* yes -- find the control file location */
		(void) strcpy(cbuf, QueueDir);
		(void) strcat(cbuf, "/");
		p = &cbuf[strlen(cbuf)];
		(void) strcpy(p, d->d_name);

		/* open control file */
		cf = fopen(cbuf, "r");
		if (cf == NULL)
		{
			/* this may be some random person sending hir msgs */
			/* syserr("orderq: cannot open %s", cbuf); */
			errno = 0;
			continue;
		}
		wlist[wn].w_name = newstr(cbuf);

		/* extract useful information */
		while (fgets(lbuf, sizeof lbuf, cf) != NULL)
		{
			fixcrlf(lbuf, TRUE);

			switch (lbuf[0])
			{
			  case 'P':		/* message priority */
				(void) sscanf(&lbuf[1], "%ld", &wlist[wn].w_pri);
				break;
			}
		}
		wn++;
		(void) fclose(cf);
	}
	(void) closedir(f);

	/*
	**  Sort the work directory.
	*/

	qsort(wlist, wn, sizeof *wlist, workcmpf);

	/*
	**  Convert the work list into canonical form.
	*/

	wp = &WorkQ;
	for (i = 0; i < wn; i++)
	{
		w = (WORK *) xalloc(sizeof *w);
		w->w_name = wlist[i].w_name;
		w->w_pri = wlist[i].w_pri;
		w->w_next = NULL;
		*wp = w;
		wp = &w->w_next;
	}

# ifdef DEBUG
	if (tTd(40, 1))
	{
		for (w = WorkQ; w != NULL; w = w->w_next)
			printf("%32s: pri=%ld\n", w->w_name, w->w_pri);
	}
# endif DEBUG
}
/*
**  WORKCMPF -- compare function for ordering work.
**
**	Parameters:
**		a -- the first argument.
**		b -- the second argument.
**
**	Returns:
**		-1 if a < b
**		0 if a == b
**		1 if a > b
**
**	Side Effects:
**		none.
*/

# define PRIFACT	1800		/* bytes each priority point is worth */

workcmpf(a, b)
	register WORK *a;
	register WORK *b;
{
	if (a->w_pri == b->w_pri)
		return (0);
	else if (a->w_pri > b->w_pri)
		return (1);
	else
		return (-1);
}
/*
**  DOWORK -- do a work request.
**
**	Parameters:
**		w -- the work request to be satisfied.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The work request is satisfied if possible.
*/

dowork(w)
	register WORK *w;
{
	register int i;
	auto int xstat;

# ifdef DEBUG
	if (tTd(40, 1))
		printf("dowork: %s pri %ld\n", w->w_name, w->w_pri);
# endif DEBUG

	/*
	**  Fork for work.
	*/

	i = fork();
	if (i < 0)
	{
		syserr("dowork: cannot fork");
		return;
	}

	if (i == 0)
	{
		char buf[MAXNAME];

		/*
		**  CHILD
		**	Change the name of the control file to avoid
		**		duplicate deliveries.   Then run the file
		**		as though we had just read it.
		**	We save an idea of the temporary name so we
		**		can recover on interrupt.
		*/

		(void) alarm(0);
		FatalErrors = FALSE;
		QueueRun = TRUE;
		MailBack = TRUE;
		(void) strcpy(buf, QueueDir);
		(void) strcat(buf, "/tfXXXXXX");
		(void) mktemp(buf);
		if (link(w->w_name, buf) < 0)
		{
			/* this can happen normally; another queuer sneaks in */
			/* syserr("dowork: link(%s, %s)", w->w_name, buf); */
			exit(EX_OK);
		}
		ControlFile = newstr(buf);
		(void) unlink(w->w_name);

		/* create ourselves a transcript file */
		openxscrpt();

		/* do basic system initialization */
		initsys();

		/* read the queue control file */
		readqf(buf);

		/* do the delivery */
		if (!FatalErrors)
			sendall(CurEnv, FALSE);

		/* if still not sent, perhaps we should time out.... */
# ifdef DEBUG
		if (tTd(40, 3))
			printf("CurTime=%ld, TimeOut=%ld\n", CurTime, TimeOut);
# endif DEBUG
		if (CurEnv->e_queueup && CurTime > TimeOut)
			timeout(w);

		/* get rid of the temporary file -- a new cf will be made */
		ControlFile = NULL;
		(void) unlink(buf);

		/* finish up and exit */
		finis();
	}

	/*
	**  Parent -- pick up results.
	*/

	errno = 0;
	while ((i = wait(&xstat)) > 0 && errno != EINTR)
	{
		if (errno == EINTR)
		{
			errno = 0;
		}
	}
}
/*
**  READQF -- read queue file and set up environment.
**
**	Parameters:
**		cf -- name of queue control file.
**
**	Returns:
**		none.
**
**	Side Effects:
**		cf is read and created as the current job, as though
**		we had been invoked by argument.
*/

readqf(cf)
	char *cf;
{
	register FILE *f;
	char buf[MAXLINE];
	register char *p;
	register int i;

	/*
	**  Open the file created by queueup.
	*/

	f = fopen(cf, "r");
	if (f == NULL)
	{
		syserr("readqf: no cf file %s", cf);
		return;
	}

	/*
	**  Read and process the file.
	*/

	if (Verbose)
		printf("\nRunning %s\n", cf);
	p = buf;
	while (fgets(p, sizeof buf - (p - buf), f) != NULL)
	{
		/*
		**  Collect any continuation lines...
		*/

		i = fgetc(f);
		if (i != EOF)
			ungetc(i, f);
		if (i == ' ' || i == '\t')
		{
			p += strlen(p);
			continue;
		}
		fixcrlf(p, TRUE);

		switch (buf[0])
		{
		  case 'R':		/* specify recipient */
			sendto(&buf[1], 1, (ADDRESS *) NULL, &CurEnv->e_sendqueue);
			break;

		  case 'H':		/* header */
			(void) chompheader(&buf[1], FALSE);
			break;

		  case 'S':		/* sender */
			setsender(newstr(&buf[1]));
			break;

		  case 'D':		/* data file name */
			CurEnv->e_df = newstr(&buf[1]);
			TempFile = fopen(CurEnv->e_df, "r");
			if (TempFile == NULL)
				syserr("readqf: cannot open %s", CurEnv->e_df);
			break;

		  case 'T':		/* timeout */
			(void) sscanf(&buf[1], "%ld", &TimeOut);
			break;

		  case 'P':		/* message priority */
			(void) sscanf(&buf[1], "%ld", &CurEnv->e_msgpriority);

			/* make sure that big things get sent eventually */
			CurEnv->e_msgpriority -= WKTIMEFACT;
			break;

		  case 'C':		/* message class */
			(void) sscanf(&buf[1], "%hd", &CurEnv->e_class);
			break;

		  case 'M':		/* define macro */
			define(buf[1], newstr(&buf[2]));
			break;

		  default:
			syserr("readqf(%s): bad line \"%s\"", cf, buf);
			break;
		}
	}
}
/*
**  TIMEOUT -- process timeout on queue file.
**
**	Parameters:
**		w -- pointer to work request that timed out.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Returns a message to the sender saying that this
**		message has timed out.
*/

timeout(w)
	register WORK *w;
{
	char buf[MAXLINE];
	extern char *TextTimeOut;

# ifdef DEBUG
	if (tTd(40, 3))
		printf("timeout(%s)\n", w->w_name);
# endif DEBUG
	message(Arpa_Info, "Message has timed out");

	/* return message to sender */
	(void) sprintf(buf, "Cannot send mail for %s", TextTimeOut);
	(void) returntosender(buf, &CurEnv->e_from, TRUE);

	/* arrange to remove files from queue */
	CurEnv->e_queueup = FALSE;
}

# endif QUEUE

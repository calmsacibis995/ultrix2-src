#ifndef lint
static	char	*sccsid = "@(#)quit.c	1.1	(ULTRIX)	1/8/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

# include	"monitor.h"
# include	<ingres.h>
# include	<aux.h>
# include	<signal.h>




/*
**  QUIT INGRES
**
**	This routine starts the death of the other processes.  It
**	then prints out the logout message, and then waits for the
**	rest of the system to die.  Note, however, that no relations
**	are removed; this must be done using the PURGE command.
**
**	Trace Flags:
**		35
*/

/* list of fatal signals */
char	*Siglist[] =
{
	"Signal 0",
	"hangup",
	"interrupt",
	"quit",
	"illegal instruction",
	"trace trap",
	"IOT",
	"EMT",
	"floating point exception",
	"killed",
	"bus error",
	"segmentation violation",
	"bad system call",
	"broken pipe",
	"alarm",
};

quit()
{
	register int	ndx;
	register int	pidptr;
	register int	err;
	char		buf[100];
	int		status;
	int		pidlist[50];
	extern int	(*ExitFn)();
	extern		exit();
	extern int	sys_nerr;
	extern char	*sys_errlist[];
	char		indexx[0400];
	extern char	SysIdent[];

#	ifdef xMTR1
	if (tTf(35, -1))
		printf("entered quit\n");
#	endif

	/* INTERCEPT ALL FURTHER INTERRUPTS */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	ExitFn = exit;

	cm_close();

#	ifdef xMTR3
	if (tTf(35, 2))
		printf("unlinking %s\n", Qbname);
#	endif

	/* REMOVE THE QUERY-BUFFER FILE */
	fclose(Qryiop);
	unlink(Qbname);
	if (Trapfile != NULL)
		fclose(Trapfile);
	pidptr = 0;
	err = 0;

	/* clear out the system error index table */
	for (ndx = 0; ndx < 0400; ndx++)
		indexx[ndx] = 0;

	/* wait for all process to terminate */
	while ((ndx = wait(&status)) != -1)
	{
#		ifdef xMTR2
		if (tTf(35, 5))
			printf("quit: pid %u: %d/%d\n",
				ndx, status >> 8, status & 0177);
#		endif
		pidlist[pidptr++] = ndx;
		if ((status & 0177) != 0)
		{
			printf("%d: ", ndx);
			ndx = status & 0177;
			if (ndx > sizeof Siglist / sizeof Siglist[0])
				printf("Abnormal Termination %d", ndx);
			else
				printf("%s", Siglist[ndx]);
			if ((status & 0200) != 0)
				printf(" -- Core Dumped");
			printf("\n");
			err++;
			indexx[0377 - ndx]++;
		}
		else
		{
			indexx[(status >> 8) & 0377]++;
		}
	}
	if (err)
	{
		printf("pid list:");
		for (ndx = 0; ndx < pidptr; ndx++)
			printf(" %u", pidlist[ndx]);
		printf("\n");
	}

	/* print index of system errors */
	err = 0;
	for (ndx = 1; ndx <= 0377; ndx++)
	{
		if (indexx[ndx] == 0)
			continue;
		if (ndx <= sys_nerr)
		{
			if (err == 0)
				printf("\nUNIX error dictionary:\n");
			printf("%3d: %s\n", ndx, sys_errlist[ndx]);
		}
		if (err == 0)
			err = ndx;
	}
	if (err > 0 && err <= sys_nerr)
		printf("\n");

	/* PRINT LOGOUT CUE ? */
	if (Nodayfile >= 0)
	{
		time(buf);
		printf("%s logout\n%s", SysIdent, ctime(buf));
		if (getuser(Usercode, buf) == 0)
		{
			for (ndx = 0; buf[ndx]; ndx++)
				if (buf[ndx] == ':')
					break;
			buf[ndx] = 0;
			printf("goodbye %s ", buf);
		}
		else
			printf("goodbye ");
		printf("-- come again\n");
	}
#	ifdef xMTR1
	if (tTf(35, 3))
		printf("quit: exit(%d)\n", err);
#	endif
	exit(err);
}

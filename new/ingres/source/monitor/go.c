#ifndef lint
static	char	*sccsid = "@(#)go.c	1.1	(ULTRIX)	1/8/85";
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
# include	<resp.h>
# include	<symbol.h>
# include	<pv.h>
# include	<pipes.h>
# include	<setjmp.h>




/*
**  PROCESS QUERY
**
**	The appropriate messages are printed, and the query is scanned.
**	Tokens are passed to the parser.  A parser response is then
**	expected.
**
**	Trace Flags:
**		5
*/

# define	QRYTRAP		"{querytrap}"

jmp_buf		GoJmpBuf;

go()
{
	FILE		*iop;
	auto char	c;
	register char	*p;
	extern int	fgetc();
	pb_t		pb;
	extern char	*macro();

	clrline(1);
	fflush(Qryiop);
	if ((iop = fopen(Qbname, "r")) == NULL)
		syserr("go: open 1");
	if (Nodayfile >= 0)
		printf("Executing . . .\n\n");

#	ifdef xMTM
	if (tTf(76, 1))
		timtrace(3, 0);
#	endif

	if (!Nautoclear)
		Autoclear = 1;

	/* arrange to call the parser */
	initp();
	call_setup(&pb, mdPARSER, NULL);
	pb_prime(&pb, PB_REG);
	pb.pb_proc = 1;		/**** PARSER MUST BE IN PROC ONE ****/
	send_off(&pb, 0, NULL);
	pb_tput(PV_EOF, "", 0, &pb);
	macinit(fgetc, iop, 1);
	while ((c = macgetch()) > 0)
		pb_put(&c, 1, &pb);
	pb_flush(&pb);
	fclose(iop);

	/* wait for the response */
	setjmp(GoJmpBuf);
	readinput(&pb);

	if (Resp.resp_tups >= 0)
		macdefine("{tuplecount}", locv(Resp.resp_tups), TRUE);
	
	if (Error_id == 0 && (p = macro(QRYTRAP)) != NULL)
		trapquery(&Resp, p);
	
	resetp();

	mcall("{continuetrap}");

#	ifdef xMTM
	if (tTf(76, 1))
		timtrace(4, 0);
#	endif
	prompt("\ncontinue");
}

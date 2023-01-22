#ifndef lint
static	char	*sccsid = "@(#)error.c	1.1	(ULTRIX)	1/8/85";
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

# include	"ctlmod.h"
# include	"pipes.h"
# include	<pv.h>


/*
**  ERROR -- Ingres error message generator
**
**	Error message `num' is sent up towards the caller with param-
**	eters `msg'.  This routine may have any number of parameters,
**	but the last one must be zero.
**
**	Parameters:
**		num -- the error number.
**		msg -- the first in a null-terminated list of
**			arguments, all of type char *, which are
**			passed as arguments to the error.
**
**	Returns:
**		non-locally
**
**	Side Effects:
**		Many and vast.  The message gets passed up the
**		stack of activations.  The activation stack gets
**		popped.  Etc.
**
**	Trace Flags:
**		6.0 - 6.7
*/

/*VARARGS2*/
error(num, msg)
int	num;
char	*msg;
{
	register char	**x;
	pb_t		pb;
	extern jmp_buf	CmReset;
	typedef int	ftype();

# ifdef xCTR1
	if (tTf(6, 0))
		lprintf("error: %d, Ctx.ctx_cmark %d\n", num, Ctx.ctx_cmark);
# endif

	/* flush the current input pipe */
	while (!bitset(PB_EOF, Ctx.ctx_ppb->pb_stat))
		pb_read(Ctx.ctx_ppb);

	/* free up some stack space (in case called from need) */
	if (num == -ERR_QBUF)
	{
		freebuf(Qbuf, Ctx.ctx_cmark);
		Ctx.ctx_pmark = Ctx.ctx_cmark;
		Ctx.ctx_new = TRUE;
	}

	/* create an error context & set the message parameters */
	initp();
	setp(PV_INT, (char *) num, 0);
	x = &msg;
	while (*x != NULL)
		setp(PV_STR, *x++, 0);

	/* send it to my caller */
	pb_prime(&pb, PB_ERR);
	call_setup(&pb, PB_NONE, (ftype *) NULL);

	/* send the message to the correct place & unwind the stack */
	proc_err(&pb, Ctx.ctx_pc, Ctx.ctx_pv);
	syserr("error: proc_err");
}
/*
**  NFERROR -- non-fatal error.
**
**	Errors of this type are passed directly to the front end.
**
**	Parameters:
**		(same as error)
**
**	Returns:
**		The error number.
**
**	Side Effects:
**		The message is sent off to the front end.  It
**		is marked as being informational only.
*/

/*VARARGS2*/
nferror(num, msg)
int	num;
char	*msg;
{
	register char	**p;
	pb_t		pb;
	typedef int	ftype();

	initp();
	setp(PV_INT, (char *) num, 0);
	for (p = &msg; *p != NULL; p++)
		setp(PV_STR, *p, 0);
	pb_prime(&pb, PB_ERR);
	call_setup(&pb, PB_NONE, (ftype *) NULL);
	pb.pb_stat |= PB_INFO;
	pb.pb_proc = PB_FRONT;
	send_off(&pb, Ctx.ctx_pc, Ctx.ctx_pv);
	pb_flush(&pb);
	resetp();
	return (num);
}

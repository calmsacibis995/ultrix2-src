#ifndef lint
static	char	*sccsid = "@(#)send_off.c	1.1	(ULTRIX)	1/8/85";
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

# include	<stdio.h>
# include	"ctlmod.h"
# include	"pipes.h"
# include	<symbol.h>
# include	<tree.h>
# include	<resp.h>


/*
**  SEND_OFF -- send a call off to another process
**
**	This routine sends a call off to the process indicated
**	ppb->pb_proc.  It also sends parameters pc & pv.  It
**	just sends it, and does not wait for a response.
**
**	WARNING:
**		The pipe buffer is not flushed after putting out
**		the call; this must be done by the caller.
**
**	Parameters:
**		ppb -- a pointer to a pipe block which is used
**			as the I/O area, and also identifies
**			the target process.
**		pc -- the parameter count.
**		pv -- the parameter vector.
**
**	Returns:
**		none
**
**	Side Effects:
**		none
**
**	Trace Flags:
**		11.0 - 11.7
*/

send_off(ppb, pc, pv)
register pb_t	*ppb;
int		pc;
register PARM	*pv;
{
	register int	i;

# ifdef xCTR1
	if (tTf(11, 0))
	{
		lprintf("send_off: ");
		pb_dump(ppb, FALSE);
	}
# endif

	/*
	**  Flush out catalog pages and standard output so that
	**	changes will show through.
	*/

	closecatalog(FALSE);
	fflush(stdout);
	clrbit(PB_EOF, ppb->pb_stat);

	/*
	**  Cycle through the parameters writing each one out.
	*/

	if (ppb->pb_type == PB_RESP)
	{
		pb_put((char *) &Resp, sizeof Resp, ppb);
/*
		send_arg(&Resp.resp_rval, ppb);
*/
	}
	else
	{
		for (i = 0; i < pc; i++)
		{
			send_arg(&pv[i], ppb);
		}
	}

	/* deallocate the space allocated to these parameters */
	freebuf(Qbuf, Ctx.ctx_pmark);
# ifdef xCTR1
	if (tTf(11, 1))
		lprintf("send_off: free %d\n", Ctx.ctx_pmark);
# endif
}
/*
**  SEND_ARG -- send argument down pipe.
**
**	Parameters:
**		pparm -- the parameter to send.
**		ppb -- the pipe to send it to.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
**
**	Called By:
**		send_off
**		do_seq
*/

send_arg(pparm, ppb)
register PARM	*pparm;
register pb_t	*ppb;
{
	register char	*p;
	extern		pb_put();

	switch (pparm->pv_type)
	{
	  case PV_STR:
		p = pparm->pv_val.pv_str;
		pb_tput(PV_STR, p, length(p) + 1, ppb);
		break;

	  case PV_INT:
		pb_tput(PV_INT, &pparm->pv_val.pv_int, sizeof (pparm->pv_val.pv_int), ppb);
		break;

	  case PV_QTREE:
		pb_tput(PV_QTREE, "", 0, ppb);
		writeqry((QTREE *) pparm->pv_val.pv_qtree, pb_put, (int) ppb);
		break;

	  case PV_TUPLE:
		pb_tput(PV_TUPLE, pparm->pv_val.pv_tuple, pparm->pv_len, ppb);
		break;

	  case PV_EOF:
		break;

	  default:
		syserr("send_arg: type %d", pparm->pv_type);
	}
}

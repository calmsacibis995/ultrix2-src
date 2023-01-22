#ifndef lint
static	char	*sccsid = "@(#)do_seq.c	1.1	(ULTRIX)	1/8/85";
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
# include	<resp.h>


/*
**  DO_SEQ -- do sequence of states
**
**	The sequence of states indicated by the start state
**	ppb->pb_st is executed.
**
**	Recursion and multiple processes are implemented in
**	this routine.  For each iteration through the top
**	loop, the next state to call is checked.  If the state
**	is local (in this process), the state is executed and
**	the next state is computed.  If the state is remote,
**	(contained in another process), we write out the
**	current call on a pipe which will (eventually) get
**	to the target process.  In this case (and the system
**	reset case) the next state is unknown.  This causes
**	this routine to read the input pipe for a command.
**	The command read is then executed -- notice that it
**	does not matter if this command is part of this level
**	of call (a sibling of the 'call'd process) or a lower
**	level (a descendent), since 'call' saved the state.
**
**	If there are no more states in the chain after execution
**	of a local state, then a response block is generated.
**	The 'readinput' call can also generate one of these.
**	If the return is local (to this process), it is just
**	returned.  Otherwise, it sends it off to the process
**	currently blocked for this state sequence to complete,
**	and the PB_UNKNOWN state is reentered.
**
**	Parameters:
**		ppb -- a pointer to the pipe block which
**			(a) describes the state to call and
**			(b) provides an I/O area.
**
**	Returns:
**		none
**
**	Side Effects:
**		Lots.  The side effects of all the states.
**		Leaves 'Resp' set to the response of the last
**			state in the chain.
**
**	Called By:
**		call
**		main
**
**	Trace Flags:
**		2.0 - 2.7
*/

do_seq(ppb)
register pb_t	*ppb;
{
	register int	i;

	/*
	**  Top Loop.
	**	Iterate until we have a response block intended
	**	for this process (and hence this state, since
	**	state invocations are properly nested inside
	**	process invocations).
	**	We also insure that we can always get at the
	**	current pipe block.
	**
	**	We return from the setjmp with non-zero value if
	**	we get a fatal error.
	*/

	Ctx.ctx_ppb = ppb;
	if (setjmp(Ctx.ctx_jbuf) != 0)
		return;

	for (;;)
	{
# ifdef xCTR1
		if (tTf(2, 0))
			lprintf("do_seq: state %d\n", ppb->pb_st);
# endif
		/* take cases */
		switch (ppb->pb_st)
		{
		  case PB_UNKNOWN:
			/*
			**  Read the input and get a state, since we
			**  don't have one already.  This will set
			**  Parmc & Parmv and change *ppb.  The
			**  state it found will be processed next
			**  time through the loop.
			*/

			readinput(ppb);
			break;

		  case PB_NONE:
			/*
			**  The 'no next state' case, i.e., a response.
			**	This only happens if the response is
			**	for this process, so we just return.
			*/

# ifdef xCTR1
			if (tTf(2, 1))
				lprintf("do_seq: exit\n");
# endif
			return;

		  default:
			/*
			** The state is known, let do_st do all
			** the dirty work.  Hand it the global
			** Parmc & Parmv to pass to the function.
			** Do_st returns the next state to execute.
			*/

			ppb->pb_st = i = do_st(ppb, Ctx.ctx_pc, Ctx.ctx_pv);
			if (i == PB_NONE)
			{
# ifdef xCM_DEBUG
				if (ppb->pb_resp == PB_UNKNOWN)
					syserr("do_seq: pb_resp");
# endif
				ppb->pb_proc = ppb->pb_resp;
				if (ppb->pb_proc != Cm.cm_myproc)
				{
					/* write to correct process */
					pb_prime(ppb, PB_RESP);
					send_off(ppb, 0, (PARM *)NULL);
					pb_flush(ppb);

					/* next state is unknown */
					ppb->pb_st = PB_UNKNOWN;
				}
			}
			break;
		}
	}
}

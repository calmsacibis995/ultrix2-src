#ifndef lint
static	char	*sccsid = "@(#)initp.c	1.1	(ULTRIX)	1/8/85";
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
# include	<ingres.h>
# include	<aux.h>
# include	<tree.h>


/*
**  INITP -- initialize parameters for call
**
**	Saves the current context if one is in effect, and
**	initializes the new context.  Further 'setp' calls
**	will set parameters, and 'call' will call the desired
**	function.
**
**	'Initp' is also used by the error routine, except
**	that the 'call' is simulated internally in 'error'
**
**	Parameters:
**		none
**
**	Returns:
**		nothing
**
**	Side Effects:
**		Saves the context 'Ctx' if active.
**		Initializes 'Ctx'.
**
**	Trace Flags:
**		4.0 - 4.3
*/

initp()
{
	register ctx_t	*pctx;
	register int	sz;
	extern char *need();

# ifdef xCTR1
	if (tTf(4, 0))
		lprintf("initp: new %d mark %d\n", Ctx.ctx_new, markbuf(Qbuf));
# endif

	if (!Ctx.ctx_new)
	{
		/*
		** Save the context.
		**	Mark the current point in Qbuf.
		**	Allocate space from Qbuf and save the context.
		**	Only save as much of the pv as is used.
		**	Leave 'pctx' as a pointer to the save area.
		*/

		Ctx.ctx_cmark = markbuf(Qbuf);
		sz = Ctx.ctx_pc * sizeof Ctx.ctx_pv[0]
		     + ((char *) Ctx.ctx_pv - (char *) &Ctx);
		pctx = (ctx_t *) need(Qbuf, sz);
		bmove((char *) &Ctx, (char *) pctx, sz);
		
		/*
		**  Initialize new context.
		**	The current context describes the attributes of
		**		the next context (e.g., ctx_size).
		**	ctx_pmark marks the base of the parameters in
		**		Qbuf.
		**	ctx_qt points to the saved query tree header (if
		**		one exists).
		*/

		Ctx.ctx_link = pctx;
		Ctx.ctx_size = sz;
		Ctx.ctx_pmark = markbuf(Qbuf);
		Ctx.ctx_qt = NULL;
		Ctx.ctx_resp = Cm.cm_myproc;

		/*
		**  If the QT hdr is in use by the
		**  context we just saved, arrange to have it saved
		**  later (if anyone every tries to use it).
		*/

		if (Qt.qt_active > 0 && Qt.qt_ctx == NULL)
		{
			Qt.qt_ctx = (char *) pctx;
		}
	}
	Ctx.ctx_pc = 0;
	Ctx.ctx_init = Ctx.ctx_new = TRUE;
	Ctx.ctx_errfn = NULL;
	Ctx.ctx_fn = NULL;
	Ctx.ctx_glob = NULL;

# ifdef xCTR2
	if (tTf(4, 2))
		lprintf("initp: cmark %d pmark %d link %x\n",
		    Ctx.ctx_cmark, Ctx.ctx_pmark, Ctx.ctx_link);
# endif
}

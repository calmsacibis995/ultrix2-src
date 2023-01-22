#ifndef lint
static	char	*sccsid = "@(#)cm_cleanup.c	1.1	(ULTRIX)	1/8/85";
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
# include	<signal.h>


/*
**  CM_CLEANUP -- cleanup after interrupt or error.
**
**	This routine does things like call the interrupt cleanup
**	function, reset the input, etc.
**
**	Parameters:
**		typ -- the type of cleanup:
**			1 -- fatal error (from error [error.c]).
**			2 -- keyboard interrupt.
**
**	Returns:
**		never (uses non-local jump to ctlmod/main.c).
**
**	Side Effects:
**		Proc_name & Cm.cm_input are reset.
**
**	Trace Flags:
**		0
*/

cm_cleanup(typ)
int	typ;
{
	register int		i;
	register struct fn_def	*f;
	extern char		*Proc_name;
	extern jmp_buf		CmReset;
	extern			rubcatch();
	register ctx_t		*ctx;

# ifdef xCTR2
	if (tTf(0, 13))
		printf("cm_cleanup: %d\n", typ);
# endif

	/*
	**  Call all interrupt cleanup functions for active
	**	modules.
	*/

	for (i = 0; i < NumFunc; i++)
	{
		f = FuncVect[i];
		if (f->fn_active > 0)
		{
			Ctx.ctx_name = Proc_name = f->fn_name;
			(*f->fn_cleanup)(typ);
		}
	}

	/* clean up memory */
	for (ctx = &Ctx; ctx != NULL; ctx = ctx->ctx_link)
	{
		if (ctx->ctx_qt != NULL)
			free(ctx->ctx_qt);
		if (ctx->ctx_glob != NULL)
		{
			bmove(ctx->ctx_glob, ctx->ctx_fn->fn_gptr, ctx->ctx_fn->fn_gsize);
			free(ctx->ctx_glob);
		}
	}

	/* return to top of loop */
	longjmp(CmReset, typ);
}

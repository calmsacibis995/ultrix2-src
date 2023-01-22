/*
 *		@(#)func.h	1.1	(ULTRIX)	1/8/85
 */

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

/*
**  FUNC.H -- declarations for function headers.
**
**	Version:
**		@(#)func.h	7.1	2/5/81
*/


/* the function definition struct */
struct fn_def
{
	char		*fn_name;	/* the name of the function */
	int		(*fn_fn)();	/* a pointer to the actual function */
	int		(*fn_initfn)();	/* initialization function */
	int		(*fn_cleanup)();/* interrupt cleanup function */
	char		*fn_gptr;	/* pointer to global space */
	unsigned	fn_gsize;	/* size of global space */
	short		*fn_tvect;	/* the trace vector itself */
	short		fn_tsize;	/* size of trace vector */
	char		fn_tflag;	/* the trace flag letter */
	char		fn_active;	/* > 0 if active */
};

extern struct fn_def	*FuncVect[];
extern int		NumFunc;	/* the number of functions */

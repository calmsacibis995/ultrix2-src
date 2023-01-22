#ifndef lint
static	char	*sccsid = "@(#)freebuf.c	1.1	(ULTRIX)	1/8/85";
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



/*
**  FREEBUF.C -- more routines for LIFO dynamic buffer allocation [need.c]
**
**	These routines allow the deallocation of a need() type buffer,
**	and also using the same buffer for various SERIALIZED purposes
**	by marking the end of one, beginning of the next.
**
**	Defines:
**		freebuf()
**		markbuf()
**		seterr()
*/






/* structure that the routines use to allocate space */
struct nodbuffer
{
	int		nleft;		/* bytes left */
	int		err_num;	/* error code on overflow */
	int		(*err_func)();	/* error function on overflow */
	char		*xfree;		/* next free byte */
	char		buffer [1];	/*beginning of buffer area */
};

/*
**  MARKBUF -- Mark a place in the buffer to deallocate to
**
**	Parameters:
**		bf -- buffer
**
**	Returns:
**		int >= 0 marking place in buffer (should be used in calling
**			freebuf())
**
**	Side Effects:
**		none
*/

markbuf(bf)
struct nodbuffer	*bf;
{
	register struct nodbuffer	*buf;

	buf = bf;
	return (buf->nleft);
}
/*
**  FREEBUF -- frees part of a buffer
**
**	Parameters:
**		bf -- buffer
**		bytes -- a previous return from markbuf().
**
**	Returns:
**		none
**
**	Side Effects:
**		none
*/

freebuf(bf, bytes)
struct nodbuffer	*bf;
int			bytes;
{
	register struct nodbuffer	*buf;
	register int			i;

	buf = bf;
	i = bytes - buf->nleft;
	if (i < 0)
		syserr("freebuf %d, %d", i, bytes);
	buf->xfree -= i;
	buf->nleft += i;
}
/*
**  SETERR -- change the error info for a buffer
**
**	Parameters:
**		bf -- buffer
**		errnum -- new overflow error code
**		err_func -- new error handler
**
**	Returns:
**		none
**
**	Side Effects:
**		adjusts buffer structure
*/

seterr(bf, errnum, err_func)
struct nodbuffer	*bf;
int			errnum;
int			(*err_func)();
{
	register struct nodbuffer	*buf;
	register int			(*erf)();

	buf = bf;
	erf = err_func;
	buf->err_num = errnum;
	bf->err_func = erf;
}

#ifndef lint
static	char	*sccsid = "@(#)need.c	1.1	(ULTRIX)	1/8/85";
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
**  NEED.C -- general buffer allocation routines
**
**	allow buffers with LIFO de-allocation
*/






/* structure that the routines use to allocate space */
struct nodbuffer
{
	int	nleft;		/* bytes left */
	int	err_num;	/* error code on overflow */
	int	(*err_func)();	/* error function on overflow */
	char	*xfree;		/* next free byte */
	char	buffer[1];	/*beginning of buffer area */
};

/*
**  NEED -- allocate space from a buffer
**
**	On buffer overflow, calls err_func from that field
**	in the buffer with the error code err_code from that field
**	in the buffer, then returns 0.
**	need() guarantees an even adress on return.
**
**	Parameters:
**		bf -- buffer
**		nbytes -- number of bytes desired
**
**	Returns:
**		pointer to allocated area
**		on buffer overflow returns 0.
**
**	Side Effects:
**		adjusts buffer structure to reflect allocation.
*/

char *
need(bf, nbytes)
struct nodbuffer 	*bf;
int			nbytes;
{
	register char			*x;
	register struct nodbuffer	*buf;
	register int			i;

	buf = bf;
	i = nbytes;
	if (i > buf->nleft)
	{
		(*buf->err_func)(buf->err_num, 0);
		return (0);
	}
	i += i & 01;
	x = buf->xfree;
	buf->xfree += i;
	buf->nleft -= i;
	clrmem(x, i);
	return(x);
}

/*
**  INITBUF -- initialize a buffer
**
**	Must be called before the first need() call on the buffer.
**
**	Parameters:
**		bf -- buffer
**		size -- size fo buffer area
**		err_num -- error code for overflow
**		err_func -- function to call with err_code on error
**
**	Returns:
**		none
**
**	Side Effects:
**		initializes buffer structure
**
**	Diagnostics:
**		"initbuf : odd buffer adress 0%o" -- buffers must start
**			at an even address.
*/

initbuf(bf, size, err_num, err_func)
char	*bf;
int	size;
int	err_num;
int	(*err_func)();
{
	register struct nodbuffer	*buf;
	register 			i;

	buf = (struct nodbuffer *) bf;
	i = (int) buf;
	if (i & 01)
		syserr("initbuf : odd buffer adress 0%o", buf);
	buf->nleft = size - sizeof *buf;
	buf->xfree = buf->buffer;
	buf->err_num = err_num;
	buf->err_func = err_func;
}

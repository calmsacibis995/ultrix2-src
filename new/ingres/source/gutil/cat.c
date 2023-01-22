#ifndef lint
static	char	*sccsid = "@(#)cat.c	1.1	(ULTRIX)	1/8/85";
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
**  CAT -- "cat" a file
**
**	This function is essentially identical to the UNIX cat(I).
**
**	Parameters:
**		file -- the name of the file to be cat'ed
**
**	Returns:
**		zero -- success
**		else -- failure (could not open file)
**
**	Side Effects:
**		"file" is open and read once through; a copy is made
**			to the standard output.
*/

cat(file)
char	*file;
{
	char		buf[512];
	register int	i;
	register int	fd;

	fd = open(file, 0);
	if (fd < 0)
		return (1);

	while ((i = read(fd, buf, 512)) > 0)
	{
		write(1, buf, i);
	}

	return (0);
}

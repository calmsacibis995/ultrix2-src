

#ifndef lint
static	char	*sccsid = "@(#)xunlink.c	1.1	(ULTRIX)	12/9/84";
#endif lint

/*
	Interface to unlink(II) which handles all error conditions.
	Returns 0 on success,
	fatal() on failure.
*/

xunlink(f)
{
	if (unlink(f))
		return(xmsg(f,"xunlink"));
	return(0);
}

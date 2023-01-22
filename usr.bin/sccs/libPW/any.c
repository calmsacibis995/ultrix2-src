

#ifndef lint
static	char	*sccsid = "@(#)any.c	1.1	(ULTRIX)	12/9/84";
#endif lint

/*
	If any character of `s' is `c', return 1
	else return 0.
*/

any(c,s)
register char c, *s;
{
	while (*s)
		if (*s++ == c)
			return(1);
	return(0);
}

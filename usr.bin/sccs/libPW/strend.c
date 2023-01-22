

#ifndef lint
static	char	*sccsid = "@(#)strend.c	1.1	(ULTRIX)	12/9/84";
#endif lint

char *strend(p)
register char *p;
{
	while (*p++)
		;
	return(--p);
}

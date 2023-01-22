

#ifndef lint
static	char	*sccsid = "@(#)logname.c	1.1	(ULTRIX)	12/9/84";
#endif lint

char *
logname()
{
	return((char *)getenv("LOGNAME"));
}

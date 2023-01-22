

#ifndef lint
static	char	*sccsid = "@(#)userexit.c	1.1	(ULTRIX)	12/9/84";
#endif lint

/*
	Default userexit routine for fatal and setsig.
	User supplied userexit routines can be used for logging.
*/

userexit(code)
{
	return(code);
}

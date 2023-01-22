

#ifndef lint
static	char	*sccsid = "@(#)rename.c	1.1	(ULTRIX)	12/9/84";
#endif lint

# include "errno.h"
# include "fatal.h"

/*
	rename (unlink/link)
	Calls xlink() and xunlink().
*/

rename(oldname,newname)
char *oldname, *newname;
{
	extern int errno;

	if (unlink(newname) < 0 && errno != ENOENT)
		return(xunlink(newname));

	if (xlink(oldname,newname) == Fvalue)
		return(-1);
	return(xunlink(oldname));
}

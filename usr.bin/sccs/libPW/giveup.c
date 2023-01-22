

#ifndef lint
static	char	*sccsid = "@(#)giveup.c	1.1	(ULTRIX)	12/9/84";
#endif lint

/*
	Chdir to "/" if argument is 0.
	Set IOT signal to system default.
	Call abort(III).
	(Shouldn't produce a core when called with a 0 argument.)
*/

# include "signal.h"

giveup(dump)
{
	if (!dump)
		chdir("/");
	signal(SIGIOT,0);
	abort();
}
#ifndef lint
static char sccsid[] = "@(#)sysacct.c	1.4 (decvax!larry) 3/6/84";
#endif

#include <sys/types.h>


/*******
 *	sysacct(bytes, time)	output accounting info
 *	time_t time;
 *	long bytes;
 */

sysacct(bytes, time)
time_t time;
long bytes;
{
	return;
}

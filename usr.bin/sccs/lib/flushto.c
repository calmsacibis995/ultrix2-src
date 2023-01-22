

#ifndef lint
static	char	*sccsid = "@(#)flushto.c	1.1	(ULTRIX)	12/9/84";
#endif lint

# include	"../hdr/defines.h"


flushto(pkt,ch,put)
register struct packet *pkt;
register char ch;
int put;
{
	register char *p;
	char *getline();

	while ((p = getline(pkt)) != NULL && !(*p++ == CTLCHAR && *p == ch))
		pkt->p_wrttn = put;

	if (p == NULL)
		fmterr(pkt);

	putline(pkt,(char *) 0);
}

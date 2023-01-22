

#ifndef lint
static	char	*sccsid = "@(#)fmterr.c	1.1	(ULTRIX)	12/9/84";
#endif lint

# include	"../hdr/defines.h"


fmterr(pkt)
register struct packet *pkt;
{
	fclose(pkt->p_iop);
	sprintf(Error,"format error at line %u (co4)",pkt->p_slnno);
	fatal(Error);
}

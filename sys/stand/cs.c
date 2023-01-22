#ifndef lint
static	char	*sccsid = "@(#)cs.c	1.3	(ULTRIX) 	3/23/86";
#endif lint

/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1985 1986 by                      *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/

/* ---------------------------------------------------------------------
 * Modification History
 *
 * 	15 Jun 1985 -- JAW 
 *		standalone version of the VAX8200 console floppy.
 *
 * ---------------------------------------------------------------------
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/gnode.h"

#include "saio.h"
#include "savax.h"
#include "../stand/cs.h"

struct v8200rx50 *v8200rx50;

csopen(io)
	register struct iob *io;
{
	int i;
	int unit;	

	v8200rx50 = (struct v8200rx50 *) 0x200b0000;
	if (io->i_unit > 2 || io->i_unit < 1) 
		_stop("cs bad unit");
	io->i_boff = 0;
}


csstrategy(io,func)
	register struct  iob *io;
	int func;
{	
	int cscnt;
	u_short csblk, rx5com, junk, track, sector;
	char *csbuf;
	

	cscnt = io->i_cc;
	csblk = io->i_bn;
	csbuf = io->i_ma;

	while(cscnt > 0) {
		rx5com = 0;
		track = csblk/ 10 ;
		sector = (((csblk % 10)/5) + ((csblk+track)*2)) % 10;
		sector++ ;

    		if( ++track > 79 ) track = 0 ;
		if ( func == READ ) {
			rx5com |= RX_READ ;
			junk = v8200rx50->rx5ca;
		} else {
			rx5com |= RX_WRITE;
			cstransfer(csbuf,RX_FILL,cscnt);
		}
		if (io->i_unit == 2)
			rx5com |= 2;


		v8200rx50->rx5cs1 = track;
		v8200rx50->rx5cs2 = sector;
		v8200rx50->rx5cs0 = rx5com & 0x7f;
		junk = v8200rx50->rx5go; 
		
		while((v8200rx50->rx5cs0 & 0x08) == 0)
			continue;

		if (v8200rx50->rx5cs0 & 0x80)
			printf("rx5 error\n");

		if (func == READ)
			cstransfer(csbuf,RX_EMPTY,cscnt);

		cscnt = cscnt - 512;
		csblk++;
		csbuf = csbuf + 512;
	}

	return(io->i_cc);
}


cstransfer(bp,op,amount)
	char *bp;
	short   op;
	int	amount;
{
	register short  nbytes;
	register char   *buf, *buf1;
	int junk;


	nbytes = v8200rx50->rx5ca;                   
	buf  = bp;

	if (amount >=512)
		nbytes = 512;
	else
		nbytes=amount;

	if (op == RX_FILL ) {
		buf1 = (char *)&v8200rx50->rx5fdb;

		while(nbytes) {
			*buf1 = *buf++ ;
			--nbytes;
		}
	} else { 
		buf1 = (char *)&v8200rx50->rx5edb;
		while(nbytes){
			*buf++ = *buf1;
			--nbytes;
		}
	}

}


csioctl(io, cmd, arg)
	struct iob *io;
	int cmd;
	caddr_t arg;
{

	return (ECMD);
}



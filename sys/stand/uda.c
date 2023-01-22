/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*	uda.c	6.1	83/07/29	*/

# ifndef lint
static	char	*sccsid = "@(#)uda.c	1.14	(ULTRIX)	4/18/86";
# endif not lint

/* ------------------------------------------------------------------------
 * Modification History: /sys/stand/uda.c
 *
 * 03-Oct-85 -- jaw
 *	MVAXII booting hang fix.
 *
 * 22-Aug-85 -- map
 *	Zero mscp packet fields before SCC and ONL commands.
 *	Fixes non-zero reserved fields.
 *
 * 26-Jun-85 -- map
 *	Fix definition of saveCSR to fix loading problem
 *
 * 24-Jun-85 -- map
 *	Change window size definition
 *
 * 18-Jun-85 -- jaw
 *	fixup so rabads would load....can_bda caused problem.
 *
 * 04-Apr-85 - Mark Parenti
 *	Add BDA support.
 *	Make it a UDA/BDA boot driver.
 *	General cleanup.
 *
 *	Stephen Reilly, 17-Jul-84
 * 001- Add uda_drive_size variable that is used by the rc25 image copy
 *	routine.
 *
 * 9-Jan-84 - Tom Tresvik
 *	Problem of not being able to attempt to open more than one file
 *	fixed.  The controller characteristics are set up only once and the
 *	target drive is only put online only once.  Both of these changes are
 * 	in raopen.  uda_offline and ra_offline[8] are added to keep track of
 *	controller and drives.
 *
 * 29 Dec 83 --jmcg
 *	Pass 22 bits of address through instead of just 18.  Supports
 *	RQDX1 on MicroVAX without using pseudo-map-registers.
 *
 * 28 Dec 83 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		uda.c	6.1	83/07/29
 *
 * ------------------------------------------------------------------------
 */


/*
 * UDA50/RAxx disk device driver
 */
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/time.h"
#include "../h/gnode.h"
#include "../h/devio.h"

#include "savax.h"

#include "saio.h"

/*
 * Parameters for the communications area
 */
#define	NRSPL2	0
#define	NCMDL2	0
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)


#include "../vaxuba/udareg.h"
#include "../vaxuba/ubareg.h"
#include "../vax/mscp.h"
#include "../vaxbi/buareg.h"

u_short udastd[] = { 0772150 };

struct iob	cudbuf;

struct udadevice *udaddr = 0;

struct bi_nodespace *birg_addr;

struct uda {
	struct udaca	uda_ca;
	struct mscp	uda_rsp;
	struct mscp	uda_cmd;
} uda;

struct uda *ud_ubaddr;			/* Unibus address of uda structure */

int uda_off[] = { 0, 15884, 0, -1, -1, -1, 49324, 131404 };

struct mscp *udcmd();


extern long saveCSR;		/* location containing boot device CSR */
extern char can_bda;		/* could be a BDA */
char bda_flg = 0;		/* If set then booting from BDA */
char uda_offline = 1;		/* Flag to prevent multiple STCON */
char ra_offline[8] = {1,1,1,1,1,1,1,1};
				/* Flag to prevent multiple ONLIN */
int uda_drive_size;		/* 001 Used by the rc25 image copy routine */

#define SA_W(addr)	*(bda_flg? &((addr)->udasaw): &((addr)->udasa) )
#define BIspace	0x00001FFF		/* BI node address space */

raopen(io)
	register struct iob *io;
{
	register struct mscp *mp;
	
	int i;

	/*
	 * Have the UDA controller characteristics already been set up
	 * (STCON)?
	 */
	if (uda_offline) {
		if (udaddr == 0) {
#define WINDOW	0x007E0000

			if (can_bda && !(saveCSR&WINDOW))    /* If can be a BDA and csr is not a BI Adapter window */
							     /* then it is a BDA */
				{
				bda_flg = 1;
				saveCSR &=  ~(BIspace);	/* Make sure we point at base address */
				birg_addr = (struct bi_nodespace *)saveCSR;
				udaddr =(struct udadevice *)(saveCSR|BDA_IP);
				}
			else {
				bda_flg = 0;
				udaddr =(struct udadevice *)ubamem(io->i_unit, udastd[0]);
			}
		}
		if (ud_ubaddr == 0) {
			cudbuf.i_unit = io->i_unit;
			cudbuf.i_ma = (caddr_t)&uda;
			cudbuf.i_cc = sizeof(uda);
			ud_ubaddr = (bda_flg? &uda: (struct uda *)ubasetup(&cudbuf, 2));
		}
		for (i=0; i < 3;i++) {
			if (bda_flg)
				{
				badwrte(&(((struct bi_nodespace *)birg_addr)->biic.biic_ctrl));
				while (birg_addr->biic.biic_ctrl & BICTRL_BROKE); /* Wait for BROKE to clear */
				}
			else
				udaddr->udaip = 0;
		
			DELAY(1000);
			while ((udaddr->udasa & (UDA_STEP1 | UDA_ERR)) == 0);

			SA_W(udaddr) = UDA_ERR|(NCMDL2<<11)|(NRSPL2<<8);
	
			DELAY(1000);
			while ((udaddr->udasa & (UDA_STEP2 | UDA_ERR)) == 0);
			SA_W(udaddr) = (short)&ud_ubaddr->uda_ca.ca_ringbase;

			DELAY(1000);
			while ((udaddr->udasa & (UDA_STEP3|UDA_ERR)) == 0);

			SA_W(udaddr) = (short)(((int)&ud_ubaddr->uda_ca.ca_ringbase) >> 16);

			DELAY(1000);
			while ((udaddr->udasa & (UDA_STEP4|UDA_ERR)) == 0) ;

			if ((udaddr->udasa & UDA_ERR) == 0) break;

		}
		
		if (i > 2) {
			_stop("ra: init");
			asm("halt");
		}	
		SA_W(udaddr) = UDA_GO;
		uda.uda_ca.ca_rspdsc[0] = (long)&ud_ubaddr->uda_rsp.mscp_cmdref;
		uda.uda_ca.ca_cmddsc[0] = (long)&ud_ubaddr->uda_cmd.mscp_cmdref;
		uda.uda_cmd.mscp_bytecnt = uda.uda_cmd.mscp_buffer = 0;
		uda.uda_cmd.mscp_cntflgs = 0;

		if (udcmd(M_OP_STCON) == 0) {
			_stop("ra: open error, STCON");
			return;
		}
		uda_offline = 0;
	}

	uda.uda_cmd.mscp_bytecnt = uda.uda_cmd.mscp_buffer = 0;

	uda.uda_cmd.mscp_unit = io->i_unit&7;
	/* 
	 * Has this unit been issued and ONLIN?
	 */
	if (ra_offline[uda.uda_cmd.mscp_unit]) {
		if ((mp = udcmd(M_OP_ONLIN)) == 0) {		/*001*/
			_stop("ra: open error, ONLIN");
			return;
		}
		else
			uda_drive_size = (daddr_t) mp->mscp_untsize;	/*001*/

		ra_offline[uda.uda_cmd.mscp_unit] = 0;
	}
	if (io->i_boff < 0 || io->i_boff > 7 || uda_off[io->i_boff] == -1)
		_stop("ra: bad unit");
	io->i_boff = uda_off[io->i_boff];
}

struct mscp *
udcmd(op)
	int op;
{
	struct mscp *mp;
	int i;

	uda.uda_cmd.mscp_opcode = op;
	uda.uda_rsp.mscp_header.uda_msglen = sizeof (struct mscp);
	uda.uda_cmd.mscp_header.uda_msglen = sizeof (struct mscp);
	uda.uda_ca.ca_rspdsc[0] |= UDA_OWN|UDA_INT;
	uda.uda_ca.ca_cmddsc[0] |= UDA_OWN|UDA_INT;
	i = udaddr->udaip;
	for (;;) {
		if (udaddr->udasa & UDA_ERR)
			{
			return(0);
			}

		if (uda.uda_ca.ca_cmdint)
			uda.uda_ca.ca_cmdint = 0;
		if (uda.uda_ca.ca_rspint)
			break;
	}
	uda.uda_ca.ca_rspint = 0;
	mp = &uda.uda_rsp;
	if (mp->mscp_opcode != (op|M_OP_END) ||
	    (mp->mscp_status&M_ST_MASK) != M_ST_SUCC)
		return(0);
	return(mp);
}

rastrategy(io, func)
	register struct iob *io;
{
	register struct mscp *mp;
	int ubinfo;

	if (!bda_flg)
		ubinfo = ubasetup(io, 1);
	mp = &uda.uda_cmd;
	mp->mscp_lbn = io->i_bn;
	mp->mscp_unit = io->i_unit&7;
	mp->mscp_bytecnt = io->i_cc;
	mp->mscp_buffer = (bda_flg? (long)io->i_ma: (ubinfo & 0x3fffff) | (((ubinfo>>28)&0xf)<<24));
	if ((mp = udcmd(func == READ ? M_OP_READ : M_OP_WRITE)) == 0) {
		printf("ra: I/O error\n");
		if (!bda_flg)
			ubafree(io, ubinfo);
		return(-1);
	}
	if (!bda_flg)
		ubafree(io, ubinfo);
	return(io->i_cc);
}

/*ARGSUSED*/
raioctl(io, cmd, arg)
	struct iob *io;
	int cmd;
	caddr_t arg;
{

	return (ECMD);
}

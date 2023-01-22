/*
 * udabads.c
 */

#ifndef lint
static	char	*sccsid = "@(#)udabads.c	1.13  (ULTRIX)        4/18/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*	uda.c	6.1	83/07/29	*/

/*
 * --------------------------------------------------------------------
 *
 * Modification History:
 *
 * 18-Oct-85  Darrell Dunnuck
 *	added some debug code to compare iob's.
 *
 * 26-Jul-85  jaw
 *	fix for MVAXII hanging
 *
 * 26-Jul-85  Darrell Dunnuck
 *	Verified that the RQDX3 did not have the same problem as the
 *	RQDX1 -- which returns the wrong block (off by one) when a
 *	bad block is reported -- and removed the code that subtracted
 *	one from the block number.
 *
 * 26-Jun-85 - map
 *	Change definition of saveCSR to fix load problem
 *
 * 20-Jun-85 - Mark Parenti
 *	Added BDA support and various minor fixes.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 18-Jun-85 --jaw
 *	fix so rabads would load...can_bda is problem...
 *
 *	Stephen Reilly, 17-Jul-84
 * 001- Add uda_drive_size variable that is used by the rc25 image copy
 *	routine.
 *
 * 9-Jan-84 - Tom Tresvik
 *	Problem of not being able to attempt to open more than one file
 *	fixed.	The controller characteristics are set up only once and the
 *	target drive is only put online only once.  Both of these changes are
 *	in raopen.  uda_offline and ra_offline[8] are added to keep track of
 *	controller and drives.
 * --------------------------------------------------------------------
 */

/*
 * UDA50/RAxx disk device driver
 */
#include "../machine/pte.h"

#include "../h/time.h"
#include "../h/param.h"
#include "../h/gnode.h"
#include "../h/devio.h"

#include "savax.h"

#include "../vax/cpu.h"

#include "saio.h"

#include "ra_saio.h"

/*
 * Parameters for the communications area
 */
#define NRSPL2	0
#define NCMDL2	0
#define NRSP	(1<<NRSPL2)
#define NCMD	(1<<NCMDL2)

#include "../vaxuba/ubareg.h"
#include "../vaxuba/udareg.h"
#include "../vaxbi/buareg.h"
#include "../vax/mscp.h"

#define rprintd4	if(badsdebug > 4)printf
#define rprintd5	if(badsdebug > 5)printf
#define rprintd6	if(badsdebug > 6)printf

/*  Define BDA stuff */

extern long saveCSR;		/* location containing boot device CSR */
extern char can_bda;		/* can it be a BDA ? */
char bda_flg = 0;		/* If set then booting from BDA */
/*  end of BDA stuff */

extern int badsdebug;
int	udabdp = 1;
u_short udastd[] = { 0772150 };

struct iob	cudbuf;
#ifdef DEBUG
struct iob	iob_sav;
#endif DEBUG

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


int uda_offline = 1;		/* Flag to prevent multiple STCON */
int ra_offline[8] = {1,1,1,1,1,1,1,1};
				/* Flag to prevent multiple ONLIN */
int uda_drive_size;		/* 001 Used by the rc25 image copy routine */

int bad_lbn;			/* lbn of bad block from ACCESS end message */

/*
 * Most of the following defines and variables are
 * shared with the RABADS program.
 */

#define RP_WRT	1	/* RABADS command is WRITE */
#define RP_RD	2	/* RABADS command is READ */
#define RP_REP	3	/* RABADS command is REPLACE */
#define RP_AC	4	/* RABADS command is ACCESS */


int	ra_badc;		/* RABADS command flag */
int	ra_badm;		/* RABADS command modifier */
daddr_t ra_rbn; 		/* RABADS RBN for REPLACE command */
int	ra_openf;	/* Controller has been initialized flag */
int	ra_stat;	/* Info saved for error messages */
int	ra_ecode;
int	ra_eflags;
int	ra_ctid;	/* controller type ID + micro-code rev level */
char	*ra_dct;	/* controller type for error messages */
int	command_modifier = 0;	/* lets you poke the modifier value so
				 * you can lower the drive ECC threshold.
				 */

/*
 * Drive info obtained from on-line command
 */

struct	ra_drv	ra_drv[MAXDRIVE];

/*
 * BDA macros
*/

#define SA_W(addr)	*(bda_flg? &((addr)->udasaw): &((addr)->udasa) )
#define BIspace 0x00001FFF		/* BI node address space */

raopen(io)
	register struct iob *io;
{
	register struct mscp *mp;
	int i;
	int unit;	/* drive unit number */

	/*
	 * Have the UDA controller characteristics already been set up
	 * (STCON)?
	 */
	if (uda_offline) {
		uda.uda_cmd.mscp_version = 0;	/* zero for second controller */
		if (udaddr == 0) {
#define WINDOW	0x007E0000

			if (can_bda && !(saveCSR&WINDOW))    /* If can be a BDA and csr is not a BI Adapter window */
							     /* then it is a BDA */
				{
				bda_flg = 1;
				saveCSR &=  ~(BIspace); /* Make sure we point at base address */
				birg_addr = (struct bi_nodespace *)saveCSR;
				}
			else
				bda_flg = 0;		/* Not a BDA */
			udaddr = (bda_flg?(struct udadevice *)(saveCSR|BDA_IP): (struct udadevice *)ubamem(io->i_unit, udastd[0]));
			}

		if (ud_ubaddr == 0) {
			cudbuf.i_unit = io->i_unit;
			cudbuf.i_ma = (caddr_t)&uda;
			rprintd6("udabads: cudbuf.i_ma = 0x%x\n", cudbuf.i_ma);
			cudbuf.i_cc = sizeof(uda);
			ud_ubaddr = (bda_flg? &uda: (struct uda *)ubasetup(&cudbuf, 2));
			rprintd6("udabads: ud_ubaddr = 0x%x, udaddr = 0x%x\n",
				ud_ubaddr, udaddr);

		}
		if (bda_flg)
			{
			badwrte(&(((struct bi_nodespace *)birg_addr)->biic.biic_ctrl));

			while (birg_addr->biic.biic_ctrl & BICTRL_BROKE); /* Wait for BROKE to clear */
			}
		else
			udaddr->udaip = 0;

		while ((udaddr->udasa & UDA_STEP1) == 0)
			;
		SA_W(udaddr) = UDA_ERR|(NCMDL2<<11)|(NRSPL2<<8);

		while ((udaddr->udasa & UDA_STEP2) == 0)
			;
		SA_W(udaddr) = (short)&ud_ubaddr->uda_ca.ca_ringbase|
		    ((cpu == VAX_780 || cpu == VAX_8600 || cpu == VAX_8200)
				? UDA_PI : 0);

		while ((udaddr->udasa & UDA_STEP3) == 0)
			;
		SA_W(udaddr) = (short)(((int)&ud_ubaddr->uda_ca.ca_ringbase) >> 16);

		while ((udaddr->udasa & UDA_STEP4) == 0)
			;
		/*
		 * added for bad block hack
		 */
		ra_ctid = udaddr->udasa & 0x1ff; /* save controller id */
		switch((ra_ctid>>4) & 0x1f) {
		case UDA50:
			ra_dct = "UDA50";
			break;
		case KLESI:
			ra_dct = "KLESI";
			break;
		case RUX1:
			ra_dct = "RUX1";
			break;
		case UDA50A:
			ra_dct = "UDA50A";
			break;
		case RQDX1:
			ra_dct = "RQDX1";
			break;
		case QDA50:
			ra_dct = "KDA50";
			break;
		case QDA25:
			ra_dct = "KDA25";
			break;
		case RQDX3:
			ra_dct = "RQDX3";
			break;
		case BDA50:
			ra_dct = "KDB50";
			break;
		default:
			ra_dct = "UNKNOWN";
			break;
		}
		/*
		 * End of code added for bad block hack
		 */
		SA_W(udaddr) = UDA_GO;
		rprintd6("udabads: building packet\n");
		uda.uda_ca.ca_rspdsc[0] = (long)&ud_ubaddr->uda_rsp.mscp_cmdref;
		uda.uda_ca.ca_cmddsc[0] = (long)&ud_ubaddr->uda_cmd.mscp_cmdref;
		rprintd6("udabads: rspdsc = 0x%x, cmddsc = 0x%x\n",
			uda.uda_ca.ca_rspdsc[0], uda.uda_ca.ca_cmddsc[0]);
		uda.uda_cmd.mscp_unit = uda.uda_cmd.mscp_modifier = 0;
		uda.uda_cmd.mscp_flags = 0;
		uda.uda_cmd.mscp_bytecnt = uda.uda_cmd.mscp_buffer = 0;
		uda.uda_cmd.mscp_errlgfl = uda.uda_cmd.mscp_copyspd = 0;
		uda.uda_cmd.mscp_opcode = 0;
		uda.uda_cmd.mscp_cntflgs = 0;
		rprintd6("udabads: timeout = 0x%x\n", uda.uda_cmd.mscp_hsttmo);
		if (udcmd(M_OP_STCON) == 0) {
			_stop("ra: open error, STCON");
			return;
		}
		uda_offline = 0;
	}
	uda.uda_cmd.mscp_modifier = 0;
	uda.uda_cmd.mscp_flags = 0;
	uda.uda_cmd.mscp_bytecnt = uda.uda_cmd.mscp_buffer = 0;
	uda.uda_cmd.mscp_errlgfl = uda.uda_cmd.mscp_copyspd = 0;
	uda.uda_cmd.mscp_opcode = 0;
	uda.uda_cmd.mscp_cntflgs = 0;
	uda.uda_cmd.mscp_unit = io->i_unit&7;

	/*
	 * Has this unit been issued an ONLIN?
	 */
	if (ra_offline[uda.uda_cmd.mscp_unit]) {
		if ((mp = udcmd(M_OP_ONLIN)) == 0) {		/*001*/
			unit = io->i_unit&7;	/* units 0-7 only */
			ra_drv[unit].ra_mediaid = 0;	/* Clear drive media id */
			printf("\nCan't bring unit %d online\n",uda.uda_cmd.mscp_unit);
			return;
		}
		else
			{
			unit = io->i_unit&7;	/* units 0-7 only */
			uda_drive_size = (daddr_t) mp->mscp_untsize;	/*001*/
			ra_drv[unit].ra_dsize = uda_drive_size;
			/*
			 * set drive info for bad block hack
			 */
			udcmd(M_OP_GTUNT);
			ra_drv[unit].ra_mediaid = (mp->mscp_mediaid);
			ra_drv[unit].ra_rbns = mp->mscp_rbns;
			ra_drv[unit].ra_ncopy = mp->mscp_rctcpys;
			ra_drv[unit].ra_rctsz = mp->mscp_rctsize;
			ra_drv[unit].ra_trksz = mp->mscp_track;
			ra_drv[unit].ra_online = 1;
			/*
			 * end of drive info for bad block hack
			 */
		}

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
	int count;

	mp = &uda.uda_cmd;
	rprintd4("LBN = %d, %s = %d\n", mp->mscp_lbn,
		op == M_OP_REPLC ? "RBN" : "bytecnt", mp->mscp_bytecnt);
	rprintd4("op = 0x%x, modifier = 0x%x, buffer = 0x%x\n",
		op, mp->mscp_modifier, mp->mscp_buffer);
	uda.uda_cmd.mscp_opcode = op;
	uda.uda_rsp.mscp_header.uda_msglen = sizeof (struct mscp);
	if (uda.uda_ca.ca_rspint) {
		uda.uda_ca.ca_rspint = 0;
		mp = &uda.uda_rsp;
		printf ("udcmd: stray rsp packet (top)\n");
		printf ("\t opcode = 0x%x\n", mp->mscp_opcode);
		printf ("\t status = 0x%x\n", mp->mscp_status);
		printf ("\tmsgtype = 0x%x\n",
			(mp->mscp_header.uda_credits & 0xf0) >> 4);
		printf ("\t msglen = %d <decimal>\n",
			mp->mscp_header.uda_msglen);
		printf ("\t   vcid = 0x%x\n",
			mp->mscp_header.uda_vcid);
	}
	uda.uda_cmd.mscp_header.uda_msglen = sizeof (struct mscp);
	uda.uda_ca.ca_rspdsc[0] |= UDA_OWN|UDA_INT;
	uda.uda_ca.ca_cmddsc[0] |= UDA_OWN|UDA_INT;
	rprintd6("udabads: head bang coming\n");
	i = udaddr->udaip;
	for (;;) {
		if (uda.uda_ca.ca_cmdint)
			uda.uda_ca.ca_cmdint = 0;
		if (uda.uda_ca.ca_rspint)
			break;
		if (udaddr->udasa & UDA_ERR) {
			printf("controller fault -- sa = 0x%x\n",
				udaddr->udasa & 0xffff);
				if (badsdebug < 4)
					_stop("can't recover -- halting");
				else
					_stop("Oh no Mr. Bill !!\n");
		}
		if(uda.uda_ca.ca_bdp){
			struct iob io;
			rprintd5("purge requested, %d\n", uda.uda_ca.ca_bdp);
#define L_TO_MSCP(x) ((struct mscp *)(&(x)))
			io.i_ma = (char *)(L_TO_MSCP(uda.uda_ca.ca_cmddsc[0])->mscp_buffer);
			io.i_unit = (int) (L_TO_MSCP(uda.uda_ca.ca_cmddsc[0])->mscp_unit);
			ubafree(io, uda.uda_ca.ca_bdp<<28);
			uda.uda_ca.ca_bdp = 0;
			SA_W(udaddr) = 0;	/* signal purge complete */
			break;
		}
	}
	count = 0;
	mp = &uda.uda_rsp;
	/* wait for the packet we want */
	rprintd4("op = 0x%x, opcode = 0x%x\n", op, mp->mscp_opcode);
	while (mp->mscp_opcode != (op|M_OP_END) && (count++ < 10)) {
		if (uda.uda_ca.ca_rspint) {
			if((uda.uda_ca.ca_rspdsc[0] & UDA_OWN) == 0) {
				printf("udcmd: stray rsp packet (bottom)\n");
				printf ("\t opcode = 0x%x\n", mp->mscp_opcode);
				printf ("\t status = 0x%x\n", mp->mscp_status);
				printf ("\tmsgtype = 0x%x\n",
					(mp->mscp_header.uda_credits & 0xff) >> 4);
				printf ("\t msglen = %d <decimal>\n",
					mp->mscp_header.uda_msglen);
				printf ("\t  vcid = 0x%x\n",
					mp->mscp_header.uda_vcid);
				uda.uda_ca.ca_rspint = 0;
			}
		}
		else
			uda.uda_ca.ca_rspdsc[0] |= UDA_OWN|UDA_INT;
		i = udaddr->udaip;
		DELAY(3000000);
	}
	uda.uda_ca.ca_rspint = 0;
	ra_stat = mp->mscp_status;
	ra_ecode = mp->mscp_opcode&0377;
	ra_eflags = mp->mscp_flags&0377;
	rprintd4("op = 0x%x, opcode = 0x%x, ra_stat = 0x%x, ra_ecode = 0x%x, ra_eflags = 0x%x\n",
		op, mp->mscp_opcode, ra_stat, ra_ecode, ra_eflags);
	if(ra_ecode == (M_OP_ACCES | M_OP_END)) {
		rprintd4("Magic = %d\n",mp->mscp_lbn);
		bad_lbn = mp->mscp_lbn;
	}
	return((mp->mscp_opcode == (op | M_OP_END) &&
		(mp->mscp_status & M_ST_MASK) == M_ST_SUCC) ? mp : 0);
}

rastrategy(io, func)
	register struct iob *io;
{
	register struct mscp *mp;
	register int;
	int unit, op;
	int ubinfo;

	unit = io->i_unit & 7;
	mp = &uda.uda_cmd;
	mp->mscp_flags = 0;
	mp->mscp_bytecnt = mp->mscp_buffer = 0;
	mp->mscp_errlgfl = mp->mscp_copyspd = 0;
	mp->mscp_opcode = 0;
	mp->mscp_cntflgs = 0;
	mp->mscp_lbn = io->i_bn;
	mp->mscp_unit = io->i_unit&7;
	mp->mscp_modifier = command_modifier;
	if (ra_badc) {	 /* allows compare and force error modifiers */
		if(ra_badc == RP_AC) {
			if((mp->mscp_modifier|=ra_badm)&(command_modifier|M_MD_COMP))
				udabdp = 0;
		}
		else {
			if((mp->mscp_modifier = ra_badm) & M_MD_COMP)
				udabdp = 0;
		}
		ra_badm = 0;
	}
	if ((ra_badc != RP_REP) && (ra_badc != RP_AC)){
		rprintd4("\nBefore map\n0x%x 0x%x\n",*(int *)(io->i_ma),*(int *)((int)io->i_ma + 4));
		if (!bda_flg)
			ubinfo = ubasetup(io,udabdp ? 1 : 0);
	}
	udabdp = 1;
	if ((ra_badc == RP_REP) || (ra_badc == RP_AC)) {
		if (ra_badc == RP_AC)
			mp->mscp_bytecnt = io->i_cc;
		else
			mp->mscp_bytecnt = ra_rbn;
		mp->mscp_buffer = 0;
	}
	else {
		mp->mscp_bytecnt = io->i_cc;
		mp->mscp_buffer = (bda_flg? (long)io->i_ma: (ubinfo & 0x3fffff) | (((ubinfo>>28)&0xf)<<24));
	}
	if (ra_badc == RP_REP)
		op = M_OP_REPLC;
	else if(ra_badc == RP_AC)
		op = M_OP_ACCES;
	else
		op = (func == READ ? M_OP_READ : M_OP_WRITE);

#ifdef DEBUG
	iob_sav = *io;	/* debug */
#endif DEBUG
	if ((mp = udcmd(op))== 0) {
		if (ra_badc == 0) {
			printf("\n%s unit %d disk error: ",
				ra_dct, io->i_unit&7);
			printf("endcode = 0x%x flags = 0x%x status = 0x%x\n",
				ra_ecode, ra_eflags,
				ra_stat);
			if (!bda_flg)
				ubafree(io, ubinfo);
		}
		else {
			rprintd4("ra: I/O error\n");
			if ((op == M_OP_READ) || (op == M_OP_WRITE)){
				rprintd5("\nBefore purge\n0x%x 0x%x",
				*(int *)(io->i_ma),*(int *)((int)io->i_ma + 4));

				if (!bda_flg)
					ubafree(io, ubinfo);
				rprintd5("\nAfter purge\n0x%x 0x%x",
				*(int *)(io->i_ma),*(int *)((int)io->i_ma + 4));
			}
			mp = & uda.uda_rsp;
			ra_badc = 0;
			if(((ra_ctid >> 4) & 0x1f) == RQDX1)
				return((mp->mscp_bytecnt == 0) ? 0 :(mp->mscp_bytecnt - 1));
			else
				return(mp->mscp_bytecnt);
		}
		ra_badc = 0;
		return (-1);
	}
#ifdef DEBUG
	iobcompare(&iob_sav, io);	/* debug */
#endif DEBUG
	if ((ra_badc != RP_REP) && (ra_badc != RP_AC))
		if (!bda_flg)
			ubafree(io, ubinfo);
	ra_badc = 0;
	return((op == M_OP_REPLC) ? io->i_cc : mp->mscp_bytecnt);
}

/*ARGSUSED*/
raioctl(io, cmd, arg)
	struct iob *io;
	int cmd;
	caddr_t arg;
{

	return (ECMD);
#ifdef DEBUG
}

/*
 *	Compare fields in saved and real iob structures.
 */

iobcompare(iob_savp, iop)
struct iob *iob_savp;
struct iob *iop;

{
	if(iob_savp->i_flgs != iop->i_flgs)
		printf("iobcompare: i_flgs was 0x%x, is 0x%x\n",
			iob_savp->i_flgs, iop->i_flgs);
	if(iob_savp->i_unit != iop->i_unit)
		printf("iobcompare: i_unit was 0x%x, is 0x%x\n",
			iob_savp->i_unit, iop->i_unit);
	if(iob_savp->i_boff != iop->i_boff)
		printf("iobcompare: i_boff was 0x%x, is 0x%x\n",
			iob_savp->i_boff, iop->i_boff);
	if(iob_savp->i_cyloff != iop->i_cyloff)
		printf("iobcompare: i_cyloff was 0x%x, is 0x%x\n",
			iob_savp->i_cyloff, iop->i_cyloff);
	if(iob_savp->i_offset != iop->i_offset)
		printf("iobcompare: i_offset was 0x%x, is 0x%x\n",
			iob_savp->i_offset, iop->i_offset);
	if(iob_savp->i_bn != iop->i_bn)
		printf("iobcompare: i_bn was 0x%x, is 0x%x\n",
			iob_savp->i_bn, iop->i_bn);
	if(iob_savp->i_cc != iop->i_cc)
		printf("iobcompare: i_cc was 0x%x, is 0x%x\n",
			iob_savp->i_cc, iop->i_cc);
	if(iob_savp->i_error != iop->i_error)
		printf("iobcompare: i_error was 0x%x, is 0x%x\n",
			iob_savp->i_error, iop->i_error);
	if(iob_savp->i_errcnt != iop->i_errcnt)
		printf("iobcompare: i_errcnt was 0x%x, is 0x%x\n",
			iob_savp->i_errcnt, iop->i_errcnt);
	if(iob_savp->i_errblk != iop->i_errblk)
		printf("iobcompare: i_errblk was 0x%x, is 0x%x\n",
			iob_savp->i_errblk, iop->i_errblk);
#endif DEBUG
}

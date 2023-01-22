#ifndef lint
static char *sccsid = "@(#)biinit.c	1.37	ULTRIX	10/23/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984,1986 by			*
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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/* ------------------------------------------------------------------------
 * Modification History:
 *
 * 6-Aug-86   -- jaw	bi device autoconf wasn't setting ui_dk to -1.  This
 *			caused iostat and vmstat programs to have bad headers.
 *			Added routinues to prevent errors while sizer is 
 *			running.
 *
 * 10-Jul-86   -- jaw	added adpt/nexus to ioctl
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 14-May-86 -- pmk	Changed where binumber gets logged, now in LSUBID
 *
 * 23-Apr-86 -- pmk 	Changed logbua() to logbigen()
 *
 * 18-Apr-86 -- jaw	hooks for nmi faults and fixes to bierrors.
 *
 * 16-Apr-86 -- darrell
 *	badaddr is now called via the macro BADADDR.
 *
 * 09-Apr-86 -- lp
 *	Couple of changes in config_cont for bvp nodes.
 *
 * 04-Apr-86 -- afd
 *	Added bidev_vec routine for setting up BI device interrupt vectors.
 *
 * 02-Apr-86 -- jaw  Fix bi autoconf for mutilple bi's.
 *
 * 09-Mar-86  darrell -- pointer to cpusw structure now passed in.
 *	No longer use a global reference.
 *
 * 05-Mar-86 -- jaw  VAXBI device and controller config code added.
 *		     todr code put in cpusw.
 *
 * 19-Feb-86 -- jrs
 *	Removed ref. to TODR as 8800 doesn't have one.  Also fix check
 *	for multiple bi errs to handle correctly before clock is set.
 *
 * 18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 *	12-Feb-86 -- pmk   Added errlogging of bi & bua errors
 *	
 * 	04-feb-86 -- jaw   get rid of biic.h.
 *
 *	06-Dec-85 -- jaw   infinite retry loop fix.
 *
 *	11-Nov-85 -- jaw   fix so we won't try to recover twice from a 
 *                         from a bi error.
 *
 *	16-Oct-85 -- jaw   bug fixes for BI error handling.
 *
 *	27-Sep-85 -- jaw   more print message fixes.
 *
 *	11-Sep-85 -- jaw   put in offical names of BI devices.
 *
 *	04-Sep-85 -- jaw   Add bi error interrupt code.
 *
 *	08-Aug-85 -- lp	   Add rev number to found nodes.
 *
 *	26-Jul-85 -- jaw   fixup BI vector allocation.
 *
 *	27-Jun-85 -- jaw   memory nodes always come-up broke.
 *
 * 	19-Jun-85 -- jaw   VAX8200 name change.
 *
 *	05 Jun 85 -- jaw   code clean-up and replace badwrite with bisst.
 *
 * 	20 Mar 85 -- jaw   add support for VAX 8200
 *
 *
 * ------------------------------------------------------------------------
 */


#include "../machine/pte.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/time.h"
#include "../h/kernel.h"
#include "../h/errlog.h"
#include "../h/cpuconf.h"
#include "../h/dk.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vax/nexus.h"
#include "../vax/scb.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vaxbi/bireg.h"
#include "../vaxbi/buareg.h"
#include "../vaxbi/bimemreg.h"
extern int numuba;
extern int nbicpus;
extern int nbitypes;
extern int dkn;		/* number of iostat dk numbers assigned so far */
extern int ignorebi;
extern struct bisw bisw[];
extern struct bidata bidata[];
extern int catcher[256];
extern int nNUBA;

#define BIMEMTEST 0x00007f00

int Xbi0err(), Xbi1err(),Xbi2err(),Xbi3err();
int bierr_dispatch[] = {(int)Xbi0err,(int)Xbi1err,(int)Xbi2err, (int)Xbi3err};

bisetvec(binumber)
int binumber;
{

	*(bidata[binumber].bivec_page+(BIEINT_BIVEC/4))=
				scbentry(bierr_dispatch[binumber],SCB_ISTACK);

}

probebi(cpup, binumber)
struct	cpusw	*cpup;
int 	binumber;
{
	register struct bi_nodespace *nxv;
	register struct bi_nodespace *nxp;
	register struct bisw *pbisw;
	int foo;
	register int i,vec;
	int broke;
	int binode;
	int alive;

	/* set VAXBI alive in adpter struct */
	config_set_alive("vaxbi",binumber);

	/* ingnore BI errors when doing autoconf */
	ignorebi = 1;
	
	bisetvec(binumber);

	/* bit mask of active nodes */
	bidata[binumber].binodes_alive = 0;
	nxv =  bidata[binumber].bivirt;
	nxp =  bidata[binumber].biphys;

	/* don't initialize first half of page on 8200 */
	if (cpu == VAX_8200) i=64;
	else i=0;

	/* initialize SCB to catcher */
	for ( ; i < 128; i++)
			*(SCB_BI_ADDR(binumber) + i) =
			    scbentry(&catcher[i*2], SCB_ISTACK);

	for(binode=0; binode < NBINODES; binode++, nxv++ ,nxp++) {
			
	    /* map bi node space */
	    nxaccess(nxp, Nexmap[(binode+(binumber*16))], BINODE_SIZE);
		
	    /* bi node alive ??? */
	    if (BADADDR((caddr_t) nxv,sizeof(long))) continue;

	    /* find active device in bi list */
	    for (pbisw = bisw ; pbisw->bi_type ; pbisw++) {	
		if (pbisw->bi_type == (short)(nxv->biic.biic_typ)) {

		    bidata[binumber].binodes_alive |= (1 << binode);
	    	    bidata[binumber].bierr[binode].pbisw= pbisw;
				
		    if (pbisw->bi_flags&BIF_SST) 
				foo=bisst(&nxv->biic.biic_ctrl);
				
		    if ((nxv->biic.biic_typ & BIMEMTEST) == 0)
			broke=((struct bimem *)nxv)->bimem_csr1&BICTRL_BROKE;
		    else broke = nxv->biic.biic_ctrl & BICTRL_BROKE;
					

		    /* set BI interupt dest to be cpu */
		    if (pbisw->bi_flags & BIF_SET_HEIE) 
			   nxv->biic.biic_int_dst=bidata[binumber].biintr_dst;

		    alive=0;	
		    if (pbisw->bi_flags&BIF_DEVICE) 
			alive |= bi_config_dev(nxv,nxp,cpup,binumber,binode);

		    if (pbisw->bi_flags&BIF_CONTROLLER) 
			alive |= bi_config_cont(nxv,nxp,cpup,binumber,binode);

		    if (pbisw->bi_flags&BIF_ADAPTER){ 
		    	(**pbisw->probes)(nxv,nxp,cpup,binumber,binode);
			alive=1;
		    }
		    if (pbisw->bi_flags&BIF_NOCONF) {
			printf ("%s at vaxbi%x node %d",
				pbisw->bi_name,binumber,binode);
	    		if (broke == 0)
				printf("\n");
	    		else
				printf(" is broken, continuing!\n");
		    	(**pbisw->probes)(nxv,nxp,cpup,binumber,binode);
			alive=1;
		    }
		    if (alive==0) binotconf(nxv,nxp,cpup,binumber,binode);

		    break;
		} 
	    }
	    if (pbisw->bi_type == 0) 
		printf ("vaxbi%x node %d, unsupported device type 0x%x\n",
			binumber,binode, (short) nxv->biic.biic_typ);
	}
	/* set up error vec offset */
	vec = (SCB_BI_OFFSET(binumber))+BIEINT_BIVEC;

	/* set up BI hard error interrupt for proper nodes */	
	nxv =  bidata[binumber].bivirt;
	for (binode = 0; binode < NBINODES; binode++,nxv++) {
		pbisw = bidata[binumber].bierr[binode].pbisw;

		if ((bidata[binumber].binodes_alive & (1 << binode)) &&
			(pbisw->bi_flags & BIF_SET_HEIE)) {

			/*clear out any remaining errors in cpu BIIC*/
	
			nxv->biic.biic_err = nxv->biic.biic_err; 
		 	nxv->biic.biic_err_int = (nxv->biic.biic_err_int &
				~(BIEINT_VECTOR|BIEINT_LEVEL|BIEINT_FORCE))
	 		 	|(BIEINT_5LEVEL|vec);

		    	nxv->biic.biic_ctrl = (BICTRL_HEIE) | 
					(nxv->biic.biic_ctrl&~(BICTRL_BROKE));
		}
	}
	ignorebi = 0;
}

struct uba_ctlr *bifindum(); 

bi_config_cont(nxv,nxp,cpup,binumber,binode)
struct bi_nodespace *nxv;
char *nxp;
struct cpusw *cpup;
int 	binumber;
int 	binode;
{
	register struct uba_device *ui;
	register struct uba_ctlr *um;
	register int (**biprobe)();
	register struct uba_driver *udp;
	register struct bisw *pbisw = bidata[binumber].bierr[binode].pbisw;
	int level;
	int (**ivec)();
	int found = 0;

	
	for ( biprobe =  pbisw->probes; *biprobe; biprobe++) {
		if ((um = bifindum(binumber,binode,biprobe)) == 0)
			if ((um = bifindum(binumber,'?',biprobe))==0) 
				if ((um = bifindum('?','?',biprobe))==0) 
					continue;
		found =1;
		if (((*biprobe)(nxv,um->um_ctlr))
			== 0){
			continue;
		}
		um->um_adpt = binumber;
		um->um_nexus = binode;
		um->um_alive = 1;
		um->um_addr = (char *)nxv;
		udp = um->um_driver;
		udp->ud_minfo[um->um_ctlr] = um;
		config_fillin(um);
		printf("\n");

		level = LEVEL15;
		bicon_vec(binumber, binode, level, um);
		for (ui = ubdinit; ui->ui_driver; ui++) {
			if (ui->ui_driver != udp || ui->ui_alive ||
			    ui->ui_ctlr != um->um_ctlr && ui->ui_ctlr != '?')
				continue;
			if ((*udp->ud_slave)(ui, nxv)) {
				ui->ui_alive = 1;
				ui->ui_ctlr = um->um_ctlr;
				ui->ui_addr = (char *)nxv;
				ui->ui_ubanum = um->um_ubanum;
				ui->ui_hd = um->um_hd;
				ui->ui_physaddr = nxp;
				ui->ui_adpt = binumber;
				ui->ui_nexus = binode;

				if (ui->ui_dk && dkn < DK_NDRIVE)
					ui->ui_dk = dkn++;
				else
					ui->ui_dk = -1;
				ui->ui_mi = um;
				/* ui_type comes from driver */
				udp->ud_dinfo[ui->ui_unit] = ui;
				if(ui->ui_slave >= 0)
				printf("%s%d at %s%d slave %d\n",
				    ui->ui_devname, ui->ui_unit,
				    udp->ud_mname, um->um_ctlr, ui->ui_slave);
				else
				printf("%s%d at %s%d\n",
				    ui->ui_devname, ui->ui_unit,
				    udp->ud_mname, um->um_ctlr);

				(*udp->ud_attach)(ui);
			}
	    }


	}
	return(found);
}


struct uba_ctlr *bifindum(binumber,binode,biprobe) 
register int binumber;
register int binode;
register int (**biprobe)();
{
	struct uba_driver *udp;
	register struct uba_ctlr *um;


	for (um=ubminit; udp =um->um_driver; um++) {
	
		if ((udp->ud_probe != *biprobe) ||
		    (um->um_adpt != binumber) ||
		    (um->um_nexus != binode) ||
		    (um->um_alive) ||
		    ((um->um_ubanum >=0) && ((um->um_ubanum < nNUBA)
		        		 || (um->um_ubanum == '?'))))continue;

		return(um);
	}
	return(0);

}		

struct uba_device *bifindui(); 

bi_config_dev(nxv,nxp,cpup,binumber,binode) 
struct bi_nodespace *nxv;
char *nxp;
struct cpusw *cpup;
register int 	binumber;
register int 	binode;
{
	register struct uba_device *ui;
	register int found = 0;
	register int (**biprobe)();
	register struct bisw *pbisw = bidata[binumber].bierr[binode].pbisw;

	
	for ( biprobe = pbisw->probes; *biprobe; biprobe++) {
		
		if ((ui = bifindui(binumber,binode,biprobe)) == 0)
			if ((ui = bifindui(binumber,'?',biprobe))==0) 
				if ((ui = bifindui('?','?',biprobe))==0) 
					continue;
		found =1;
		if (((*biprobe)(nxv,nxp,cpup,binumber,binode,ui))
			== 0){
			continue;
		}

 		ui->ui_adpt = binumber;
		ui->ui_nexus = binode;
		config_fillin(ui);
		printf("\n");
		ui->ui_dk = -1;
		ui->ui_alive = 1;
		ui->ui_addr=(char *)nxv;
		ui->ui_driver->ud_dinfo[ui->ui_unit] = ui;
		(*ui->ui_driver->ud_attach)(ui);

	}
	return(found);

}

struct uba_device *bifindui(binumber,binode,biprobe) 
register int binumber;
register int binode;
register int (**biprobe)();
{
	struct uba_driver *udp;
	register struct uba_device *ui;


	for (ui=ubdinit; udp =ui->ui_driver; ui++) {
	
		if ((udp->ud_probe != *biprobe) ||
		    (ui->ui_adpt != binumber) ||
		    (ui->ui_nexus != binode) ||
		    (ui->ui_alive) ||
		    (ui->ui_slave != -1)) continue;

		return(ui);
	}
	return(0);

}		


#define BI_THRESHOLD 2 	/* 2 sec */
int nignorebi = 0;

unsigned lastbierr = 0;

bierrors(binumber,pcpsl)
int binumber;
int *pcpsl;
{
	register struct bi_regs *biregp;
	register struct bidata *bid;
	register struct el_rec *elrp;
	register struct el_bier *elbierp;
	register struct bi_nodespace *nxv;
	int node;
	int nodecnt = 0;
	int *intp;
	struct bisw *pbisw;
	int recover=1;
	int priority;


	bid = &bidata[binumber];
	bid->bi_err_cnt++;

	if (ignorebi) {
		if (nignorebi++ > 0x10000) panic("Too many VAXBI errors");
		return;
	}

	if ((time.tv_sec - bid->bilast_err_time) < BI_THRESHOLD){
		priority = EL_PRISEVERE;
		recover =0;
	} else 
		priority = EL_PRIHIGH;

	bid->bilast_err_time = time.tv_sec;

	/*count number of active BI nodes */
	for (node = 0; node < NBINODES; node++) {
	    if (bid->binodes_alive & (1 << node))
		nodecnt++;
	}

	/* get error log packet and fill in header. */
	elrp = ealloc(sizeof(struct bi_regs) * nodecnt + 12, priority);

	if (elrp != NULL) {
	    LSUBID(elrp,ELCT_BUS,ELBUS_BIER,EL_UNDEF,binumber,EL_UNDEF,EL_UNDEF);
	    elbierp = &elrp->el_body.elbier;
	    biregp = elbierp->biregs;
	} 

	if (recover == 0) cprintf(" hard error VAXBI%d\n",binumber);

	nxv =  bid->bivirt;
	for(node=0; node < NBINODES; node++,nxv++) {

	    if (bid->binodes_alive & (1<<node)) { 
	    		pbisw = bid->bierr[node].pbisw;
			bid->bierr[node].bierr1 = bid->bierr[node].bierr;
			bid->bierr[node].bierr=nxv->biic.biic_err&(~BIERR_UPEN);
			if (recover == 0) {
				cprintf("%s at node %d error %b ",
				pbisw->bi_name, node, bid->bierr[node].bierr,
				BIERR_BITS);
			}
			/* reset error bits on proper nodes */
			if (pbisw->bi_flags & BIF_SET_HEIE)
				nxv->biic.biic_err = nxv->biic.biic_err;
			
			if (  (pbisw->bi_type == BI_BUA)
			   || (pbisw->bi_type == BI_BLA)) {
				/* log bua csr if an error bit is set */
				if (((struct bua_regs *)nxv)->bua_ctrl &
				   BUACR_MASK) {
				   	bid->bierr[node].bierr = 
						((struct bua_regs *)nxv)->bua_ctrl;
					logbigen(binumber,pcpsl,nxv);
				}
				if (recover == 0) cprintf(" cr %b ",
				   (((struct bua_regs *)nxv)->bua_ctrl),
				   BUAERR_BITS);
			}
		        if (recover == 0) cprintf("\n");

			if (elrp != NULL) {
				/* log the node status */
				biregp->bi_typ = nxv->biic.biic_typ;
				biregp->bi_ctrl = nxv->biic.biic_ctrl;
				biregp->bi_err = nxv->biic.biic_err;
				biregp->bi_err_int = nxv->biic.biic_err_int;
				biregp->bi_int_dst = nxv->biic.biic_int_dst;
				biregp++;
			}
		}
	}

	/* finish logging error */
	if (elrp != NULL) {
		intp = (int *)biregp;
		elbierp->bier_nument = (short)nodecnt;
		*intp++ = *pcpsl++;
		*intp = *pcpsl; 
		EVALID(elrp);
	}

	if (recover==0)  panic("VAXBI error");

	/* loop back through nodes and call reset routines if preset */
	nxv =  bid->bivirt;
	for(node=0; node < NBINODES; node++,nxv++) {
		if (bid->binodes_alive & (1<<node)) { 
			pbisw= bid->bierr[node].pbisw;
			(*(pbisw->bi_reset))(binumber,node,nxv);
		}
	}

}

logbigen(binum,pcpsl,nxv)
int binum;
int *pcpsl;
register struct bi_nodespace *nxv;
{
	register int class;
	register int type;
	register struct el_rec *elrp;
	register struct el_bigen *elbp;
	
	elrp = ealloc(sizeof(struct el_bigen), EL_PRIHIGH);
	if (elrp != NULL) {
	    elbp = &elrp->el_body.elbigen;
	    elbp->bigen_dev = nxv->biic.biic_typ;
	    elbp->bigen_bicsr = nxv->biic.biic_ctrl;
	    elbp->bigen_ber = nxv->biic.biic_err;
	    elbp->bigen_csr = ((struct bua_regs *)nxv)->bua_ctrl;
	    switch (elbp->bigen_dev & BITYP_TYPE) {
		case BI_BUA:
		    class = ELCT_ADPTR;
		    type = ELADP_BUA;
		    break;
		case BI_BLA:
		    class = ELCT_DCNTL;
		    type = ELBI_BLA;
		    break;
		default:
		    class = EL_UNDEF;
		    type = EL_UNDEF;
		    break;
	    }
	    if (type == ELADP_BUA)
	        elbp->bigen_fubar = 
			((struct bua_regs *)nxv)->bua_fubar & BUAFUBAR;
	    else
		elbp->bigen_fubar = 0;
	    elbp->bigen_pc = *pcpsl++;
	    elbp->bigen_psl = *pcpsl;
	    LSUBID(elrp,class,type,EL_UNDEF,binum,EL_UNDEF,EL_UNDEF);
	    EVALID(elrp);
	}
}

extern int nNVAXBI;

biclrint(){

int binumber,binode;
struct bisw *pbisw;
struct bi_nodespace *nxv;

    	for (binumber= 0; binumber<nNVAXBI; binumber++) {
		nxv = bidata[binumber].bivirt;
		if (bidata[binumber].binodes_alive) 
		    for(binode=0; binode < 16; binode++,nxv++){

			pbisw = bidata[binumber].bierr[binode].pbisw;

			if ((bidata[binumber].binodes_alive & 1<<binode) &&
				(pbisw->bi_flags & BIF_SET_HEIE)){
				nxv->biic.biic_ctrl &= ~(BICTRL_HEIE); 
				if ((pbisw->bi_type == BI_BUA) ||
				   (pbisw->bi_type == BI_BLA)) 
					((struct bua_regs *)nxv)->bua_ctrl &=
						~(BUACR_BUAEIE);
	
			}

		    }
	}
}
bisetint(){

int binumber,binode;
struct bisw *pbisw;
struct bi_nodespace *nxv;

    	for (binumber= 0; binumber<nNVAXBI; binumber++) {
	
		nxv = bidata[binumber].bivirt;
		if (bidata[binumber].binodes_alive) 
		    for(binode=0; binode < 16; binode++,nxv++){

			pbisw = bidata[binumber].bierr[binode].pbisw;

			if ((bidata[binumber].binodes_alive & 1<<binode) &&
				(pbisw->bi_flags & BIF_SET_HEIE)){
				nxv->biic.biic_err = nxv->biic.biic_err;
				nxv->biic.biic_ctrl |= (BICTRL_HEIE); 
				
				if ((pbisw->bi_type == BI_BUA) ||
				   (pbisw->bi_type == BI_BLA)){ 
					((struct bua_regs *)nxv)->bua_ctrl =
					   ((struct bua_regs *)nxv)->bua_ctrl;
					((struct bua_regs *)nxv)->bua_ctrl |=
						(BUACR_BUAEIE);
				}
			}

		    }
	}
}

/*
 * bidev_vec(): To set up BI device interrupt vectors.
 * It is called with 4 parameters:
 *	binum:	the BI number that the device is on
 *	binode: the BI node number of the device
 *	level:  the offset corresponding to the interrupt priority level
 *		to start at.  See ../vaxbi/bireg.h: LEVEL{14,15,16,17}.
 *	ui:	the device structure (for names of interrupt routines)
 */

bidev_vec(binum, binode, level, ui)
	int binum, binode, level;
	struct uba_device *ui;
{
	register int (**ivec)();
	register int (**addr)();	/* double indirection neccessary to keep
				   	   the C compiler happy */
	for (ivec = ui->ui_intr; *ivec; ivec++) {
		addr = (int (**)())(SCB_BI_VEC_ADDR(binum,binode,level));
		*addr = scbentry(*ivec,SCB_ISTACK);
		level += BIVECSIZE;
	}
}

bicon_vec(binum, binode, level, um)
	int binum, binode, level;
	struct uba_ctlr *um;
{
	register int (**ivec)();
	register int (**addr)();	/* double indirection neccessary to keep
				   	   the C compiler happy */
	for (ivec = um->um_intr; *ivec; ivec++) {
		addr = (int (**)())(SCB_BI_VEC_ADDR(binum,binode,level));
		*addr = scbentry(*ivec,SCB_ISTACK);
		level += BIVECSIZE;
	}
}


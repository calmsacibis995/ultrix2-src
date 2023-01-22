#ifndef lint
static char *sccsid = "@(#)bbr.c	1.6	ULTRIX	10/3/86";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************
 *									
 *									
 *	Facility:  Dynamic Bad Block Replacement
 *									
 *	Abstract:  This module implements the BBR algorithm as
 *		   passed at the Feb. 1986 DSA Woods meeting.
 *		   
 *									
 *	Creator:	Mark Parenti	Creation Date:	May 6, 1986
 *
 *	Modification history:
 *
 *	14-Aug-86 - map
 *		Change handling of unexpected mscp responses. Print message
 *		and return a value of 1. The uda driver will then pass
 *		the packet on to it's response processing.
 *		Clear bbr_mods field before issuing multi_read in step1_multi.
 *
 *	24-Jun-86 - map
 *		Fixes from Colorado certification.
 *
 * 	13-Jun-86   -- jaw 	fix to uba reset and drivers.
 *
 * 	10-Jun-86 - map
 *		Initial version.
 */
/**/

/*
 *	Include files
 */
#include "../machine/pte.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/dk.h"
#include "../h/cmap.h"
#include "../h/uio.h"
#include "../h/ioctl.h"
#include "../ufs/fs.h"
#include "../h/dkio.h"
#include "../h/devio.h"

#include "../vax/cpu.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"
#include "../vax/mtpr.h"
#include "../vaxbi/buareg.h"


#define NRSPL2	3		/* log2 number of response packets */
#define NCMDL2	3		/* log2 number of command packets */
#define NRSP	(1<<NRSPL2)
#define NCMD	(1<<NCMDL2)

#include "../vaxuba/udareg.h"
#include "../vax/mscp.h"
#include "../h/bbr.h"

/* MSCP protocol storage area */
struct uda {
	struct udaca	uda_ca; 	/* Communications area		*/
	struct mscp	uda_rsp[NRSP];	/* Response packets		*/
	struct mscp	uda_cmd[NCMD];	/* Command packets		*/
};

extern struct uda_softc uda_softc[];
extern struct uda uda[];

/* END OF STUFF WHICH SHOULD BE READ IN PER DISK */
extern	struct	buf udutab[];		/* Drive queue */
extern struct  buf udwtab[];		   /* I/O wait queue, per controller */
extern int     udamicro[];	   /* to store microcode level */
extern struct ra_info ra_info[];
extern struct	rct_search rctsearch[];

/*
 *	Various definitions
 */

int	bbr_res_need = 0;
/* 
 * External definitions
 */

extern	BBR_STAB	bbr_stab[BBR_MAX_STEP + 1][BBR_MAX_SUBSTEP + 1];
extern	struct	buf	*geteblk();
extern	struct	mscp	*udgetcp();
extern	struct	timeval	time;
/*
extern	u_char	*kmemall();
extern	void	kmemfree();
#define	KM_CLRSG	1
*/
#define b_ubinfo        b_resid         /* Unibus mapping info, per buffer */
#define PHYS(addr)	((long) \
		((long)ptob(Sysmap[btop((long)(addr)&0x7fffffff)].pg_pfnum)  \
			| (long)( PGOFSET & (long)(addr) ) ) )

int     bbrdebug = 0;
#define printd  if (bbrdebug) printf


#define	BBR_DEBUG
#ifdef	BBR_DEBUG
/*
 *	Trace tables
 */
char	*trace_step[] = {
		"Invalid",
		"Step 1 ",
		"Step 3 ",
		"Step 4 ",
		"Step 5 ",
		"Step 6 ",
		"Step 7 ",
		"Step 7B ",
		"Step 7C ",
		"Step 8 ",
		"Step 9 ",
		"Step 10 ",
		"Step 11 ",
		"Step 12 ",
		"Step 12B ",
		"Step 12C ",
		"Step 13 ",
		"Step 14 ",
		"Step 15 ",
		"Step 16 ",
		"Step 17 ",
		"Step 18 ",
		"Multi-read ",
		"Multi-write ",
		"RCT search "
};

char	*trace_substep[] = {
		"Invalid",
		"Sub-0",
		"Sub-1",
		"Read_end",
		"Search_end",
		"Replace_end",
		"STUNT_end",
		"Write_end",
		"Read_end_2",
		"Write_end_2",
		"BBR_lin_read",
		"BBR linear"
};
#endif	BBR_DEBUG

/**/
/*
 *
 *
 *	Name:		bbr_dispatch
 *	
 *	Abstract:	Main dispatch routine for BBR state machine
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	bbr_dispatch( ri )
struct	ra_info		*ri;
{
struct	uda_softc	*sc;
struct	uba_device	*ui = ri->ui;
struct	uba_ctlr 	*um = ui->ui_mi;

	sc = &uda_softc[um->um_ctlr];
	for(;;) {
#ifdef	BBR_DEBUG
		printd("Dispatch: %s, %s\n", trace_step[ri->bbr_step],
			trace_substep[ri->bbr_substep]);
		printd("ri = %x\n", ri);
		if(bbrdebug>0)DELAY(10000);
#endif	BBR_DEBUG

		sc->sc_progress = time;/* log progess */

		if ( (*bbr_stab[ri->bbr_step][ri->bbr_substep].action)( ri )
			 == NULL ) break;
	}
		
}
/**/
/*
 *
 *
 *	Name:		bbr_start
 *	
 *	Abstract:	Start of BBR algorithm. Determine reason for
 *			invocation.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	bbr_start( ui, lbn, force )
struct	uba_device	*ui;
int			lbn, force;
{
struct	ra_info		*ri = &ra_info[ui->ui_unit];
struct	rct_search	*ss = &rctsearch[ui->ui_unit];
struct	uda_softc	*sc;
struct	uba_ctlr 	*um = ui->ui_mi;

	sc = &uda_softc[um->um_ctlr];

	if( ri->bbr_lock )	/* If already in progress then just requeue */
		return(BBR_RET_LOCK); /* Will be detected next time around  */
	if( ri->raunitflgs & M_UF_REPLC ) /* Return if controller BBR  */
		return(BBR_RET_CTLR);

	ri->bbr_lock = 1;	/* Lock out access to unit while BBR in progress */
	sc->sc_bbr_ip++;	/* Increment count of BBR's on this 	    */
				/* controller. Used to force retention of   */
				/* the buffered data path (if there is one) */
	ri->search = ss;
	ri->ui = ui;	/* Save ui pointer	*/
	ri->bbr_ip = BBR_IN_PROGRESS;
	if( lbn != -1 ) {	/* Invoked because Bad Block Found	*/

		if( ri->raunitflgs & (M_UF_WRTPH | M_UF_WRTPS) ){ /* Can't do BBR   */
								 /* if write protect*/
			mprintf("\tunit %d, ", ui->ui_slave);
			mprintf("Cannot start replacement - disk write protected\n");
			printf("ra%d: Cannot start replacement of LBN %d\n"
			, ui->ui_unit, lbn);
			printf("\tUnit is write-protected\n");
			sc->sc_bbr_ip--;	/* One less BBR on this ctlr*/
			ri->bbr_lock = 0;	/* Unlock unit		*/
			ri->bbr_ip = BBR_FAILURE;
			wakeup( ri );	/* Wakeup anyone waiting	*/
			return(BBR_RET_WRTPRT);
		}
	/*
 	 * If force is not 0 then we are being called from the user
 	 * program and want to force the lbn to be replaced.
	 */
		if( force == 0) 
			ri->bbr_reason = BAD_BLOCK_FOUND;
		else
			ri->bbr_reason = FORCE_REPLACE;	
		ri->bad_LBN = lbn;		/* LBN to replace 	*/
		ri->bbr_step = BBR_STEP_3;	/* Go to step 3		*/
		ri->bbr_substep = SUB_0;	/* Substep 0		*/
	}
	else {			/* Invoked for ONLINE check		*/
		ri->bbr_step = BBR_STEP_1;	/* Still step 1		*/
		ri->bbr_substep = SUB_0;	/* Multi_read 		*/
	}
	bbr_dispatch( ri );		/* Start up the state machine	*/
	return(BBR_RET_SUCC);	
}
/**/
/*
 *
 *
 *	Name:		bbr_rsp
 *	
 *	Abstract:	Special response processing routine for BBR. If the
 *			command reference number matches the special BBR
 *			command reference number then this routine is called.
 *			This routine saves some state in the ra_info structure
 *			and calls bbr_dispatch to restart the BBR process.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	bbr_rsp( ui, mp )
struct	uba_device	*ui;
struct	mscp		*mp;

{
struct	ra_info		*ri;
struct	uba_ctlr 	*um = ui->ui_mi;
struct	buf		*bp;
u_long			st;
struct	uda_softc	*sc;

#ifdef	BBR_DEBUG
	printd("BBR: Response received - mp = %x\n", mp);
	if(bbrdebug>0)DELAY(10000);
#endif	BBR_DEBUG
	ri = &ra_info[ui->ui_unit];
	bp = ri->cur_buf;
	sc = &uda_softc[um->um_ctlr];
	sc->sc_progress = time;/* log progess */
	ri->status = mp->mscp_status;
	ri->mscp_flags = mp->mscp_flags;
	st = mp->mscp_status & M_ST_MASK;
#ifdef	BBR_DEBUG
	printd("st = %x\n", st);
	if(bbrdebug>0)DELAY(10000);
#endif	BBR_DEBUG
	switch( mp->mscp_opcode ) {

	case  M_OP_READ | M_OP_END:
	case  M_OP_WRITE | M_OP_END:

		ubarelse(um->um_ubanum, (int *)&bp->b_ubinfo);
		if (uba_hd[um->um_ubanum].uba_type & (UBABUA|UBA750))
			ubapurge(um);

		if ( (st == M_ST_OFFLN) || (st == M_ST_AVLBL ) ) {
			mscp_cmd(0, M_OP_ONLIN, ri, 0);
			return( NULL );
		}
		break;

	case	M_OP_ONLIN | M_OP_END:

		if ( (st == M_ST_OFFLN) || (st == M_ST_AVLBL ) ) {
			bbr_error( ri, BBR_ERR_OFFLN);
			ri->bbr_lock = 0;
			ri->bbr_ip = BBR_FAILURE;
			sc->sc_bbr_ip--; /* One less BBR in progress	*/
			wakeup( ri );	/* Wakeup ONLINE waiter		*/
			return( NULL );
		}
		ri->bbr_step = BBR_STEP_1;
		ri->bbr_substep = SUB_0;
		break;

	case	M_OP_STUNT | M_OP_END:
		ri->raunitflgs = mp->mscp_unitflgs;
		break;

	case	M_OP_REPLC | M_OP_END:
		break;

	default:
		mprintf("BBR: Unexpected Response - opcode = %x\n", mp->mscp_opcode);
		return(1);
		break;
	}
	bbr_dispatch( ri );
	return(NULL);
}
/**/
/*
 *
 *
 *	Name:		step1_multi
 *	
 *	Abstract:	Perform step 1 multi-read of RCT
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step1_multi(ri)
struct	ra_info		*ri;
{
RCT_SECTOR_0		*rct;
struct	buf		*bp;
struct	uba_device	*ui = ri->ui;
struct	uba_ctlr 	*um = ui->ui_mi;
struct	uda_softc	*sc = &uda_softc[um->um_ctlr];


	switch( ri->bbr_substep ) {

	case SUB_0:
		if( ri->rct0_buf == 0 )
			ri->rct0_buf = geteblk(512);
		(void)bzero( ri->rct0_buf->b_un.b_addr, 512 );
		ri->multi_buf = ri->rct0_buf;
		ri->bbr_substep = READ_END;
		ri->bbr_mods = 0;
		multi_read( ri, ri->radsize );/* Start multi_read algorithm */
		return(NULL);
		break;

	case READ_END:
		if ( ri->status != M_ST_SUCC ) {
			bbr_error( ri, BBR_ERR_MREAD );
			ri->bbr_step = BBR_STEP_18;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		rct = (RCT_SECTOR_0 *)ri->rct0_buf->b_un.b_addr;
		ri->bad_LBN = rct->lbn;	/* LBN we are replacing	   */
		if ( rct->flags & RCT_P1 ) {	/* Phase 1 Recovery	   */
			if( rct->flags & RCT_P2 ) {	/* RCT Corrupt	*/
				bbr_error( ri, BBR_ERR_RCT0 );
				ri->bbr_step = BBR_STEP_18;
				ri->bbr_substep = SUB_0;
				return(1);
			}

			ri->bbr_reason = BBR_PHASE_1;
		}
 		else if ( rct->flags & RCT_P2 ) { /* Phase 2 Recovery	   */
			if( rct->flags & RCT_P1 ) {	/* RCT Corrupt	*/
				bbr_error( ri, BBR_ERR_RCT0 );
				ri->bbr_step = BBR_STEP_18;
				ri->bbr_substep = SUB_0;
				return(1);
			}

			ri->bbr_reason = BBR_PHASE_2;
			ri->new_RBN = rct->new_RBN; /* Get RBN to use	   */
			if ( rct->flags & RCT_BR ) {   /* Got a bad RBN	   */
				ri->old_RBN = rct->bad_RBN;
				ri->prev_repl = 1;
			}
		}
		else {			/* No BBR in progress		*/
			ri->bbr_ip = BBR_SUCCESS;
			sc->sc_bbr_ip--; /* One less BBR in progress	*/
			ri->bbr_lock = 0; /* Unlock unit		*/
			wakeup( ri );	/* Wakeup ONLINE waiter		*/
			return (NULL);
		}

		mprintf("ra%d: Interrupted replacement on LBN %d restarted\n",
			ui->ui_unit, ri->bad_LBN);
		ri->bbr_fe = (rct->flags & RCT_FE ? 1 : 0);/* Set forced error */
		ri->bbr_substep = READ_END_2;		/* Read sector 1    */
		if( ri->orig_buf == 0 ) 
			ri->orig_buf = geteblk(512);
		(void)bzero( ri->orig_buf->b_un.b_addr, 512 );
		ri->multi_buf = ri->orig_buf;
		ri->bbr_mods = 0;
		multi_read( ri, (ri->radsize) + 1);
		return( NULL );		
		break;


	case	READ_END_2:

		if ( ri->status != M_ST_SUCC ) {
			bbr_error( ri, BBR_ERR_MREAD );
			ri->bbr_step = BBR_STEP_18;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		ri->bbr_step = BBR_STEP_3;   /* Goto step 3  */
		ri->bbr_substep = SUB_0; /* Substep 0	   */
		break;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step3
 *	
 *	Abstract:	This step dispatches based on the reason BBR
 *			was invoked.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step3( ri )
struct	ra_info		*ri;
{
int	i;

	ri->recurs = 0;		/* Init recursion counter		*/
	switch( ri->bbr_reason) {

	case BAD_BLOCK_FOUND:
	case FORCE_REPLACE:
		
		ri->bbr_step = BBR_STEP_4;	/* Proceed to step 4	*/
		ri->bbr_substep = SUB_0;	/* Substep 0		*/
		break;

	case BBR_PHASE_1:

		ri->bbr_step = BBR_STEP_7;	/* Proceed to step 7	*/
		ri->bbr_substep = SUB_0;	/* Substep 0		*/
		break;

	case BBR_PHASE_2:

		i = ( ri->bad_LBN/ri->lbnstrk ) * ri->rbnstrk;
		if ( i == ri->new_RBN ) 
			ri->is_primary = 1;	/* We are doing a primary */
		else
			ri->is_primary = 0;	/* We are doing a tertiary */
		ri->bbr_step = BBR_STEP_11;	/* Proceed to step 11	*/
		ri->bbr_substep = SUB_0;	/* Substep 0		*/
		break;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step4_start
 *	
 *	Abstract:	This module starts the step 4 process. This step
 *			attempts to read the user's data from the bad
 *			block and stores it away for later use. The algorithm
 *			makes 4 attempts at reading the suspect block. If the
 *			data is not successfully read the FE flag is set and
 *			the best guess data is used.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step4_start( ri )
struct	ra_info		*ri;
{

	if( ri->orig_buf == 0 ) 
		ri->orig_buf = geteblk(512);
	(void)bzero( ri->orig_buf->b_un.b_addr, 512 );
	ri->bbr_cnt = 0;	/* Zero read counter		*/
	ri->bbr_fe = 0;		/* Zero forced error indicator	*/
	ri->suc = 0;		/* Zero success indicator	*/
	ri->bbr_mods = 0;		/* No modifiers			*/
	ri->bbr_substep = SUB_1; /* Next state is substep 1	*/
	mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->orig_buf ); /* Issue read to bad block */
	return( NULL );
}
/**/
/*
 *
 *
 *	Name:		step4_cont
 *	
 *	Abstract:	This routine analyzes the results of step 4 read
 *			attempts.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step4_cont( ri )
struct	ra_info		*ri;
{

	if ( (ri->status == M_ST_SUCC) || (ri->status == M_DT_FEM) )
			ri->suc = 1;
	if ( ri->status == M_DT_FEM)
			ri->bbr_fe = 1;
	ri->bbr_cnt++;

	if ( (ri->bbr_cnt == STEP4_MAX_READ) || ri->suc) {
		if( ri->suc == 0 )
			ri->bbr_fe = 1; /* All reads failed - data invalid */
		ri->bbr_step = BBR_STEP_5; /* Either got it or never will */
		ri->bbr_substep = SUB_0;   /* Substep 0			  */
		return(1);
	}
	mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->orig_buf );
	return(NULL);
}
/**/
/*
 *
 *
 *	Name:		step5_start
 *	
 *	Abstract:	This routine starts the multi-write to place the
 *			user data into sector 1 of the RCT.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step5_start( ri )
struct	ra_info		*ri;
{
RCT_SECTOR_0		*rct;

	switch( ri->bbr_substep ) {

	case SUB_0:
		ri->multi_buf = ri->orig_buf;	/* Write step 4 data	*/
		ri->bbr_mods = 0;
		ri->bbr_substep = WRITE_END;
		multi_write( ri, (ri->radsize)+1 ); /* Write to saved data to RCT sector 1 */
		return(NULL);
		break;

	case WRITE_END:
		if( ri->status == M_ST_SUCC ) {	/* Multi-write succeeded */
			ri->bbr_step = BBR_STEP_6;
			ri->bbr_substep = SUB_0;
		}
		else {
			bbr_error( ri, BBR_ERR_MWRITE );
			ri->bbr_step = BBR_STEP_18;	/* Die gracefully */
			ri->bbr_substep = SUB_0;
		}
		break;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step6_start
 *	
 *	Abstract:	This routine updates sector 0 of the RCT to indicate
 *			that replacement is in the Phase 1 stage.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step6_start( ri )
struct	ra_info		*ri;
{
RCT_SECTOR_0		*rct;

	switch( ri->bbr_substep ) {

	case SUB_0:

		if( ri->rct0_buf == 0 ) 
			ri->rct0_buf = geteblk(512);
		(void)bzero( ri->rct0_buf->b_un.b_addr, 512 );
		ri->multi_buf = ri->rct0_buf;
		ri->bbr_mods = 0;
		ri->bbr_substep = READ_END;
		multi_read( ri, ri->radsize );	/* Start multi-read algorithm	*/
		return(NULL);
		break;

	case READ_END:

		if(ri->status == M_ST_SUCC) {	/* Multi-read succeeded */
/*
 *	Prepare sector 0 for re-write
 */
			rct = (RCT_SECTOR_0 *)ri->rct0_buf->b_un.b_addr;
			rct->lbn = ri->bad_LBN;	
			if ( ri->bbr_fe )
				rct->flags |= RCT_FE;
			else
				rct->flags &= ~RCT_FE;
			rct->flags |= RCT_P1;	/* We are now in Phase 1 ! */
			ri->multi_buf = ri->rct0_buf;
			ri->bbr_mods = 0;
			ri->bbr_substep = WRITE_END;
			multi_write( ri, ri->radsize );
			return(NULL);
		}
		else {
			bbr_error( ri, BBR_ERR_MREAD );
			ri->bbr_step = BBR_STEP_18;	/* Die gracefully */
			ri->bbr_substep = SUB_0;
		}
		break;

	case WRITE_END:
		if ( ri->status == M_ST_SUCC ) {

		/*
		 * If we are forcing the replace then go directly to step 9
		 */
			if( ri->bbr_reason == FORCE_REPLACE ) {
				ri->bbr_step = BBR_STEP_9;
				ri->bbr_substep = SUB_0;
			}
			else {
				ri->bbr_step = BBR_STEP_7;
				ri->bbr_substep = SUB_0;
			}
		}
		else {
			bbr_error( ri, BBR_ERR_MWRITE );
			ri->bbr_step = BBR_STEP_17;	/* Unwind the BBR */
			ri->bbr_substep = SUB_0;
		}
		break;
	}
		
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step7_start
 *	
 *	Abstract:	This routine begins BBR step 7 which tests the 
 *			suspect block to determine if it is really
 *			defective.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step7_start(ri)
struct	ra_info		*ri;
{
	switch( ri->bbr_substep) {

	case SUB_0:

		if( ri->test_buf == 0 ) 
			ri->test_buf = geteblk(512);
		(void)bzero( ri->test_buf->b_un.b_addr, 512 );
		ri->bbr_mods =  M_MD_SECOR | M_MD_SEREC;
		ri->bbr_cnt = 0;
		ri->err = 0;
		ri->suc = 0;
		ri->bbr_substep = READ_END;
		mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->test_buf );
		return( NULL );
		break;

	case READ_END:

		switch ( ri->status ) {

		case M_ST_SUCC:

			if ( ri->mscp_flags & M_EF_BBLKR )
				ri->err = 1;
			break;

		case M_DT_IVHDR:
		case M_DT_DST:
		case M_DT_UCECC:

			ri->err = 1;	/* Block is really bad	*/
			break;

		default:
			
			break;

		}
		ri->bbr_cnt++;
		if( ri->err ) {
			ri->bbr_step = BBR_STEP_8;
			ri->bbr_substep = SUB_0;
		}
		else if( ri->bbr_cnt == STEP7_MAX_READ ) {
			ri->bbr_cnt = 0;
			ri->bbr_step = BBR_STEP_7B;
			ri->bbr_substep = SUB_0;
		}
		else {
			mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->test_buf );
			return( NULL );
		}
		return(1);
		break;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step7_b
 *	
 *	Abstract:	Write the user data and then try reading again.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step7_b( ri )
struct	ra_info		*ri;
{
DATA_SECTOR	*orig;
DATA_SECTOR	*invert;
int	i;

	switch( ri->bbr_substep) {

	case SUB_0:

		if( ri->inv_buf == 0 ) 
			ri->inv_buf = geteblk(512);
		(void)bzero( ri->inv_buf->b_un.b_addr, 512 );
		orig = (DATA_SECTOR *)ri->orig_buf->b_un.b_addr;
		invert = (DATA_SECTOR *)ri->inv_buf->b_un.b_addr;
		for( i = 0; i < 128; i++ ) 
			invert->data[i] = ~(orig->data[i]);
		ri->bbr_rep = 0;	/* Reset repetition counter */

	case SUB_1:

		ri->bbr_mods = M_MD_SECOR | M_MD_SEREC;
		ri->bbr_substep = WRITE_END;
		mscp_cmd( ri->bad_LBN, M_OP_WRITE, ri, ri->orig_buf );
		return( NULL );
		break;

	case WRITE_END:

		if ( ri->status != M_ST_SUCC ) {
			ri->err = 1;
			ri->bbr_step = BBR_STEP_8;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		else {
			ri->bbr_mods |= M_MD_COMP; /* Do a read compare	*/
			ri->bbr_substep = READ_END;
			mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->test_buf );
			return( NULL );
		}
		break;

	case READ_END:

		switch ( ri->status ) {

		case M_ST_SUCC:

			if ( ri->mscp_flags & M_EF_BBLKR )
				ri->err = 1;
			break;

		case M_DT_IVHDR:
		case M_DT_DST:
		case M_DT_UCECC:

			ri->err = 1;	/* Block is really bad	*/
			break;

		default:
			
			break;

		}
		ri->bbr_cnt++;
		if( ri->err ) {
			ri->bbr_step = BBR_STEP_8;
			ri->bbr_substep = SUB_0;
		}
		else if( ri->bbr_cnt == STEP7_MAX_READ ) {
			ri->bbr_cnt = 0;
			ri->bbr_step = BBR_STEP_7C;
			ri->bbr_substep = SUB_0;
		}
		else {
			mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->test_buf );
			return( NULL );
		}
		return(1);
		break;
	}		
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step7_c
 *	
 *	Abstract:	Write the one's complement of the user data to
 *			the suspect block (with a forced error) and then
 *			attempt to read it back.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step7_c( ri )
struct	ra_info		*ri;
{
	switch( ri->bbr_substep ) {

	case SUB_0:

		ri->bbr_mods = M_MD_SECOR | M_MD_SEREC | M_MD_ERROR;
		ri->bbr_substep = WRITE_END;
		mscp_cmd( ri->bad_LBN, M_OP_WRITE, ri, ri->inv_buf );
		return( NULL );
		break;

	case WRITE_END:

		if ( ri->status != M_ST_SUCC ) {
			ri->err = 1;
			ri->bbr_step = BBR_STEP_8;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		else {
			ri->bbr_mods |= M_MD_COMP; /* Do a read compare	*/
			ri->bbr_mods &= ~M_MD_ERROR; /* Get rid of forced error */
			ri->bbr_substep = READ_END;
			mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->test_buf );
			return( NULL );
		}
		break;

	case READ_END:

		
		switch ( ri->status ) {

		case M_ST_SUCC:
		case M_DT_IVHDR:
		case M_DT_DST:
		case M_DT_UCECC:

			ri->err = 1;	/* Block is really bad	*/
			break;

		default:
			
			break;

		}
		ri->bbr_cnt++;
		if( ri->err ) {
			ri->bbr_step = BBR_STEP_8;
			ri->bbr_substep = SUB_0;
		}
		else if( ri->bbr_cnt == STEP7_MAX_READ ) {
			ri->bbr_cnt = 0;
			ri->bbr_rep++;
			if( ri->bbr_rep == STEP7_MAX_REP ) {
				ri->bbr_step = BBR_STEP_8;
				ri->bbr_substep = SUB_0;
			}
			else {
				ri->bbr_step = BBR_STEP_7B;
				ri->bbr_substep = SUB_1;
			}
		}
		else {
			mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->test_buf );
			return( NULL );
		}
		return(1);
		break;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step8
 *	
 *	Abstract:	Write the saved user data back to the original
 *			block following testing of the block. If the
 *			block was determined to be bad (in step 7) then
 *			continue the algorithim else cleanup our tracks
 *			and get outta town.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step8( ri )
struct	ra_info		*ri;
{
DATA_SECTOR	*orig;
DATA_SECTOR	*test;
int		i;
struct	uba_device	*ui = ri->ui;


	switch( ri->bbr_substep ) {

	case	SUB_0:

		if( ri->bbr_fe ) 
			ri->bbr_mods = M_MD_ERROR;
		else
			ri->bbr_mods = 0;
		ri->bbr_substep = WRITE_END;
		mscp_cmd( ri->bad_LBN, M_OP_WRITE, ri, ri->orig_buf );
		return( NULL );
		break;

	case	WRITE_END:

		if ( ri->status == M_ST_SUCC ) {
			ri->bbr_substep = READ_END;
			ri->bbr_mods = M_MD_COMP;
			mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->test_buf );
			return( NULL );
		}
		else {	/* Write Failed - Replace the block	*/

			ri->bbr_step = BBR_STEP_9;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		break;

	case	READ_END:

		ri->suc = 0;
		switch ( ri->status ) {

		case	M_ST_SUCC:

			if ( (ri->mscp_flags & M_EF_BBLKR) == 0 )
				ri->suc = 1;
			break;

		case	M_DT_FEM:

			if ( ri->bbr_fe == 0)	/* Do we expect forced err? */
				ri->suc = 1;
			break;

		default:

			ri->suc = 0;

		}

/*
 *		Do the Host compare of read and written data
 */

		orig = (DATA_SECTOR *)ri->orig_buf->b_un.b_addr;
		test = (DATA_SECTOR *)ri->test_buf->b_un.b_addr;

		for( i = 0; i < 128; i++) {	/* Compare the data	*/
			if( orig->data[i] != test->data[i] ) {
				ri->err = 1;
				break;
			}
		}

		if (ri->err || (ri->suc == 0) ) {

			ri->bbr_step = BBR_STEP_9;
			ri->bbr_substep = SUB_0;
		}
		else {
			mprintf("\tunit %d, ", ui->ui_slave);
			mprintf("Transient error on LBN %d, Block not bad\n",
				ri->bad_LBN);
			ri->bbr_step = BBR_STEP_13;
			ri->bbr_substep = SUB_0;

		}
		return(1);
		break;
			
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step9
 *	
 *	Abstract:	This routine calls the RCT search algorithm
 *			to find out if this block has been revectored
 *			previously or find an empty slot if it has
 *			not.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step9( ri )
struct	ra_info		*ri;
{
int	res;
RCT_SECTOR_0		*rct = (RCT_SECTOR_0 *)ri->rct0_buf->b_un.b_addr;

	switch( ri->bbr_substep) {

	case	SUB_0:

		if( ri->recurs < 2 )	/* Still on safe ground		*/
			ri->recurs++;
		else {			/* We're in a loop - gotta die	*/
			mprintf("\tunit %d, ", ri->ui->ui_slave);
			mprintf("Replacement failure code 2\n");
			mprintf("\tunit %d, ", ri->ui->ui_slave);
			mprintf("BAD RBN FLAG\n");
			ri->bbr_step = BBR_STEP_16;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		ri->bbr_substep = SEARCH_END;
		rct_search( ri );
		return( NULL );
		break;

	case	SEARCH_END:

		switch	( ri->result ) {
	
		case	RCTS_PRIM_EMP:
		case	RCTS_NONPRIM_EMP:
	
			ri->is_primary = (ri->result == RCTS_PRIM_EMP ? 1 : 0);
			ri->bbr_step = BBR_STEP_10;
			ri->bbr_substep = SUB_0;
			break;

		case	RCTS_FULL_TAB:
		case	RCTS_READ_ERR:
		case	RCTS_RCT_CORRUPT:
	
			if( ri->result == RCTS_FULL_TAB ) {
				bbr_error( ri, BBR_ERR_RCTF );
			}
			else if( ri->result == RCTS_READ_ERR ) {
				bbr_error( ri, BBR_ERR_MREAD );
			}	
			else
				bbr_error( ri, BBR_ERR_RCTCOR );
			ri->bbr_step = BBR_STEP_16;
			ri->bbr_substep = SUB_0;
			break;
	
		}
		return(1);
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step10
 *	
 *	Abstract:	Update RCT sector 0 to indicate we are now in 
 *			phase 2 of the replacement algorithm.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step10( ri )
struct	ra_info		*ri;
{
RCT_SECTOR_0		*rct = (RCT_SECTOR_0 *)ri->rct0_buf->b_un.b_addr;

	switch( ri->bbr_substep ) {

	case	SUB_0:
		rct->new_RBN = ri->new_RBN;
		if ( ( ri->prev_repl ) == 0 ) 
			rct->flags &= ~RCT_BR;	/* Clear BR flag	*/
		else {
			rct->flags |= RCT_BR;	/* Set BR flag		*/
			rct->bad_RBN = ri->old_RBN;
		}
		rct->flags &= ~RCT_P1;
		rct->flags |= RCT_P2;
		ri->bbr_substep = WRITE_END;
		ri->multi_buf = ri->rct0_buf;
		ri->bbr_mods = 0;
		multi_write( ri, ri->radsize );
		return( NULL );
		break;

	case	WRITE_END:

		if( ri->status == M_ST_SUCC ) {
			ri->bbr_step = BBR_STEP_11;
			ri->bbr_substep = SUB_0;
		}
		else {
			bbr_error( ri, BBR_ERR_MWRITE );
			ri->bbr_step = BBR_STEP_16;
			ri->bbr_substep = SUB_0;
		}
		return(1);
		break;

	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step11
 *	
 *	Abstract:	Update the descriptors in the RCT to permanently
 *			record the replacement. If 2 blocks must be modified
 *			( replacing a previously replaced block ) then both
 *			blocks must be read before either is written.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step11( ri )
struct	ra_info		*ri;
{
RCT_SECTOR		*rct_new;
RCT_SECTOR		*rct_old;

	switch( ri->bbr_substep ) {

	case	SUB_0:

		ri->new_block = (ri->new_RBN / 128) + ri->radsize + 2;
		ri->new_offset = ri->new_RBN % 128;
		ri->bbr_substep = READ_END;
		if( ri->new_buf == 0 ) 
			ri->new_buf = geteblk(512);
		(void)bzero( ri->new_buf->b_un.b_addr, 512 );
		ri->bbr_mods = 0;
		ri->multi_buf = ri->new_buf;
		multi_read( ri, ri->new_block );
		return( NULL );
		break;

	case	READ_END:

		if ( ri->status != M_ST_SUCC ) {

			bbr_error( ri, BBR_ERR_MREAD );
			ri->bbr_step = BBR_STEP_16;
			ri->bbr_substep = SUB_0;
			return(1);
		}

		if( ri->prev_repl ) {
			ri->old_block = (ri->old_RBN / 128) + ri->radsize + 2;
			ri->old_offset = ri->old_RBN % 128;
			if( ri->old_block != ri->new_block ) {	/* Need to read other block	*/
				ri->bbr_substep = READ_END_2;
				if( ri->old_buf == 0 ) 
					ri->old_buf = geteblk(512);
				(void)bzero( ri->old_buf->b_un.b_addr, 512 );
				ri->multi_buf = ri->old_buf;
				ri->bbr_mods = 0;
				multi_read( ri, ri->old_block );
				return( NULL );
			}
			else {
				ri->status = M_ST_SUCC;
				ri->bbr_substep = READ_END_2;
			}
		}
		else {
			ri->status = M_ST_SUCC;
			ri->bbr_substep = READ_END_2;
		}
		return(1);
		break;

	case	READ_END_2:

		if ( ri->status != M_ST_SUCC ) {
			bbr_error( ri, BBR_ERR_MREAD );
			ri->bbr_step = BBR_STEP_16;
			ri->bbr_substep = SUB_0;
			return(1);
		}

		rct_new = (RCT_SECTOR *)ri->new_buf->b_un.b_addr;
		rct_new->desc[ri->new_offset].lbn = ri->bad_LBN;
		rct_new->desc[ri->new_offset].hdr =
			 (ri->is_primary ? RCT_HDR_ALL_PRI : RCT_HDR_ALL_NPR);
		if( ri->prev_repl ) {
			if( ri->new_block == ri->old_block ) {
				rct_new->desc[ri->old_offset].lbn = 0;
				rct_new->desc[ri->old_offset].hdr = RCT_HDR_BAD;
			}
			else {
				rct_old = (RCT_SECTOR *)ri->old_buf->b_un.b_addr;
				rct_old->desc[ri->old_offset].lbn = 0;
				rct_old->desc[ri->old_offset].hdr = RCT_HDR_BAD;
			}
		}
		ri->multi_buf = ri->new_buf;
		ri->bbr_mods = 0;
		ri->bbr_substep = WRITE_END;
		multi_write( ri, ri->new_block );
		return( NULL );
		break;

	case	WRITE_END:

		if ( ri->status != M_ST_SUCC ) {
			bbr_error( ri, BBR_ERR_MWRITE );
			ri->bbr_step = BBR_STEP_15;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		if( ri->prev_repl && (ri->new_block != ri->old_block) ) {
			ri->bbr_substep = WRITE_END_2;
			ri->multi_buf = ri->old_buf;
			ri->bbr_mods = 0;
			multi_write( ri, ri->old_block );
			return( NULL );
		}
		else {
			ri->bbr_step = BBR_STEP_12; /* Do the replacement */
			ri->bbr_substep = SUB_0;
		}
		break;


	case	WRITE_END_2:

		if ( ri->status != M_ST_SUCC ) {
			bbr_error( ri, BBR_ERR_MWRITE );
			ri->bbr_step = BBR_STEP_15;
			ri->bbr_substep = SUB_0;
		}
		else {
			ri->bbr_step = BBR_STEP_12; /* Do the replacement */
			ri->bbr_substep = SUB_0;
		}
		return(1);
		break;

	}

	return(1);
}
/**/
/*
 *
 *
 *	Name:		step12_a
 *	
 *	Abstract:	Issue the MSCP REPLACE command to finally do the
 *			replacement. If the REPLACE succeeds then a READ
 *			is issued to the replaced block.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step12_a( ri )
struct	ra_info		*ri;
{
RCT_SECTOR_0	*rct = (RCT_SECTOR_0 *)ri->rct0_buf->b_un.b_addr;

	switch( ri->bbr_substep ) {

	case	SUB_0:

	/*
	 * Get test buffer (if not already there) for later step 12 use
	 */
		if( ri->test_buf == 0 ) 
			ri->test_buf = geteblk(512);
		(void)bzero( ri->test_buf->b_un.b_addr, 512 );
		if( ri->is_primary ) 
			ri->bbr_mods = M_MD_PRIMR;
		else
			ri->bbr_mods = 0;
		ri->bbr_substep = REPLACE_END;
		mscp_cmd( ri->bad_LBN, M_OP_REPLC, ri, (struct buf *)NULL );
		return( NULL );
		break;

	case	REPLACE_END:

		if( ri->status != M_ST_SUCC ) {	/* We got big problems	*/
			ri->bbr_step = BBR_STEP_12B;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		else if( ri->bbr_reason == BBR_PHASE_2 ) {
			ri->bbr_step = BBR_STEP_12C; /* Skip re-read	*/
			ri->bbr_substep = SUB_0;
			return(1);
		}
		else {
			ri->bbr_mods = 0;
			ri->bbr_substep = READ_END;
			mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->test_buf );
			return( NULL );
		}
		break;

	case	READ_END:

		if( ri->status == M_DT_FEM ) {	/* Success	*/
			ri->bbr_step = BBR_STEP_12C; /* Write and Compare */
			ri->bbr_substep = SUB_0;
			return(1);
		}
		else if( (ri->status == M_DT_IVHDR) ||
			 (ri->status == M_DT_DST)   ||
			 (ri->status == M_DT_UCECC) ) { /* Bad RBN */
			ri->old_RBN = ri->new_RBN;
			rct->bad_RBN = ri->old_RBN;
			rct->flags |= RCT_BR;
			ri->prev_repl = 1;
			bbr_error( ri, BBR_ERR_BADRBN );
			ri->bbr_step = BBR_STEP_9;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		else {
			ri->bbr_step = BBR_STEP_12B; /* Replace Failed	*/
			ri->bbr_substep = SUB_0;
			return(1);
		}
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step12_b
 *	
 *	Abstract:	This routine is called when there is a REPLACE
 *			command failure. This is an extremely serious
 *			error. The saved data is write/compared back to
 *			the offending LBN, the "Data Safety Write Protect"
 *			(software write protect) is set and the device
 *			is marked as seriously brain-damaged.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step12_b( ri )
struct	ra_info		*ri;
{
RCT_SECTOR_0	*rct = (RCT_SECTOR_0 *)ri->rct0_buf->b_un.b_addr;

	switch( ri->bbr_substep ) {

	case	SUB_0:

		ri->bbr_mods = M_MD_COMP;
		ri->bbr_substep = WRITE_END;
		mscp_cmd( ri->bad_LBN, M_OP_WRITE, ri, ri->orig_buf );
		return( NULL );
		break;

	case	WRITE_END:

		mprintf("\tunit %d, ", ri->ui->ui_slave);
		mprintf("Replace command failed. Disk is in unknown state.\n");
		mprintf("\tunit %d, ", ri->ui->ui_slave);
		mprintf("Please backup media and reformat.\n");
		printf("ra%d: Replace command failed\n", ri->ui->ui_unit);
		printf("ra%d: Backup media and reformat\n", ri->ui->ui_unit);
		ri->bbr_step = BBR_STEP_17;	/* Recover as much as we can*/
		ri->bbr_substep = SUB_0;
		return(1);
		break;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step12_c
 *	
 *	Abstract:	Write-compare the saved data to the newly replaced
 *			block. Follow this with a read to make sure
 *			everything is working. A compare of the read and
 *			written data is done to verify the write success.
 *			If there are any failures the algorithm returns 
 *			to step 9 to search for a new RBN.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step12_c( ri )
struct	ra_info		*ri;
{
RCT_SECTOR_0	*rct = (RCT_SECTOR_0 *)ri->rct0_buf->b_un.b_addr;
DATA_SECTOR	*orig;
DATA_SECTOR	*test;
int		i;
struct	uba_device	*ui = ri->ui;

	switch( ri->bbr_substep ) {

	case	SUB_0:

		if( ri->bbr_fe )
			ri->bbr_mods = M_MD_ERROR;
		else
			ri->bbr_mods = 0;

		ri->bbr_substep = WRITE_END;
		mscp_cmd( ri->bad_LBN, M_OP_WRITE, ri, ri->orig_buf );
		return( NULL );
		break;

	case	WRITE_END:

		if( ri->status == M_ST_SUCC ) {
			ri->suc = 1;
			ri->bbr_substep = READ_END;
			ri->bbr_mods = M_MD_COMP;
			mscp_cmd( ri->bad_LBN, M_OP_READ, ri, ri->test_buf );
			return( NULL );
		}
		else {
			ri->suc = 0;
			ri->bbr_substep = READ_END;
			return(1);
		}
		break;

	case	READ_END:

/*
 *	If write was successful then we check the status of the read
 *	response. If it was either successful with no BBR or had an
 *	expected forced error then a host compare of the read and written
 *	data is performed. If that is successful then replacement was a
 *	success and we go to step 13 to clean things up.
 *	If the write was not successful or the read was not successful 
 *	by the above criteria or the host compare failed  this indicates
 *	the RBN is bad and we must try again.
 */
		if( ri->suc != 0 ) { /* Write was successful - check read */

			if( ( (ri->status == M_ST_SUCC) && ((ri->mscp_flags & M_EF_BBLKR) == 0) ) ||
			     ((ri->status == M_DT_FEM) && (ri->bbr_fe)) ){

		/*
		 *		Do the Host compare of read and written data
		 */

				ri->err = 0;
				orig = (DATA_SECTOR *)ri->orig_buf->b_un.b_addr;
				test = (DATA_SECTOR *)ri->test_buf->b_un.b_addr;

				for( i = 0; i < 128; i++) {	/* Compare the data */
					if( orig->data[i] != test->data[i] ) {
						ri->err = 1;
						break;
					}
				}

				if( ri->err == 0) {
					mprintf("\tunit %d, ", ui->ui_slave);
					mprintf("LBN %d replaced\n", ri->bad_LBN);
					ri->bbr_step = BBR_STEP_13;
					ri->bbr_substep = SUB_0;
					return(1);
				}
			}
		}
/*
 * 	Bad RBN  - go find another one
 */

		ri->old_RBN = ri->new_RBN;
		rct->bad_RBN = ri->old_RBN;
		rct->flags |= RCT_BR;
		ri->prev_repl = 1;
		bbr_error( ri, BBR_ERR_BADRBN );
		ri->bbr_step = BBR_STEP_9;
		ri->bbr_substep = SUB_0;
		return(1);
		break;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step13_start
 *	
 *	Abstract:	This routine updates sector 0 of the RCT to indicate
 *			that we are no longer in the BBR process.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step13_start( ri )
struct	ra_info		*ri;
{
RCT_SECTOR_0		*rct = (RCT_SECTOR_0 *)ri->rct0_buf->b_un.b_addr;

	switch( ri->bbr_substep ) {

	case	SUB_0:

		rct->flags &= ~( RCT_P1 | RCT_P2 | RCT_BR | RCT_FE );
		rct->lbn = 0;
		rct->new_RBN = 0;
		rct->bad_RBN = 0;
		ri->bbr_substep = WRITE_END;
		ri->multi_buf = ri->rct0_buf;
		ri->bbr_mods = 0;
		multi_write( ri, ri->radsize );
		return( NULL );
		break;

	case	WRITE_END:

		if( ri->status == M_ST_SUCC ) {
			ri->bbr_step = BBR_STEP_14;	/* Final cleanup */
			ri->bbr_substep = SUB_0;
		}
		else {
			bbr_error( ri, BBR_ERR_MWRITE );
			ri->bbr_step = BBR_STEP_17;	/* Cleanup	*/
			ri->bbr_substep = SUB_0;
		}
		return(1);
		break;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step14
 *	
 *	Abstract:	Final cleanup on success. This routine releases the
 *			lock put on the unit when BBR was started eons ago.
 *			The drive is then relinked to the controller queue.
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step14( ri )
struct	ra_info		*ri;
{
struct	uba_device	*ui = ri->ui;
struct	uba_ctlr 	*um = ui->ui_mi;
struct	buf		*dp;
struct	uda_softc	*sc = &uda_softc[um->um_ctlr];

		dp = &udutab[ui->ui_unit];
		/*
		 * Link the drive onto the controller queue
		 */
		if (dp->b_active == 0) {
			dp->b_forw = NULL;
			if (um->um_tab.b_actf == NULL)
				um->um_tab.b_actf = dp;
			else
				um->um_tab.b_actl->b_forw = dp;
			um->um_tab.b_actl = dp;
			dp->b_active = 1;
		}
		ri->bbr_lock = 0;		/* Release lock		 */
		ri->bbr_ip = BBR_SUCCESS; /* BBR successfully completed */
		sc->sc_bbr_ip--;	/* One less BBR in progress	*/
		wakeup( ri );	  /* Wakeup anyone listening	*/
		return( NULL );
}
/**/
/*
 *
 *
 *	Name:		step15
 *	
 *	Abstract:	Attempt to write the RCT sector containing the
 *			modified descriptor. This is a last gasp attempt.
 *			If two RCT blocks were involved an attempt is made
 *			for each of them.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step15( ri )
struct	ra_info		*ri;
{
RCT_SECTOR		*rct_new;
RCT_SECTOR		*rct_old;
u_char			hdr;
u_long			hash_block, hash_offset, hash_RBN;

	switch( ri->bbr_substep ) {

	case	SUB_0:

		ri->bbr_substep = WRITE_END;
		ri->multi_buf = ri->new_buf;
		ri->bbr_mods = 0;
		multi_write( ri, ri->new_block );
		return( NULL );
		break;

	case	WRITE_END:

		if( ri->status != M_ST_SUCC ) {
			bbr_error( ri, BBR_ERR_MWRITE );
		}
		if( ri->prev_repl && (ri->new_block != ri->old_block) ) {
			ri->bbr_substep = WRITE_END_2;
			ri->bbr_mods = 0;
			ri->multi_buf = ri->old_buf;
			multi_write( ri, ri->old_block );
			return( NULL );
		}
		else {
			ri->bbr_step = BBR_STEP_16;
			ri->bbr_substep = SUB_0;
			return(1);
		}
		break;

	case	WRITE_END_2:

		if( ri->status != M_ST_SUCC ) {
			bbr_error( ri, BBR_ERR_MWRITE );
		}
		ri->bbr_step = BBR_STEP_16;
		ri->bbr_substep = SUB_0;
		return(1);
		break;

	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step16
 *	
 *	Abstract:	Write the saved data back to the original LBN with
 *			a forced error iff the saved data is invalid.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step16( ri )
struct	ra_info		*ri;
{
	switch( ri->bbr_substep ) {

	case	SUB_0:

		if( ri->bbr_fe )
			ri->bbr_mods = M_MD_ERROR;
		else
			ri->bbr_mods = 0;
		ri->bbr_substep = WRITE_END;
		mscp_cmd( ri->bad_LBN, M_OP_WRITE, ri, ri->orig_buf );
		return( NULL );
		break;

	case	WRITE_END:

		if( ri->status != M_ST_SUCC ) {
			bbr_error( ri, BBR_ERR_WRITE );
		}
		ri->bbr_step = BBR_STEP_17;
		ri->bbr_substep = SUB_0;
		return(1);
		break;

	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step17
 *	
 *	Abstract:	Update sector 0 of the RCT to indicate that BBR
 *			is no longer in progress.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step17( ri )
struct	ra_info		*ri;
{
RCT_SECTOR_0		*rct = (RCT_SECTOR_0 *)ri->rct0_buf->b_un.b_addr;

	switch( ri->bbr_substep ) {

	case	SUB_0:

		rct->flags &= ~( RCT_P1 | RCT_P2 | RCT_BR | RCT_FE );
		rct->lbn = 0;
		rct->new_RBN = 0;
		rct->bad_RBN = 0;
		ri->bbr_mods = 0;
		ri->bbr_substep = WRITE_END;
		ri->multi_buf = ri->rct0_buf;
		multi_write( ri, ri->radsize );
		return( NULL );
		break;

	case	WRITE_END:

		if( ri->status != M_ST_SUCC ) {
			bbr_error( ri, BBR_ERR_MWRITE );
		}
		ri->bbr_step = BBR_STEP_18;
		ri->bbr_substep = SUB_0;
		return(1);
		break;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		step18
 *	
 *	Abstract:	Release lock and return failure.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	step18( ri )
struct	ra_info		*ri;
{
struct	uba_device	*ui = ri->ui;
struct	uba_ctlr 	*um = ui->ui_mi;
struct	buf		*dp, *bp;
struct	uda_softc	*sc = &uda_softc[um->um_ctlr];


	switch( ri->bbr_substep) {

	case	SUB_0:

		ri->bbr_mods = M_MD_STWRP; /* Set write protect		*/
		ri->unit_flags = M_UF_WRTPS; /* Set write protect	*/
		ri->bbr_substep = STUNT_END;
		mscp_cmd( ri->bad_LBN, M_OP_STUNT, ri, 0 );
		return( NULL );
		break;


	case	STUNT_END:


		mprintf("\tunit %d, Replacement failed\n", ui->ui_slave);
		mprintf("\tunit %d, Volume is Software Write Protected\n", ui->ui_slave);
		printf("ra%d: Replacement of LBN %d failed\n", ui->ui_unit,
			ri->bad_LBN);
		printf("ra%d: Volume is Software Write Protected\n", ui->ui_unit);
		ri->ra_flags |= DEV_WRTLCK; /* Set flag for devioctl	*/
		dp = &udutab[ui->ui_unit];
	/* 
	 * We're dead in the water. Send all queued bufs back with an error
	 */
		while (bp = dp->b_actf) {
			dp->b_actf = bp->av_forw;
			bp->b_flags |= B_ERROR;
			iodone(bp);
		}
		if( um->um_ubinfo ) {
			if (uba_hd[um->um_ubanum].uba_type&(UBABUA|UBA750)){
				if((udwtab[um->um_ctlr].av_forw==&udwtab[um->um_ctlr])
				     && (um->um_tab.b_actf == NULL)) 
					ubarelse(um->um_ubanum, &um->um_ubinfo);
	
			}
			else 	ubapurge(um);			
			}
		ri->bbr_lock = 0;		/* Release lock		 */
		ri->bbr_ip = BBR_FAILURE; /* BBR successfully completed */
		sc->sc_bbr_ip--;	/* One less BBR in progress	*/
		wakeup( ri );	  /* Wakeup anyone listening	*/
		return( NULL );
	}
}
/**/
/*
 *
 *
 *	Name:		mscp_cmd
 *	
 *	Abstract:	Issue an MSCP command
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	mscp_cmd( block, op_code, ri, bp )
u_long		block;
u_char		op_code;
struct	ra_info	*ri;
struct	buf	*bp;
{
struct	mscp	*mp;
int		i, k, tempi;
struct uba_ctlr *um;
struct udadevice *udaddr;
struct	uba_device	*ui = ri->ui;

	um = ui->ui_mi;
	udaddr = (struct udadevice *)um->um_addr;

/*
 *	Need to get uba resources before we try to get a command packet
 *	because we can give uba resources back but we cannot give
 *	command packets back.
 */
	if( op_code == M_OP_READ || op_code == M_OP_WRITE ) {
		if (um->um_hd->uba_type& UBABDA) k = UBA_CANTWAIT;
		else if (um->um_hd->uba_type & UBA780)
				k = UBA_NEEDBDP|UBA_CANTWAIT;
		else if (um->um_hd->uba_type & (UBABUA|UBA750))
				k = um->um_ubinfo|UBA_HAVEBDP|UBA_CANTWAIT;
		else if (um->um_hd->uba_type & (UBA730))
				k = UBA_CANTWAIT;
		else if (um->um_hd->uba_type & (UBAUVI|UBAUVII))
				k = UBA_CANTWAIT|UBA_MAPANYWAY;

		if((i = ubasetup(um->um_ubanum, bp, k)) == 0) {
#ifdef	BBR_DEBUG
			printd("Need resource - UBA\n");
#endif	BBR_DEBUG
			ri->res_blk = block;
			ri->res_op = op_code;
			ri->res_bp = bp;
			ri->res_wait++;
			bbr_res_need++;
			return(NULL);
		}
		if (um->um_hd->uba_type & (UBABUA|UBA750))
			tempi = i & 0xfffffff;         /* mask off bdp */
		else 
			tempi = i;
	}

	if ((mp = udgetcp(um)) == NULL) {
#ifdef	BBR_DEBUG
		printd("Need resource - cp\n");
#endif	BBR_DEBUG
		if( op_code == M_OP_READ || op_code == M_OP_WRITE ) 
			ubarelse(um->um_ubanum, &tempi);
		ri->res_blk = block;
		ri->res_op = op_code;
		ri->res_bp = bp;
		ri->res_wait++;
		bbr_res_need++;
		return(NULL);
	}
	if(bbrdebug>0)DELAY(10000);
	mp->mscp_opcode = op_code;
	mp->mscp_modifier = ri->bbr_mods;
	switch( op_code ) {

	case	M_OP_READ:
	case	M_OP_WRITE:


		mp->mscp_lbn = block;
		bp->b_blkno = block;	/* Save lbn for later use	*/
		mp->mscp_bytecnt = 512;
		if (uba_hd[um->um_ubanum].uba_type & (UBABDA|UBAUVI)) {
			mp->mscp_buffer = (i & 0x3ffff) | UDA_MAP;
			mp->mscp_mapbase = (long)
				&(uba_hd[um->um_ubanum].uh_physuba->uba_map[0]);
		} else 
			mp->mscp_buffer = (i & 0x3ffff) | (((i>>28)&0xf)<<24);
	
		bp->b_ubinfo = tempi;               /* save mapping info */
		ri->cur_buf = bp;		/* Save buf for later use */
		break;

	case	M_OP_REPLC:

		mp->mscp_lbn = block;
		mp->mscp_rbn = ri->new_RBN;
		break;

	case	M_OP_STUNT:

		mp->mscp_unitflgs = ri->unit_flags;
		break;

	case	M_OP_ONLIN:

		mp->mscp_modifier = 0;
		
	}
	mp->mscp_cmdref = BBR_CMD_REF;	/* Special cmd_ref for use in udrsp */
	mp->mscp_unit = ui->ui_slave;
	*((long *)mp->mscp_dscptr) |= UDA_OWN|UDA_INT;

	i = udaddr->udaip;              /* initiate polling */
}
/**/
/*
 *
 *
 *	Name:		multi_read
 *	
 *	Abstract:	Implement multi-read algorithm
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	multi_read( ri, lbn )
struct	ra_info		*ri;
u_long			lbn;	/* Block to start multi_read on	*/
{

	ri->save_step = ri->bbr_step; /* Save step number	*/
	ri->save_substep = ri->bbr_substep; /* Save substep		*/
	ri->bbr_cnt = 0;		/* Init counter		*/
	ri->bbr_step = MULTI_READ;	/* Make step be multi-read */
	ri->bbr_substep = SUB_0;	/* Substep 0		*/
	mscp_cmd( lbn, M_OP_READ, ri, ri->multi_buf );	/* Read first RCT block	*/
	return( NULL );
}

/**/
/*
 *
 *
 *	Name:		multi_read_cont
 *	
 *	Abstract:	Main path of multi_read
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	multi_read_cont( ri )
struct	ra_info		*ri;
{
u_long		lbn;
struct	buf	*bp;

	if ( ri->status == M_ST_SUCC ) {	/* We got one		*/
		ri->bbr_step = ri->save_step;
		ri->bbr_substep = ri->save_substep;
		return(1);
	}
	else {			/* Read failed			*/
		ri->bbr_cnt++;
		if ( ri->bbr_cnt == ri->nrct ) {
			bbr_error( ri, BBR_ERR_MREAD );
			ri->status = 1;
			ri->bbr_step = ri->save_step;
			ri->bbr_substep = ri->save_substep;
		}
		else {
			bp = ri->multi_buf;	/* Get buffer		*/
			lbn = bp->b_blkno;	/* Get last block num	*/
			lbn = lbn + ri->rctsize;	/* Read next copy */
			mscp_cmd( lbn, M_OP_READ, ri, ri->multi_buf );
			return( NULL );
		}
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		multi_write
 *	
 *	Abstract:	Implement multi-write algorithm
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	multi_write( ri, lbn )
struct	ra_info		*ri;
u_long			lbn; 	/* Block to start multi_write	*/
{

	ri->save_step = ri->bbr_step; /* Save step number	*/
	ri->save_substep = ri->bbr_substep; /* Save substep		*/
	ri->bbr_step = MULTI_WRITE;	/* Make step be multi-write */
	ri->bbr_substep = SUB_0;	/* Substep 0		*/
	ri->bbr_cnt = 0;		/* Init counter		*/
	ri->suc = 0;			/* Count of successful writes	*/
	ri->bbr_mods |= M_MD_COMP;		/* Do a write-compare	*/
	mscp_cmd( lbn, M_OP_WRITE, ri, ri->multi_buf );
	return (NULL);
}
/**/
/*
 *
 *
 *	Name:		multi_write_cont
 *	
 *	Abstract:	Main code for multi_write algorithm
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	multi_write_cont( ri )
struct	ra_info		*ri;
{
u_long		lbn;
struct	buf	*bp;

	if ( ri->status == M_ST_SUCC ) 	/* We got one		*/
		ri->suc++;
	ri->bbr_cnt++;
	if( ri->bbr_cnt == ri->nrct ) {
		if( ri->suc ) {	/* Did we write at least one ?	*/
			ri->status = M_ST_SUCC; /* Make sure status is SUCC */
			ri->bbr_step = ri->save_step;
			ri->bbr_substep = ri->save_substep;
			return(1);
		} else {
			ri->status = 1;	/* Failed	*/
			ri->bbr_step = ri->save_step;
			ri->bbr_substep = ri->save_substep;
		}
	} else {
		bp = ri->multi_buf;	/* Get buffer		*/
		lbn = bp->b_blkno;	/* Get last block number */
		lbn = lbn + ri->rctsize; /* Read next copy	*/
		mscp_cmd( lbn, M_OP_WRITE, ri, ri->multi_buf );
		return( NULL );
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		rct_search
 *	
 *	Abstract:	Implement the RCT search ping-pong algorithm as
 *			described in DSDF.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	rct_search( ri )
struct	ra_info		*ri;
{
int			result;
u_long			hash_block;
int			hash_offset;
struct	rct_search	*ss;

	ss = ri->search;
	ss->hash_block = (((ri->bad_LBN / ri->lbnstrk) * ri->rbnstrk) / 128) + ri->radsize + 2;
	ss->hash_offset = ((ri->bad_LBN / ri->lbnstrk) * ri->rbnstrk) % 128;
	ss->hash_RBN = (ss->hash_block - (ri->radsize + 2)) * 128 + ss->hash_offset;
	ss->save_step = ri->bbr_step;
	ss->save_substep = ri->bbr_substep;
	ri->bbr_step = RCT_SEARCH;
	ri->bbr_substep = READ_END;
	if( ri->new_buf == 0 ) 
		ri->new_buf = geteblk(512);
	(void)bzero( ri->new_buf->b_un.b_addr, 512 );
	ri->bbr_mods = 0;
	ri->multi_buf = ri->new_buf;
	multi_read( ri, ss->hash_block );
	return( NULL );

}
/**/
/*
 *
 *
 *	Name:		search_cont
 *	
 *	Abstract:	Continue the RCT ping-pong algorithm.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	search_cont( ri )
struct	ra_info		*ri;
{
struct	rct_search	*ss;
int	RCT_offset;
int	step;

	ss = ri->search;
	if( ri->status != M_ST_SUCC ) {
		ri->result = RCTS_READ_ERR;
		ri->bbr_step = ss->save_step;
		ri->bbr_substep = ss->save_substep;
		return(1);
	}
	ss->current_RBN = ss->hash_RBN;
	ss->RCT_block = ss->hash_block;
	RCT_offset = ss->hash_offset;
	ri->result = -1;
	ss->delta = 0;
	ri->old_RBN = 0;
	ri->prev_repl = 0;
	ss->empty = RCTS_PRIM_EMP;
	ss->eot = 0;

	do {
		test_descriptor( ri, RCT_offset, ss->current_RBN );
		if( ri->result < 0 ) {	/* Not done yet		*/
			ss->empty = RCTS_NONPRIM_EMP; /* Definitely not a primary*/
			ss->delta = -(ss->delta);
			if( ss->delta >= 0 )
				ss->delta++;
			ss->current_RBN = ss->hash_RBN + ss->delta;
			RCT_offset = ss->hash_offset + ss->delta;
		}
	} while ( (ri->result < 0) && (RCT_offset >= 0) &&
		 (RCT_offset <= 127) && (ss->eot == 0) );

	if( (ri->result < 0) && (ss->eot == 0) ) {
		ss->delta = -(ss->delta);
		if( ss->delta >= 0 )
			ss->delta++;
		if( RCT_offset < 0 )
			step = 1;	/* Linear towards bottom	*/
		else
			step = -1;	/* Linear towards top		*/
		RCT_offset = ss->hash_offset + ss->delta;
		ss->current_RBN = ss->hash_RBN + ss->delta;

		while( (ri->result < 0)  &&  (RCT_offset >= 0)  &&
			   (RCT_offset <= 127)  &&  (ss->eot == 0) ) {

			test_descriptor( ri, RCT_offset, ss->current_RBN );
			RCT_offset += step;
			ss->current_RBN += step;
		} 
	}

	if( ri->result < 0 ) {	/* Need to keep looking		*/

	/*
	 *	Linear search through blocks is done in separate substep 
	 *	because of the need to be able to re-call the routine after 
	 *	the multi_read is issued.
	 */
		ri->bbr_substep = BBR_LIN_READ;
	}
	else {			/* Found what we were looking for	*/

		ri->bbr_step = ss->save_step;
		ri->bbr_substep = ss->save_substep;
	}
	return(1);
}
/**/
/*
 *
 *
 *	Name:		bbr_linear
 *	
 *	Abstract:	Implement the linear search through the non-hash
 *			RCT blocks.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	bbr_linear( ri )
struct	ra_info		*ri;
{
struct	rct_search	*ss;
int	RCT_offset;

	ss = ri->search;
	while( ((ri->result < 0) && (ss->current_RBN != ss->hash_RBN)) ) {
		if( ri->bbr_substep == BBR_LIN_READ) {
			if( ss->eot == 0 ) 
				ss->RCT_block  += 1;
			else {
				ss->RCT_block = ri->radsize + 2;
				ss->eot = 0;
			}
			ss->current_RBN = (ss->RCT_block - (ri->radsize + 2)) * 128;
			if( ss->current_RBN == ss->hash_RBN ) { 
				ri->result = RCTS_FULL_TAB;
				ri->bbr_step = ss->save_step;
				ri->bbr_substep = ss->save_substep;
				return(1);
			}
			else {
				ri->bbr_substep = BBR_LINEAR;
				ri->bbr_mods = 0;
				ri->multi_buf = ri->new_buf;
				multi_read( ri, ss->RCT_block );
				return( NULL );
			}
		}
		else {
			if( ri->status != M_ST_SUCC ) {
				ri->result = RCTS_READ_ERR;
				ri->bbr_step = ss->save_step;
				ri->bbr_substep = ss->save_substep;
				return(1);
			}
			ri->bbr_substep = BBR_LIN_READ;
			RCT_offset = 0;
			while( ((ri->result < 0) &&
				 (ss->current_RBN != ss->hash_RBN) &&
				 (RCT_offset <= 127) && (ss->eot == 0)) ) {

				test_descriptor(ri, RCT_offset, ss->current_RBN);
				RCT_offset += 1;
				ss->current_RBN += 1;
			}
			if( ss->current_RBN == ss->hash_RBN ) /* Full Table */
				ri->result = RCTS_FULL_TAB;
		}
			
	} 

	ri->bbr_step = ss->save_step;
	ri->bbr_substep = ss->save_substep;
	return(1);

}
/**/
/*
 *
 *
 *	Name:		test_descriptor
 *	
 *	Abstract:	Test a RBN descriptor in the given RCT block.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	test_descriptor( ri, RCT_offset, current_RBN )
struct	ra_info		*ri;
int			RCT_offset;
u_long			current_RBN;
{
RCT_SECTOR	*rct_blk = (RCT_SECTOR *)ri->new_buf->b_un.b_addr;
u_short		hdr_code;
static u_long	minimum_RBN;
struct	rct_search	*ss;

	ss = ri->search;
	minimum_RBN = (((ri->radsize - 1) / ri->lbnstrk) * ri->rbnstrk) + ri->rbnstrk;
	hdr_code = rct_blk->desc[RCT_offset].hdr;

	switch( hdr_code ) {

	case	RCT_HDR_NULL:	

		if( (rct_blk->desc[RCT_offset].lbn == 0) 
			&& ( minimum_RBN < current_RBN ) ) 
				ss->eot = 1;  /* Reached end of RCT */
		else
			ri->result = RCTS_RCT_CORRUPT;
		break;

	case	RCT_HDR_UNALL:

		if( rct_blk->desc[RCT_offset].lbn == 0 ) {
			ri->new_RBN = current_RBN;
			ri->result = ss->empty;
		}
		else
			ri->result = RCTS_RCT_CORRUPT;
		break;
				
	case	RCT_HDR_ALL_PRI:
	case	RCT_HDR_ALL_NPR:

		if( rct_blk->desc[RCT_offset].lbn == ri->bad_LBN ) {
			ri->prev_repl = 1;
			ri->old_RBN = current_RBN;
		}
		break;


	case	RCT_HDR_BAD:
	case	RCT_HDR_ALT_BAD:

		if( rct_blk->desc[RCT_offset].lbn != 0 )
			ri->result = RCTS_RCT_CORRUPT;
		break;

	default:

		ri->result = RCTS_RCT_CORRUPT;

	}

	return(1);
}
/**/
/*
 *
 *
 *	Name:		bbr_requeue
 *	
 *	Abstract:	Requeue the command which generated the BBR.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	bbr_requeue( ui, mp )
struct	uba_device	*ui;
struct	mscp		*mp;
{
struct	buf		*bp, *dp;
struct	uba_ctlr	*um;

	um = ui->ui_mi;
	dp = &udutab[ui->ui_unit];
	bp = (struct buf *)mp->mscp_cmdref;

	/*
	 * Unlink buffer from I/O wait queue.
	 */
	bp->av_back->av_forw = bp->av_forw;
	bp->av_forw->av_back = bp->av_back;

	/*
	 * Mark the buffered data path as active so it doesn't get freed
	 */
	um->um_tab.b_active = 1;
	/*
	 * Link the buffer onto the front of the drive queue
	 */
	if ((bp->av_forw = dp->b_actf) == 0)
		dp->b_actl = bp;
	dp->b_actf = bp;
	dp->b_active = 0;		/* Mark drive no longer active	*/

/*
 *	Release map registers associated with failed transfer
 *	and purge buffered datapath if necessary.
 */
	ubarelse(um->um_ubanum, (int *)&bp->b_ubinfo);
		if (uba_hd[um->um_ubanum].uba_type & (UBABUA|UBA750))
			ubapurge(um);

	return(1);
}
/**/
/*
 *
 *
 *	Name:		bbr_unlink
 *	
 *	Abstract:	Unlink drive from controller queue if present
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	bbr_unlink( ui )
struct	uba_device	*ui;
{
struct	uba_ctlr	*um;
struct	buf		*dp, *cp;

	um = ui->ui_mi;
	dp = &udutab[ui->ui_unit];
	/*
	 * Remove drive from controller queue until bbr finished
	 */
	if( um->um_tab.b_actf == dp ) { /* Is it the first one ?	*/
		um->um_tab.b_actf = dp->b_forw;
		cp = (struct buf *)NULL;
	}
	else {
		if ( (cp = um->um_tab.b_actf) == NULL ) 
				return(1); /* No active drives - we're safe */

		for( ;; ) {
			if( cp->b_forw == dp ) {	/* Found it	*/
				cp->b_forw = dp->b_forw;
				break;
			} else 
				if( (cp = cp->b_forw) == NULL ) 
					break; /* This drive not active */
		}
	}
	if( um->um_tab.b_actl == dp )	/* Is it the last one ?		*/
		um->um_tab.b_actl = cp;
	return(1);
}
/**/
/*
 *
 *
 *	Name:		bbr_res_wait
 *	
 *	Abstract:	This routine is called if a bbr thread is
 *			waiting for a resource. It is called from the
 *			driver interrupt routine.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	bbr_res_wait(ri)
struct	ra_info		*ri;
{
#ifdef	BBR_DEBUG
	printd("Have resource\n");
#endif	BBR_DEBUG
	ri->res_wait--;
	bbr_res_need--;
	mscp_cmd( ri->res_blk, ri->res_op, ri, ri->res_bp );
	return;
}
/**/
/*
 *
 *
 *	Name:		bbr_inval_step
 *	
 *	Abstract:	Invalid state machine step/substep combination.
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	bbr_inval_step( ri )
struct	ra_info		*ri;
{
	printf("Invalid step ( %d ) / substep ( %d ) combo\n", ri->bbr_step, 
		ri->bbr_substep);
	panic(" BBR_INVALID_STEP \n");
}
/**/
/*
 *
 *
 *	Name:		bbr_error
 *	
 *	Abstract:	Print error messages
 *			
 *			
 *			
 *	Inputs:
 *
 *	
 *	
 *
 *	Outputs:
 *
 *	
 *	
 *	
 *
 *	Return 
 *	Values:
 *
 *	
 *	
 * 
 *	Side		
 *	Effects:		
 *				
 *				
 *				
 *				
 *
 *				
 */
int	bbr_error( ri, code )
struct	ra_info		*ri;
int			code;
{
struct	uba_device	*ui = ri->ui;

	mprintf("\tunit %d, ", ui->ui_slave);
	switch( code ) {

	case	BBR_ERR_OFFLN:
		mprintf("Unit offline\n");
		break;

	case	BBR_ERR_MREAD:
		mprintf("Multi-read access to RCT failed\n");
		break;

	case	BBR_ERR_RCT0:
		mprintf("RCT Sector 0 Corrupt\n");
		break;

	case	BBR_ERR_MWRITE:
		mprintf("Multi-write access to RCT failed\n");
		break;

	case	BBR_ERR_RCTF:
		mprintf("RCT full\n");
		break;

	case	BBR_ERR_RCTCOR:
		mprintf("RCT is corrupt\n");
		break;

	case	BBR_ERR_BADRBN:
		mprintf("Bad RBN %d\n", ri->new_RBN);
		break;

	case	BBR_ERR_WRITE:
		mprintf("WRITE command failed.\n");
		break;

	default:
		mprintf("Unknown BBR error\n");
	}
}

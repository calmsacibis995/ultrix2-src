#ifndef lint
static char sccsid[]  =  "@(#)eribld.c	1.10   (ULTRIX)   1/29/87"; 
#endif  lint

/*
*	.TITLE	ERIBLD - Builds the EIMS standard event record
*	.IDENT	/1-001/
*
* COPYRIGHT (C) 1986 DIGITAL EQUIPMENT CORP.,
* CSSE SOFTWARE ENGINEERING
* MARLBOROUGH, MASSACHUSETTS
*
* THIS SOFTWARE IS FURNISHED UNDER A LICENSE FOR USE ONLY ON A 
* SINGLE COMPUTER SYSTEM AND MAY BE COPIED ONLY WITH THE INCLUSION
* OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE,  OR ANY OTHER
* COPIES THEREOF, MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE
* TO ANY OTHER PERSON EXCEPT FOR USE ON SUCH SYSTEM AND TO ONE WHO
* AGREES TO THESE LICENSE TERMS.  TITLE TO AND OWNERSHIP OF THE
* SOFTWARE SHALL AT ALL TIMES REMAIN IN DEC.
*
* THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
* NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL
* EQUIPMENT CORPORATION.
*
* DEC ASSUMES NO RESPONSIBILITY FOR THE USE OR RELIABILITY OF
* ITS SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DEC.
*
*++
*
* FACILITY:		FMA - Event Information Management System
*
* ABSTRACT:
*
*	This module sets up the segment buffers and calls the transformation
*	engine to build each segment of the standard record.
*	
* ENVIRONMENT:	VAX/VMS C,  ULTRIX-32 C
*
* AUTHOR:  Bob Winant,  CREATION DATE:  11-Dec-85
*
* MODIFIED BY:
*
*
*--
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/errlog.h>
#include </sys/vax/cpu.h>
#include "eims.h"
#include "erms.h"
#include "select.h"
#include "eiliterals.h"
#include "ulliterals.h"
#include "generic_dsd.h"
#include "std_dsd.h"
#include "os_dsd.h"

#define MSCP_CTL_LEN 22
#define MSCP_MEM_LEN 28
#define MSCP_CTL1_LEN 14
#define MSCP_ADS_LEN 16

/***************   TABLES  *****************/

static short bi_bus_err_tbl[16] =
    {
    OS$bi1_bus_err,  OS$bi2_bus_err,  OS$bi3_bus_err,  OS$bi4_bus_err,
    OS$bi5_bus_err,  OS$bi6_bus_err,  OS$bi7_bus_err,  OS$bi8_bus_err,
    OS$bi9_bus_err,  OS$bi10_bus_err, OS$bi11_bus_err, OS$bi12_bus_err,
    OS$bi13_bus_err, OS$bi14_bus_err, OS$bi15_bus_err, OS$bi16_bus_err
    };

/********************************************/

void get_subtypes();
extern long ei$filseg();		/* Declare globally for module */
					       
extern struct segment_info seg_info;
   struct bvpsts
   {
      unsigned fill_1  : 8;
      unsigned bvp_err : 8;
      unsigned fill_2  : 16;
   };


/*
*	.SBTTL	EI$BLD - Builds the standard record
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine sets up the buffers for building a standard event
*	record, gets the definitions for each record from the
*	corresponding tables, and passes each segment to the transformation
*	engine to build a standard EIMS record
*	
* CALLING SEQUENCE:		CALL EI$BLD (..See Below..)
*					Called from EI$GET with address
*					of the raw event buffer and pointer
*					to a structure
*					containing segment ptrs
* FORMAL PARAMETERS:		
*
*	buf_addr		Address of the raw event buffer
*	eisptr			Pointer to eis
*	disptr			Pointer to dis
*	cdsptr			Pointer to cds
*	sdsptr			Pointer to sds
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* ROUTINE VALUE:		NONE
*
*
* SIDE EFFECTS:			
*
*--
*/

/*...	ROUTINE EI$BLD (eisptr, disptr, cdsptr, sdsptr, elrec_ptr)	*/
void ei$bld(eisptr, disptr, cdsptr, sdsptr, elrec_ptr)

struct el_rec *elrec_ptr;
EIS *eisptr;
DIS *disptr;
CDS *cdsptr;
SDS *sdsptr;

{

seg_info.ads_num  = 0;
seg_info.ads_next = 0;
seg_info.raw_subid_type = 1;
				/* First assign types and
				   versions to the segments	*/
eisptr->type = ES$EIS;
eisptr->subtype = 1;		/* Always 1 because there is no subtype */
eisptr->version = 1;

disptr->type = ES$DIS;
disptr->subtype = 1;		/* Always 1 because there is no subtype */
disptr->version = 1;
				/* Decide which CDS we want built*/
cdsptr->type = ES$CDS;
cdsptr->version = 1;

sdsptr->type = ES$SDS;			/* Do the SDS segment	*/
sdsptr->version = 1;

seg_info.raw_subid_class = elrec_ptr->elsubid.subid_class;

seg_info.cds_subtype = 0;
seg_info.sds_subtype = 0;
seg_info.ads_subtype = 0;

seg_info.eis_flag = FALSE;
seg_info.dis_flag = FALSE;
seg_info.cds_flag = FALSE;
seg_info.sds_flag = FALSE;
seg_info.ads_flag = FALSE;

get_subtypes(elrec_ptr);		/* get subtypes		*/
					/* for CDS, SDS, ADS	*/
cdsptr->subtype = seg_info.cds_subtype;
sdsptr->subtype = seg_info.sds_subtype;

 					/* Go get the data and 
					   fill the segments	*/

if (ei$filseg(elrec_ptr, eisptr) == EI$SUCC)
    {
    seg_info.ads_flag = TRUE;
    seg_info.eis_flag = TRUE;
    if (ei$filseg(elrec_ptr, disptr) != EI$SUCC)
	seg_info.dis_flag = FALSE;
    else
	{
	seg_info.dis_flag = TRUE;
	if (cdsptr->subtype == 0)	/* doit if a subtype is defined */
	    seg_info.cds_flag = FALSE;
	else
	    if (ei$filseg(elrec_ptr, cdsptr) != EI$SUCC)
		seg_info.cds_flag = FALSE;
	    else
		seg_info.cds_flag = TRUE;
	}
    if (ei$filseg(elrec_ptr, sdsptr) == EI$FAIL)
	seg_info.sds_flag = FALSE;
    else
	seg_info.sds_flag = TRUE;
    }
}
/*...	ENDROUTINE EI$BLD						    */


/*
*	.SBTTL	GET_SUBTYPES - Gets subtype values for CDS, SDS, ADS
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine figures out the subtype values for the 
*	CDS, SDS, and ADS from the raw record type.
*	
* CALLING SEQUENCE:		CALL GET_SUBTYPES (..See Below..)
*					Called from EI$BLD with address
*					of the raw event buffer.
* FORMAL PARAMETERS:		
*
*	buf_addr		Address of the raw event buffer
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* ROUTINE VALUE:		NONE
*
*
* SIDE EFFECTS:			
*
*--
*/

/*...	ROUTINE GET_SUBTYPES(elrec_ptr)			*/
void get_subtypes(elrec_ptr)

struct el_rec *elrec_ptr;
{
   short get_bvp_rec(), get_ci_rec();

   switch(elrec_ptr->elsubid.subid_class)
   {
    case ELCT_MCK :
	seg_info.raw_subid_type = elrec_ptr->elsubid.subid_type;
	switch(elrec_ptr->elsubid.subid_type)
	    {
	    case ELMCKT_780 :
		seg_info.cds_subtype = DD$MC780CDS;
		seg_info.ads_subtype = DD$MC78_ADS;
		break;
	    case ELMCKT_750 :
		seg_info.cds_subtype = DD$MC750_CDS;
		seg_info.ads_subtype = DD$MC75_ADS;
		break;
	    case ELMCKT_730 :
		seg_info.cds_subtype = DD$MC730_CDS;
		seg_info.ads_subtype = DD$MC73_ADS;
		break;
	    case ELMCKT_8600 :
		seg_info.cds_subtype = DD$MC8600_CDS;
		seg_info.ads_subtype = DD$MC86_ADS;
		break;
	    case ELMCKT_8200 :
		if (elrec_ptr->elsubid.subid_ctldevtyp == ST_8300)
		    seg_info.raw_subid_type = OS$mach_chk_8300;
		seg_info.cds_subtype = DD$MC8200_CDS;
		seg_info.ads_subtype = DD$MC82_ADS;
		break;
	    case ELMCKT_8800 :
		switch (elrec_ptr->elsubid.subid_ctldevtyp)
		{
		    case ST_8500:
			seg_info.raw_subid_type = OS$mach_chk_8500;
			break;
		    case ST_8550:
			seg_info.raw_subid_type = OS$mach_chk_8550;
			break;
		    case ST_8700:
			seg_info.raw_subid_type = OS$mach_chk_8700;
			break;
		    default:
			break;
		}
		seg_info.cds_subtype = DD$MC8800_CDS;
		seg_info.ads_subtype = DD$MC88_ADS;
		break;
	    case ELMCKT_UVI :
		seg_info.cds_subtype = DD$MCUVAX1_CDS;
		seg_info.ads_subtype = DD$MCUVI_ADS;
		break;
	    case ELMCKT_UVII :
		seg_info.cds_subtype = DD$MCUVAX2_CDS;
		seg_info.ads_subtype = DD$MCUVII_ADS;
		break;
	    }
	break;
    case ELCT_MEM :
        seg_info.raw_subid_type = elrec_ptr->elsubid.subid_ctldevtyp;
	switch(elrec_ptr->elsubid.subid_ctldevtyp)
	{
	    case ELMCNTR_780C:
		seg_info.cds_subtype = DD$MEMC78_CDS;
		break;
	    case ELMCNTR_780E:
		seg_info.cds_subtype = DD$MEME78_CDS;
		break;
	    case ELMCNTR_750: case ELMCNTR_730:
		seg_info.cds_subtype = DD$MEM750_CDS;
		break;
	    case ELMCNTR_8600:
		seg_info.cds_subtype = DD$MEM860_CDS;
		break;
	    case ELMCNTR_BI:
		seg_info.cds_subtype = DD$MEM820_CDS;
		if (elrec_ptr->elsubid.subid_type == ST_8300)
		    seg_info.raw_subid_type = OS$mem_8300;
		break;
	    case ELMCNTR_NMI:
		seg_info.cds_subtype = DD$MEM880_CDS;
		switch (elrec_ptr->elsubid.subid_type)
		{
		    case ST_8500:
			seg_info.raw_subid_type = OS$mem_8500;
			break;
		    case ST_8550:
			seg_info.raw_subid_type = OS$mem_8550;
			break;
		    case ST_8700:
			seg_info.raw_subid_type = OS$mem_8700;
			break;
		    default:
			break;
		}
		break;
	    case ELMCNTR_630: case ELMCNTR_VAXSTAR:
		seg_info.cds_subtype = DD$MEMUVAX2_CDS;
		break;
	    default:
		break;
	    }
	seg_info.ads_subtype = DD$MEM_ADS;
	break; 
    case ELCT_DISK:
    case ELCT_TAPE:
	seg_info.raw_subid_type = elrec_ptr->elsubid.subid_type;
	switch(elrec_ptr->el_body.elbdev.eldevdata.mslg.mslg_format)
	    {
	    case EI$MSCP_DSK_TRN:
		seg_info.raw_subid_type = OS$dsk_transfer_err;
		seg_info.cds_subtype = DD$DSK_XFR_CDS;
		seg_info.sds_subtype = DD$DSK_XFER_SDS;
		seg_info.ads_subtype = DD$DSK_XFER_ADS;
		break;
	    case EI$MSCP_SDI_ERR:
		if (elrec_ptr->elsubid.subid_ctldevtyp == ELDT_RA60)
		    {
		    seg_info.raw_subid_type = OS$ra60_sdi;
		    seg_info.cds_subtype = DD$RA60_SDI_CDS;
		    seg_info.sds_subtype = DD$RA60_SDI_SDS;
		    seg_info.ads_subtype = DD$SDI_GENERIC_ADS;
		    }
		else if (elrec_ptr->elsubid.subid_ctldevtyp == ELDT_RA80
		      || elrec_ptr->elsubid.subid_ctldevtyp == ELDT_RA81
		      || elrec_ptr->elsubid.subid_ctldevtyp == ELDT_RA82)
		    {
		    seg_info.raw_subid_type = OS$ra8_dsk_sdi;
		    seg_info.cds_subtype = DD$RA8_SDI_CDS;
		    seg_info.sds_subtype = DD$RA8_SDI_SDS;
		    seg_info.ads_subtype = DD$SDI_GENERIC_ADS;
		    }
		break;
	    case EI$MSCP_SML_DSK:
		seg_info.raw_subid_type = OS$small_dsk_err;
		seg_info.cds_subtype = DD$SMALL_DISK_CDS;
		seg_info.sds_subtype = DD$SMALL_DSK_SDS;
		seg_info.ads_subtype = DD$SMALL_DSK_ADS;
		break;
	    case EI$MSCP_TPE_TRN:
		seg_info.raw_subid_type = OS$tape_xfer_err;
		seg_info.cds_subtype = DD$TAP_XFER_CDS;
		seg_info.sds_subtype = DD$TAP_XFER_SDS;
		seg_info.ads_subtype = DD$TAP_XFER_ADS;
		break;
	    case EI$MSCP_STI_ERR:
		seg_info.raw_subid_type = OS$sti_comm_err;
		seg_info.cds_subtype = DD$STI_COM_CDS;
		seg_info.sds_subtype = DD$STI_COMM_SDS;
		seg_info.ads_subtype = DD$STI_COMM_ADS;
		break;
	    case EI$MSCP_STI_DRV:
		seg_info.raw_subid_type = OS$sti_drv_err;
		seg_info.cds_subtype = DD$STI_DRV_CDS;
		seg_info.sds_subtype = DD$STI_DRV_SDS;
		seg_info.ads_subtype = DD$STI_DRV_ADS;
		break;
	    case EI$MSCP_STI_FRM:
		seg_info.raw_subid_type = OS$sti_get_format;
		seg_info.cds_subtype = DD$STI_FMT_CDS;
		seg_info.sds_subtype = DD$STI_FMT_SDS;
		seg_info.ads_subtype = DD$STI_FMT_ADS;
		break;
	    case EI$MSCP_REPLACE:
		seg_info.raw_subid_type = OS$bad_blk_replace;
		seg_info.cds_subtype = DD$BAD_BLK_CDS;
		seg_info.sds_subtype = DD$BAD_BLK_SDS;
		seg_info.ads_subtype = DD$BAD_BLK_ADS;
		break;
	    default:
		printf("EI$BLD - unknown MSCP error %d\n",
			elrec_ptr->el_body.elbdev.eldevdata.mslg.mslg_format);
		break;
	    }
	break;
    case ELCT_DCNTL:
	switch (elrec_ptr->elsubid.subid_type)
	{
	    case ELMSCP_CNTRL: case ELTMSCP_CNTRL:
		switch(elrec_ptr->el_body.elbdev.eldevdata.mslg.mslg_format)
	    	{
		    case EI$MSCP_BUS_ADR:
		        seg_info.raw_subid_type = OS$disk_host_mem_err;
		        seg_info.cds_subtype = DD$DSK_MEM_CDS;
		        seg_info.sds_subtype = DD$DSK_MEM_SDS;
		        seg_info.ads_subtype = DD$DSK_MEM_ADS;
		        break;
		    case EI$MSCP_CNT_ERR:
		        seg_info.raw_subid_type = OS$mscp_disk_ctlr_err;
		        seg_info.cds_subtype = DD$DSK_CTL_CDS;
		        seg_info.sds_subtype = DD$DSK_CTL_SDS;
		        seg_info.ads_subtype = DD$DSK_CTL_ADS;
		        break;
		    default:
		        break;
	        }
		break;
	   case ELBI_BVP:
	      seg_info.raw_subid_type = get_bvp_rec(elrec_ptr->el_body.elbvp.bvp_pstatus, elrec_ptr);
	      switch(seg_info.raw_subid_type)
	      {
		 case OS$bvp_bier_rec:
		    seg_info.cds_subtype = DD$BVP_BIER_CDS;
		    break;
		 case OS$bvp_reg_rec:
		    seg_info.cds_subtype = DD$BVP_GEN_CDS;
		    break;
		 default:
		    printf("\nEI$BLD - unknown BVP error\n");
		    break;
	      }
	      seg_info.sds_subtype = DD$BVP_SDS;
	      break;
	   case ELCI_ATTN:
	      seg_info.raw_subid_type = get_ci_rec(elrec_ptr);
	      switch(seg_info.raw_subid_type)
	      {
		 case OS$ci_gen_rec:
		    seg_info.cds_subtype = DD$CI_GEN_CDS;
		    break;
		 case OS$cibi_common:
		    seg_info.cds_subtype = DD$CI_GEN_CDS;
		    seg_info.sds_subtype = DD$CIBI_COMMON_SDS;
		    break;
		 case OS$ci_dattn_common:
		 case OS$ci7_ucode_dattn:
		    seg_info.cds_subtype = DD$CI7_DATTN_CDS;
		    seg_info.sds_subtype = DD$CI7_DATTN_SDS;
		    seg_info.ads_subtype = DD$CI7_DATTN_ADS;
		    break;
		 case OS$cici_dattn_common:
		 case OS$cibci_ucode_dattn:
		    seg_info.cds_subtype = DD$CIBCI_DATTN_CDS;
		    seg_info.sds_subtype = DD$CIBCI_DATTN_SDS;
		    seg_info.ads_subtype = DD$CIBI_DATTN_ADS;
		    break;
		 case OS$cibca_dattn_common:
		 case OS$cibca_ucode_dattn:
		    seg_info.cds_subtype = DD$CIBCA_DATTN_CDS;
		    seg_info.sds_subtype = DD$CIBCA_DATTN_SDS;
		    seg_info.ads_subtype = DD$CIBI_DATTN_ADS;
		    break;
		 default:
		    break;
	      }
	      break;
	   case ELCI_LPKT:
	      seg_info.raw_subid_type = get_ci_rec(elrec_ptr);
	      seg_info.cds_subtype = DD$CI_LPKT_CDS;
	      seg_info.sds_subtype = DD$CI_LPKT_SDS;
	      switch(seg_info.raw_subid_type)
	      {
		 case OS$ci_prot_lpkt:
		    seg_info.ads_subtype = DD$CI_LPKT_PROT_ADS;
		    break;
		 case OS$ci_collis_lpkt:
		    seg_info.ads_subtype = DD$CI_LPKT_COLLIS_ADS;
		    break;
		 case OS$ci_logged_lpkt:
		    seg_info.ads_subtype = DD$CI_LPKT_PKT_ADS;
		    break;
		 default:
		    break;
	      }
	      break;
	   case ELUQ_ATTN:
      	      seg_info.raw_subid_type = elrec_ptr->elsubid.subid_ctldevtyp;
	      switch (elrec_ptr->elsubid.subid_ctldevtyp)
	      {
		 case ELUQHPT_UDA50: case ELUQHPT_UDA50A:
	      	    seg_info.raw_subid_type = OS$uda5x_attn_err;
		    seg_info.cds_subtype = DD$UDA5X_ATTN_CDS;
		    break;
		 case ELUQHPT_TK50: case ELUQHPT_TK70:
	      	    seg_info.raw_subid_type = OS$tk57_attn_err;
		    seg_info.cds_subtype = DD$TK57_ATTN_CDS;
		    break;
		 case ELUQHPT_KDB50:
		    seg_info.cds_subtype = DD$KDB50_ATTN_CDS;
		    break;
		 case ELUQHPT_RC25:
		    seg_info.cds_subtype = DD$RC25_ATTN_CDS;
		    break;
		 default:
		    seg_info.cds_subtype = DD$UQ_PORT_CDS;
		    break;
	      }
	      break;
	   case ELBI_BLA:
	      seg_info.raw_subid_type = elrec_ptr->elsubid.subid_type;
	      seg_info.cds_subtype = DD$BLA_ERR_CDS;
	      seg_info.sds_subtype = DD$BLA_ERR_SDS;
	      seg_info.ads_subtype = DD$BLA_ERR_ADS;
	      break;
	   default:
	      printf("\nEI$BLD - unknown controller error %d\n",
		    elrec_ptr->elsubid.subid_type);
	      break;
	}
	break;
    case ELCT_ADPTR:
	seg_info.raw_subid_type = elrec_ptr->elsubid.subid_type;
	switch(elrec_ptr->elsubid.subid_type)
	    {
	    case ELADP_BUA:
		seg_info.cds_subtype = DD$BUA_ERR_CDS;
		seg_info.sds_subtype = DD$BUA_ERR_SDS;
		seg_info.ads_subtype = DD$BUA_ERR_ADS;
		break;
	    case ELADP_UBA:
		seg_info.cds_subtype = DD$UBA780_CDS;
		seg_info.ads_subtype = DD$UBA78_ADS;
		break;
	    case ELADP_NBI:
		seg_info.cds_subtype = DD$NMIADP_CDS;
		seg_info.sds_subtype = DD$NMIADP_SDS;
		break;
	    default:
		printf("\nEI$BLD - unknown adapter error %d\n",
				 elrec_ptr->elsubid.subid_type);
		break;
	    }
	break;
    case ELCT_BUS:
	seg_info.raw_subid_type = elrec_ptr->elsubid.subid_type;
	switch(elrec_ptr->elsubid.subid_type)
	    {
	    case ELBUS_SBI780:
		seg_info.cds_subtype = DD$SBI780_CDS;
		seg_info.ads_subtype = DD$SBI78_ADS;
		break;
	    case ELBUS_SBI8600:
		seg_info.cds_subtype = DD$SBIA8600_CDS;
		seg_info.ads_subtype = DD$SBI86_ADS;
		break;
	    case ELBUS_BIER:
		seg_info.cds_subtype = DD$BI_BUS_ERR_CDS;
		seg_info.ads_subtype = DD$BI_BUS_ERR_ADS;
		seg_info.ads_num = elrec_ptr->el_body.elbier.bier_nument;
		seg_info.raw_subid_type =
			bi_bus_err_tbl[seg_info.ads_num-1];
		break;
	    case ELBUS_NMIFLT:
		seg_info.cds_subtype = DD$NMIFLT_CDS;
		seg_info.sds_subtype = DD$NMIFLT_SDS;
		seg_info.ads_subtype = DD$NMIFLT_ADS1;
	    default:
		printf("EI$BLD - unknown bus error %d\n",
				 elrec_ptr->elsubid.subid_type);
		break;
	    }
	break;
    case ELCT_SINT:
/*	seg_info.cds_subtype = 				*/
	seg_info.ads_subtype = DD$STRAY_ADS;
	break;
    case ELCT_AWE:
	seg_info.cds_subtype = DD$ASYNCH_WRT_CDS;
	seg_info.ads_subtype = DD$ASYNCWR_ADS;
	break;
    case ELCT_EXPTFLT:
/*	seg_info.cds_subtype = 				*/
	seg_info.ads_subtype = DD$EXPTFLT_ADS;
	break;
    case ELSW_PNC:
	seg_info.cds_subtype = DD$PANIC_CDS;
	seg_info.ads_subtype = DD$PANIC_ADS;
	break;
    case ELMSGT_SU: case ELMSGT_SD:
	seg_info.cds_subtype = DD$SYS_STARTUP_CDS;
    case ELMSGT_INFO:
    case ELMSGT_DIAG:
    case ELMSGT_REPAIR:
	seg_info.cds_subtype = DD$INFO_MSG_CDS;
	break;
    default :
	seg_info.cds_subtype = 0;
	break;
    }
}
/*...	ENDROUTINE GET_SUBTYPES    */


/*
*	.SBTTL	BLD_CORRUPT_EIS - Builds an eis for a corrupted raw event
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine 
*	
* CALLING SEQUENCE:		CALL BLD_CORRUPT_EIS(..See Below..)
*					Called from EI$GET with address
*					if a corrupt raw system event is
*					found.
* FORMAL PARAMETERS:		
*
*	eis			Pointer to eis
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* ROUTINE VALUE:		EI$SUCC (success always returned)
*
*
* SIDE EFFECTS:			
*
*--
*/
/*...	ROUTINE BLD_CORRUPT_EIS(eis)				*/
void bld_corrupt_eis(eis)
   EIS *eis;
{
					/* Assign EI$CORRUPT to 
					   eventclass, and 0 out 
					   everything else	*/
eis->eventclass = EI$CORRUPT;
eis->eventtype = 0;
eis->recordnumber = 0;
eis->ostype = 0;
eis->datetime = 0;
eis->uptime = 0;
eis->serialnumber = 0;
eis->hostname = 0;

}

/*...	ENDROUTINE BLD_CORRUPT_EIS					    */



/*
*	.SBTTL	ADS_BLD - Builds an ads
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine 
*	
* CALLING SEQUENCE:		CALL ADS_BLD(..See Below..)
*					Called from EI$ADS_GET
*
* FORMAL PARAMETERS:		
*
*	adsptr			Pointer to ads
*	event_ptr		Pointer to data
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* ROUTINE VALUE:		NONE
*
*
* SIDE EFFECTS:			Completes a standard EIMS record
*
*--
*/

/*...	ROUTINE ADS_BLD(adsptr, event_ptr)			*/
long ads_bld(adsptr, event_ptr)

ADS *adsptr;
char *event_ptr;

{
extern struct segment_info seg_info;
struct el_rec *elrec_ptr;
static short pkt_len;		/* Length of pkt */
static ctl_info_len;		/* controller specific info length */

elrec_ptr = (struct el_rec *)event_ptr;	/* See it as an error record */
if (seg_info.ads_flag != TRUE)
    return(EI$FAIL);

adsptr->subtype = seg_info.ads_subtype;

/**************************** BI BUS ERROR **************************/

if (seg_info.ads_subtype == DD$BI_BUS_ERR_ADS)
   {
   if (seg_info.ads_next < seg_info.ads_num)
      {
      seg_info.raw_subid_type = bi_bus_err_tbl[seg_info.ads_next++];
      }
   else
      return (EI$FAIL);
   }

else
   switch (seg_info.raw_subid_class){
/**************************** MSCP ERRORS ***************************/
      case ELCT_DISK: case ELCT_TAPE:
	 if (seg_info.ads_next == 0)
	    adsptr->subtype = DD$DSA_GENERIC_ADS;
	 else
	    if (seg_info.ads_next == 1)
	    {
	       if (seg_info.raw_subid_class == ELCT_DISK)
	           adsptr->subtype = DD$GENERIC_DISK_ADS;
	       else
	           adsptr->subtype = DD$GENERIC_TAPE_ADS;
	    }
            else
	        seg_info.ads_flag = FALSE;	/* last ads*/
	 seg_info.ads_next++;
   	 break;
      case ELCT_DCNTL:
	 if (seg_info.ads_next == 0){
	    switch (elrec_ptr->elsubid.subid_type){
		case ELMSCP_CNTRL: case ELTMSCP_CNTRL:
		    if (elrec_ptr->elsubid.subid_type == ELMSCP_CNTRL)
			pkt_len = elrec_ptr->el_body.elbdev.eldevdata.mslg.mslg_header.uda_msglen;
		    else
			pkt_len = elrec_ptr->el_body.elbdev.eldevdata.tmslg.mslg_header.tmscp_msglen;
		    if ((pkt_len > MSCP_CTL_LEN) ||
			(pkt_len > MSCP_MEM_LEN)){
			switch (elrec_ptr->el_body.elbdev.eldevdata.mslg.mslg_format){
			  case EI$MSCP_BUS_ADR:
			    ctl_info_len = pkt_len - MSCP_MEM_LEN;
			    seg_info.ads_subtype = DD$DSK_MEM_ADS1;
			    ctl_info_len -= MSCP_ADS_LEN;
			    break;
			  case EI$MSCP_CNT_ERR:
			    ctl_info_len = pkt_len - MSCP_CTL_LEN;
			    seg_info.ads_subtype = DD$DSK_CTL_ADS1;
			    ctl_info_len -= MSCP_CTL1_LEN;
			    break;
			  default:
			    break;
			}
		    seg_info.ads_next++;
		    }
		    break;
		default:
		    seg_info.ads_next++;
		    ctl_info_len = 0;
		    break;
	    }
	 }
	    else{
		if (ctl_info_len > 0){
		    switch (seg_info.ads_next){
		      case 1:
			seg_info.ads_subtype = DD$DSA_CTL_ADS1;
			break;
		      case 2:
			seg_info.ads_subtype = DD$DSA_CTL_ADS2;
			break;
		      case 3:
			seg_info.ads_subtype = DD$DSA_CTL_ADS3;
			break;
		      case 4:
			seg_info.ads_subtype = DD$DSA_CTL_ADS4;
			break;
		      default:
			seg_info.ads_flag = FALSE;
			break;
		    }
		ctl_info_len -= MSCP_ADS_LEN;
		seg_info.ads_next ++;
		}
		else
		    seg_info.ads_flag = FALSE;
	    }
	 break;
/**************************** PANICS ***************************/
      case ELSW_PNC:
	 switch (seg_info.ads_next)
	    {
	       case 0:
		 seg_info.ads_subtype = DD$PANIC_KERNEL1_ADS;
		  break;
	       case 1:
		  seg_info.ads_subtype = DD$PANIC_INTSTK1_ADS;
		  break;
	       default:
		  seg_info.ads_flag = FALSE;	/* Set flag off to sho done */
		  break;
	    }
	 seg_info.ads_next++;
	 break;


/**************************** ALL OTHERS 1 ADS **********************/
	
   default:
      seg_info.ads_flag = FALSE;	/* Set flag off to sho done */
      break;
   }
/********************************************************************/

if (ei$filseg(event_ptr, adsptr) == EI$FAIL)
    return (EI$FAIL);
else
      return (EI$SUCC);
}
short get_bvp_rec(status_reg, rec_ptr)
   struct bvpsts status_reg;
   struct el_rec *rec_ptr;
{
   short rec_type;

   if (rec_ptr->elsubid.subid_ctldevtyp == ELBVP_ACP ||
       rec_ptr->elsubid.subid_ctldevtyp == ELBVP_SHDWFAX)
      rec_type = OS$lynx_acp_sxerr;
   else
   {
      switch(status_reg.bvp_err)
      {
         case 1: case 3: case 4:
	    rec_type = OS$bvp_bier_rec;
         default:
	    rec_type = OS$bvp_reg_rec;
      }
   }
   return (rec_type);
}


/*
*	.SBTTL	GET_CI_REC - Determines the type of CI record
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine determines what CI record description to use
*	depending on the error code found in the raw record,
*	
* CALLING SEQUENCE:		CALL GET_CI_REC (..See Below..)
*					Called from GET_SUBTYPES with a pointer
*					to the raw event buffer
*
* FORMAL PARAMETERS:		
*
*	rec_ptr			Pointer to the raw event buffer
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		The raw_subid_type field of the seg_info
*				structure
*
* ROUTINE VALUE:		NONE
*
*
* SIDE EFFECTS:			
*
*--
*/

/*...	ROUTINE GET_CI_REC (rec_ptr)				*/
short get_ci_rec(rec_ptr)
   struct el_rec *rec_ptr;
{
   struct el_rec chk_rec_size;
   short rec_type;	/* For os record descriptor */

/* Get the right os record descriptor */
   switch(rec_ptr->elsubid.subid_type)
   {
/* Check for data attention */
      case ELCI_ATTN:
	 switch(rec_ptr->elsubid.subid_errcode)
	 {
	    case 0x7:
	    case 0xE:
	    case 0x4000001:
	    case 0x4000002:
	    case 0x4000003:
	    case 0x4000004:
	    case 0x4000005:
	    case 0x4000006:
	    case 0x4000007:
	    case 0x4000008:
	    case 0x4000009:
	    case 0x4000014:
	    case 0x4000015:
	    case 0x4000016:
	    case 0x4000017:
	    case 0x4000018:
	    case 0x4000019:
	    case 0x400001A:
	    case 0x400001B:
	    case 0x400001C:
	    case 0x400001D:
	    case 0x5000002:
	       switch(rec_ptr->elsubid.subid_ctldevtyp)
	       {
		  case ELCIHPT_CI780: case ELCIHPT_CI750:
		     rec_type = OS$ci_dattn_common;
		     break;
		  case ELCIHPT_CIBCI:
		     rec_type = OS$cici_dattn_common;
		     break;
		  case ELCIHPT_CIBCA:
		     rec_type = OS$cibca_dattn_common;
		     break;
		  default:
		     break;
	       }	       
	       break;
	    case 0x8:
	    case 0x9:
	    case 0xA:
	    case 0xB:
	    case 0xC:
	       switch(rec_ptr->elsubid.subid_ctldevtyp)
	       {
		  case ELCIHPT_CI780: case ELCIHPT_CI750:
		     rec_type = OS$ci_gen_rec;
		     break;
		  case ELCIHPT_CIBCI: case ELCIHPT_CIBCA:
		     rec_type = OS$cibi_common;
		     break;
		  default:
		     break;
	       }	       
	       break;
	    case 0xD:
	       switch(rec_ptr->elsubid.subid_ctldevtyp)
	       {
		  case ELCIHPT_CI780: case ELCIHPT_CI750:
		     rec_type = OS$ci7_ucode_dattn;
		     break;
		  case ELCIHPT_CIBCI:
		     rec_type = OS$cibci_ucode_dattn;
		     break;
		  case ELCIHPT_CIBCA:
		     rec_type = OS$cibca_ucode_dattn;
		     break;
		  default:
		     break;
	       }	       
	       break;
	    default:
	       break;
	 }
	 break;

/* Logged packet error */
      case ELCI_LPKT:
	 if ((rec_ptr->elsubid.subid_errcode == 0x3000004) ||
	     (rec_ptr->elsubid.subid_errcode == 0x3000005))
	 {
	    if ((sizeof(rec_ptr->el_body)) ==
		(sizeof(chk_rec_size.el_body.elci.elci_types.elcilpkt)))
	       rec_type = OS$ci_logged_lpkt;
	    else
	       rec_type = OS$ci_common_lpkt;
	 }
	 else
		switch(rec_ptr->elsubid.subid_errcode)
		{
		   case 0x1:
		   case 0x2:
		   case 0x3:
		   case 0x4:
		   case 0x5:
		   case 0x6:
		   case 0x2000007:
		   case 0x3000001:
		   case 0x3000006:
		   case 0x3000009:
		   case 0x5000003:
		      rec_type = OS$ci_common_lpkt;
		      break;
		   case 0x1000001:
		   case 0x1000002:
		      rec_type = OS$ci_prot_lpkt;
		      break;
		   case 0x1000003:
		      rec_type = OS$ci_collis_lpkt;
		      break;
		   case 0x1000004:
		   case 0x2000001:
		   case 0x2000002:
		   case 0x2000003:
		   case 0x2000004:
		   case 0x2000005:
		   case 0x2000006:
		   case 0x3000003:
		   case 0x3000007:
		   case 0x3000008:
		   case 0x300000A:
		   case 0x400000A:
		   case 0x400000B:
		   case 0x400000C:
		   case 0x400000D:
		   case 0x400000E:
		   case 0x400000F:
		   case 0x4000010:
		   case 0x4000011:
		   case 0x4000012:
		   case 0x4000013:
		   case 0x5000001:
		      rec_type = OS$ci_logged_lpkt;
		      break;
		   default:
		      break;
		 }
		 break;
	      default:
		 rec_type = 0;
		 break;
	   }
   return (rec_type);
}

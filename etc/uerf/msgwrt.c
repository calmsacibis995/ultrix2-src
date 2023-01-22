#ifndef lint
static char sccsid[]  =  "@(#)msgwrt.c	1.8   (ULTRIX)   12/16/86"; 
#endif  lint

/*
*	.TITLE  MSGWRT - output field formatter for BTT
*	.IDENT	/1-001/
*
* COPYRIGHT (C) 1985 DIGITAL EQUIPMENT CORP.,
* CSSE SOFTWARE ENGINEERING
* MARLBOROUGH, MASSACHUSETTS
*
* THIS SOFTWARE IS FURNISHED UNDER A LICENSE FOR USE ONLY ON A 
* SINGLE COMPUTER SYSTEM AND MAY BE COPIED ONLY WITH THE INCLUSION
* OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE,  OR ANY OTHER
* COPIES THEREOF; MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE 
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
*	This module does the final output field processing for BTT
*	process.  It receives a pointer to the specific field in the
*	segment and a pointer to the field descriptor.  It uses this 
*	to transform the field to its final output.
*	
* ENVIRONMENT:	VAX/VMS C,  ULTRIX-32 C
*
* AUTHOR:  Bob Winant,  CREATION DATE:  05-Dec-85
*
* MODIFIED BY:
*
*	Luis Arce	24-Jan-86	complete initial development
*
*
*--
*/

#include <stdio.h>
#include <sys/time.h>
#include "ueliterals.h"
#include "uestruct.h"
#include "uerror.h"
#include "btliterals.h"
#include "generic_dsd.h"
#include "std_dsd.h"


/*
*	.SBTTL	OUTPUT_FLD
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine separates output fields by field type
*	as designated by the field descriptor.
*	
* CALLING SEQUENCE:		CALL OUTPUT_FLD (..See Below..)
*
* FORMAL PARAMETERS:		dsd_ctx
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* ROUTINE VALUE:		invalid_field (error)
*				good_field (success)
*	
* SIDE EFFECTS:			NONE
*
*--
*/



/***********  ROUTINE DEFINITIONS TO USE IN MATRIX  **********/

#define shtdec   1
#define shthex	 2
#define shtoct	 3
#define lngdec	 4
#define lnghex	 5
#define lngoct	 6
#define strstr	 7
#define shtinx	 8
#define lnginx	 9
#define srghex	10
#define srgdec	11
#define lrghex	12
#define lrgdec	13
#define dattim	14
#define bytvec	15
#define invmov	16
#define ctsvec	17
#define ctlvec	18
#define ctadvc	19
#define byvcdc  20

#define display_num	7		/* num of display codes	*/
#define type_num	12		/* number of type codes	*/

/********    TABLE MATRIX OF ROUTINES TO PRINT DATA   ****************/

/* ------------------ display format codes --------------------------

 DEFLT   DEC    HEX    OCT    DATE   ELAP   HEX
			      TIME   TIME   DUMP
---------------------------------------------------------------------*/
short rtn_tbl[type_num][display_num] = {

{shtdec,shtdec,shthex,shtoct,invmov,invmov,invmov},	/* SHORT     */
{lngdec,lngdec,lnghex,lngoct,invmov,invmov,invmov},	/* LONG	     */
{strstr,invmov,invmov,invmov,invmov,invmov,invmov},	/* STRING    */
{shtinx,invmov,invmov,invmov,invmov,invmov,invmov},	/* SHORT INX */
{lnginx,invmov,invmov,invmov,invmov,invmov,invmov},	/* LONG INX  */
{srghex,srgdec,srghex,invmov,invmov,invmov,invmov},	/* SHORT REG */
{lrghex,lrgdec,lrghex,invmov,invmov,invmov,invmov},	/* LONG REG  */
{invmov,invmov,invmov,invmov,dattim,invmov,invmov},	/* DATE	     */
{bytvec,byvcdc,bytvec,invmov,invmov,invmov,invmov},	/* BYTE VEC  */
{ctsvec,invmov,bytvec,invmov,invmov,invmov,ctsvec},	/* CT_SH VEC */
{ctlvec,invmov,bytvec,invmov,invmov,invmov,ctlvec},	/* CT_LG VEC */
{invmov,invmov,invmov,invmov,invmov,invmov,ctadvc}	/* CT_AD VEC */
};


/*     declaration of other processing routines ***/

long  bld_reg_string();
long  bld_invalid_string();
long  bld_dmy_string();
long  print_string();
long  hex_dump();

extern struct in_struc in_st;

/*...	ROUTINE OUTPUT_FLD (dsd_ctx)				    */

long  output_fld(dsd_ctx)

DD$STD_DSD_CTX *dsd_ctx;

{
short i;
short len;
unsigned short sht;
unsigned long  lng;
long  addr;
long  reg;
char  *string_ptr;

if (dsd_ctx->item_VALID_code == DD$N_A)
    return(UE$SUCC);

if (in_st.out_form == UE$OUT_TERSE)
    {
    if (dsd_ctx->user_1 == BT$NEW_SEG)
	{
        dsd_ctx->user_1 = UE$NULL;
	printf("%-*.*s", BT$F1_LEN, BT$F1_LEN,
			dsd_ctx->item_DSD_ptr->LABEL);
	}
    else
        printf("\n%-*.*s", BT$F1_LEN, BT$F1_LEN,
			dsd_ctx->item_DSD_ptr->LABEL);
    }

else 
    {
    if (dsd_ctx->user_1 == BT$NEW_SEG)
        {
        dsd_ctx->user_1 = UE$NULL;
        if (strlen(dsd_ctx->segment_DSD_ptr->LABEL) > 1)
            printf("\n\n----- %s -----\n", dsd_ctx->segment_DSD_ptr->LABEL);
        }
    printf("\n%-*.*s", BT$F1_LEN, BT$F1_LEN,
			dsd_ctx->item_DSD_ptr->LABEL);
    }

switch (dsd_ctx->item_VALID_code)
    {
    case (DD$VALID):
	switch (rtn_tbl[dsd_ctx->item_DSD_ptr->TYPE-1]
		   [dsd_ctx->item_DSD_ptr->DISPLAY-1])
	{
		/************************************************/
		/* Following routines are in order of frequency */
		/************************************************/

	case (shtinx):			/*** short indexed 	***/
	    if (*((short *) dsd_ctx->item_ptr) == -1)
    		break;
	    if ((int)(string_ptr = (char *) decode_std_item
			(dsd_ctx, *((short *) dsd_ctx->item_ptr)))
			!= DD$UNKNOWN_CODE)
	        {
		printf("%*s", BT$F2_LEN, BT$SPACE);
		printf("%*s", BT$F3_LEN, BT$SPACE);
	        print_string(string_ptr,(BT$F1_LEN + BT$F2_LEN + BT$F3_LEN));
	        }
    	break;

	case (lnghex):			/*** long Hexidecimal	***/
	    if (*((long *) dsd_ctx->item_ptr) == -1)
	        break;
	    printf("%*s", BT$F2_LEN-9, BT$SPACE);
	    lng = *((long *) dsd_ctx->item_ptr);
	    printf("x%08.8X", lng);
    	break;

	case (strstr):			/*** string		***/
	    if (in_st.out_form != UE$OUT_TERSE)
		{
	        printf("%*s", BT$F2_LEN, BT$SPACE);
	        printf("%*s", BT$F3_LEN, BT$SPACE);
		i = (BT$F1_LEN + BT$F2_LEN + BT$F3_LEN);
		}
	    else
		{
	        printf("   ");
		i = (BT$F1_LEN + 3);
		}
	    print_string(*((long *) dsd_ctx->item_ptr), i);
    	break;

	case (lngdec):			/*** long decimal	***/
	    if (*((long *) dsd_ctx->item_ptr) == -1)
	        break;
	    printf("%*d.", BT$F2_LEN-1, *((long *) dsd_ctx->item_ptr));
    	break;

	case (lrgdec):			/*** long reg decimal	***/
	    printf("%*d.",BT$F2_LEN-1, *((long *) dsd_ctx->item_ptr));
	    reg = *((long *) dsd_ctx->item_ptr);
	    if ((reg != -1) && (in_st.out_form != UE$OUT_TERSE))
	        bld_reg_string(dsd_ctx,reg);
    	break;

	case (lrghex):			/*** long reg hex	***/
	    printf("%*s", BT$F2_LEN-9, BT$SPACE);
	    lng = *((long *) dsd_ctx->item_ptr);
	    printf("x%08.8X", lng);
	    reg = *((long *) dsd_ctx->item_ptr);
	    if ((reg != -1) && (in_st.out_form != UE$OUT_TERSE))
	        bld_reg_string(dsd_ctx,reg);
    	break;

	case (dattim):			/*** date		***/
	    bld_dmy_string((long ) dsd_ctx->item_ptr);
    	break;

	case (lnginx):			/*** long indexed	***/
	    if (*((long *) dsd_ctx->item_ptr) == -1)
	        break;
	    if ((int)(string_ptr = (char *) decode_std_item
			(dsd_ctx, *((long *) dsd_ctx->item_ptr)))
			!= DD$UNKNOWN_CODE)
	        {
		printf("%*s", BT$F2_LEN, BT$SPACE);
		printf("%*s", BT$F3_LEN, BT$SPACE);
	        print_string(string_ptr,(BT$F1_LEN + BT$F2_LEN + BT$F3_LEN));
	        }
    	break;

	case (shthex):			/*** short hexidecimal	***/
	    if (*((short *) dsd_ctx->item_ptr) == -1)
    		break;
	    printf("%*s", BT$F2_LEN-5, BT$SPACE);
	    sht = *((short *) dsd_ctx->item_ptr);
	    printf("x%04.4X", sht);
    	break;

	case (ctlvec):			/*** count long vector  ***/
	    len = *((long *)dsd_ctx->item_ptr);
	    if (len > dsd_ctx->item_DSD_ptr->COUNT)
		len = dsd_ctx->item_DSD_ptr->COUNT;
	    if (len > 0)
		hex_dump((dsd_ctx->item_ptr + 4), (len * 4),4,0);
	break;

	case (ctadvc):			/*** count addr vector  ***/
	    len  = *((long *)(dsd_ctx->item_ptr+4));
	    addr = *((long *)dsd_ctx->item_ptr);
	    if (len > dsd_ctx->item_DSD_ptr->COUNT)
		len = dsd_ctx->item_DSD_ptr->COUNT;
	    if (len > 0)
		hex_dump((dsd_ctx->item_ptr + 8), (len * 4),4,addr);
	break;

	case (bytvec):			/*** byte vector	***/
	    printf("%*s", (BT$F2_LEN - 1 - (2 * dsd_ctx->item_DSD_ptr->COUNT)),
				BT$SPACE);
	    printf("x");
	    for (i = dsd_ctx->item_DSD_ptr->COUNT -1; i >= 0; i--)
		{
	    	printf("%02.2X", dsd_ctx->item_ptr[i]);
		}
    	break;

	case (byvcdc):			/*** byte vector decimal ***/
	    printf("%*s", (BT$F2_LEN - 1 - dsd_ctx->item_DSD_ptr->COUNT),
				BT$SPACE);
	    for (i = dsd_ctx->item_DSD_ptr->COUNT -1; i >= 0; i--)
		{
	    	printf("%01.1d", dsd_ctx->item_ptr[i]);
		}
	    printf(".");
    	break;

	case (shtdec):			/*** short decimal	***/
	    if (*((short *) dsd_ctx->item_ptr) == -1)
    		break;
	    printf("%*d.", BT$F2_LEN-1, *((short *) dsd_ctx->item_ptr));
    	break;

	case (shtoct):			/*** short octal	***/
	    if (*((short *) dsd_ctx->item_ptr) == -1)
	        break;
	    printf("%*s", BT$F2_LEN-5, BT$SPACE);
	    printf("o%04o", *((short *) dsd_ctx->item_ptr));
    	break;

	case (lngoct):			/*** long octal		***/
	    if (*((long *) dsd_ctx->item_ptr) == -1)
	        break;
	    printf("%*s", BT$F2_LEN-9, BT$SPACE);
	    printf("o%08o", *((long *) dsd_ctx->item_ptr));
    	break;

	case (srgdec):			/*** short reg decimal	***/
	    printf("%*d.",BT$F2_LEN-1, *((short *) dsd_ctx->item_ptr));
	    reg = *((long *) dsd_ctx->item_ptr);
	    if ((reg != -1) && (in_st.out_form != UE$OUT_TERSE))
	        bld_reg_string(dsd_ctx,reg);
    	break;

	case (srghex):			/*** short reg hex	***/
	    printf("%*s", BT$F2_LEN-5, BT$SPACE);
	    sht = *((short *) dsd_ctx->item_ptr);
	    printf("x%04.4X", sht);
	    reg = *((long *) dsd_ctx->item_ptr);
	    if ((reg != -1) && (in_st.out_form != UE$OUT_TERSE))
	        bld_reg_string(dsd_ctx,reg);
    	break;

	case (ctsvec):			/*** count short vector ***/
	    len = *((short *)dsd_ctx->item_ptr);
	    if (len > dsd_ctx->item_DSD_ptr->COUNT)
		len = dsd_ctx->item_DSD_ptr->COUNT;
	    if (len > 0)
		hex_dump((dsd_ctx->item_ptr + 2), (len * 2),2,0);
	break;

	case (invmov):			/*** invalid move	***/
	    printf("Invalid move TYPE = %d, DISPLAY = %d",
		dsd_ctx->item_DSD_ptr->TYPE,dsd_ctx->item_DSD_ptr->DISPLAY);
    	break;

	}
    break;
    case (DD$N_V):
	bld_invalid_string(dsd_ctx);
    break;
    case (DD$N_V$N_A):
	printf("%*s", BT$F2_LEN, BT$INV_ITEM);
	printf("%*s", BT$F3_LEN, BT$SPACE);
	printf("%-s", BT$NOT_AVAIL);
    break;
    }
return(UE$SUCC);   
}

/*...	ENDROUTINE OUTPUT_FLD				    */



/*
*	.SBTTL	BLD_REG_STRING - Builds register formated output
*
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine is called to build formatted
*	register output of the given input.
*	
* CALLING SEQUENCE:		CALL BLD_REG_STRING (..See Below..)
*
* FORMAL PARAMETERS:		
*
*	reg			a long with the register contents
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		Final formatted output string
*	
* ROUTINE VALUE:		UE$SUCC  if successful
*				UE$FAIL  if failure
*	
* SIDE EFFECTS:			Creates the final output
*
*--
*/
/*...	ROUTINE BLD_REG_STRING (ctx,reg)	    */
long  bld_reg_string (dsd_ctx,reg)

DD$STD_DSD_CTX *dsd_ctx;
unsigned long  reg;

{
char *string_ptr;
unsigned long bits;
short first;

first = TRUE;

do
    {
				/* The next three instructions need  */
				/* to be kept separate because VMS   */
				/* will propagate the high order bit */
				/* on the last shift right if they   */
				/* are combined into one instruction */

    bits = reg  >> dsd_ctx->field_position;
    bits = bits << (32 - dsd_ctx->field_DSD_ptr->SIZE);
    bits = bits >> (32 - dsd_ctx->field_DSD_ptr->SIZE);

				/*************************************/

    switch (dsd_ctx->field_DSD_ptr->CLASS)
	{
	case (DC_INTEGER):
	    if (!first)
		{
		printf("\n%*s",BT$F1_LEN,BT$SPACE);
		printf("%*s",  BT$F2_LEN,BT$SPACE);
		}
	    printf("%*s",  BT$F3_LEN,BT$SPACE);
	    printf("%-*.*s",BT$F4A_LEN, BT$F4A_LEN,
	    		dsd_ctx->field_DSD_ptr->LABEL);
	    printf("%*s",  BT$F4B_LEN,BT$SPACE);
	    if (dsd_ctx->field_DSD_ptr->DISPLAY == DF_DECIMAL)
		printf("%-d.", bits);
	    else if (dsd_ctx->field_DSD_ptr->DISPLAY == DF_OCTAL)
		printf("o%-o", bits);
	    else
		printf("x%-X", bits);
	    break;
	case (DC_CODED):
	    if ((int)(string_ptr = (char *)
		    decode_register_field(dsd_ctx,bits))
		    != DD$UNKNOWN_CODE)
		{
		if (!first)
		    {
		    printf("\n%*s",BT$F1_LEN,BT$SPACE);
		    printf("%*s",  BT$F2_LEN,BT$SPACE);
		    }
		printf("%*s",  BT$F3_LEN,BT$SPACE);
		if (strlen(dsd_ctx->field_DSD_ptr->LABEL) > 1)
		    {
		    printf("%-*.*s",BT$F4A_LEN, BT$F4A_LEN,
				dsd_ctx->field_DSD_ptr->LABEL);
		    printf("%*s",BT$F4B_LEN,BT$SPACE);
	            print_string(string_ptr,
				(BT$F1_LEN + BT$F2_LEN + BT$F3_LEN +
				 BT$F4A_LEN + BT$F4B_LEN));
		    }
		else
	            print_string(string_ptr,
				(BT$F1_LEN + BT$F2_LEN + BT$F3_LEN));
		}
	    break;
	default:
	    break;
	}
    first = FALSE;
    }
while ((get_next_field_dsd(dsd_ctx)) == DD$SUCCESS);
return(UE$SUCC);
}
/*...	ENDROUTINE BLD_REG_STRING					    */



/*
*	.SBTTL	BLD_INVALID_STRING - Builds invalid output
*
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine is called to build a string
*	from invalid input.
*	
* CALLING SEQUENCE:		CALL BLD_INVALID_STRING (..See Below..)
*
* FORMAL PARAMETERS:		dsd_ctx
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		Final formatted output string
*	
* ROUTINE VALUE:		UE$SUCC  if successful
*				UE$FAIL  if failure
*	
* SIDE EFFECTS:			Creates the final output
*
*--
*/
/*...	ROUTINE BLD_INVALID_STRING (dsd_ctx)	    */
long  bld_invalid_string (dsd_ctx)

DD$STD_DSD_CTX *dsd_ctx;

{
unsigned short sht;
unsigned long  lng;

printf("%*s",BT$F2_LEN, BT$INV_ITEM);
printf("%*s",BT$F3_LEN,BT$SPACE);		/* skip filler		*/
switch (dsd_ctx->item_DSD_ptr->TYPE)
    {
    case (DT_LONG):
    case (DT_INDEXED):
    case (DT_REGISTER):
	printf("%*s", BT$F4_LEN-9, BT$SPACE);
	lng = *((long *) dsd_ctx->item_ptr);
	printf("x%-08.8X", lng);
	break;
    case (DT_SHORT):
    case (DT_SHORT_INDEX):
    case (DT_SHORT_REGISTER):
	printf("%*s", BT$F4_LEN-5, BT$SPACE);
	sht = *((short *) dsd_ctx->item_ptr);
	printf("x%-04.4X", sht);
	break;
    }
}
/*...	ENDROUTINE BLD_INVALID_STRING					    */



/*
*	.SBTTL	BLD_DMY_STRING - Builds date  and time formated output
*
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine is called to build formatted
*	date and time output of the given input.
*	
* CALLING SEQUENCE:		CALL BLD_DMY_STRING (..See Below..)
*
* FORMAL PARAMETERS:		
*
*	std_item		pointer to the input field
*	dsd_item		pointer to the field descriptor
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		Final formatted output string
*	
* ROUTINE VALUE:		UE$SUCC  if successful
*				UE$FAIL  if failure
*	
* SIDE EFFECTS:			Creates the final output
*
*--
*/
/*...	ROUTINE BLD_DMY_STRING (bindate)	    */
long  bld_dmy_string (bindate)

long  *bindate;

{
struct timeval  tv, *tvp;
struct timezone tz, *tzp;
struct tm       lt, *ltp;

printf("%*s",BT$F2_LEN,BT$SPACE);		/* skip numeric field	*/
printf("%*s",BT$F3_LEN,BT$SPACE);		/* skip filler		*/
if (*bindate == UE$NULL)
    printf("NULL Date & Time");
else
    {
    tvp = &tv;
    tzp = &tz;
    ltp = &lt;
    gettimeofday(tvp,tzp);
    ltp = localtime(bindate);
    printf("%-24.24s %s", ctime(bindate),	/* print string */
			  timezone(tzp->tz_minuteswest,ltp->tm_isdst));
    }
}
/*...	ENDROUTINE BLD_DMY_STRING					    */



/*
*	.SBTTL	PRINT_STRING - Prints out a string on the right side of
*				the screen.
*
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine is called to print out a char string within
*	th boundries of the field on the right hand side of the screen.
*	
* CALLING SEQUENCE:		CALL PRINT_STRING (..See Below..)
*
* FORMAL PARAMETERS:		
*
*	str_ptr 		pointer to the input field
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		Final formatted output string
*	
* ROUTINE VALUE:		UE$SUCC  if successful
*				UE$FAIL  if failure
*	
* SIDE EFFECTS:			Creates the final output
*
*--
*/
/*...	ROUTINE PRINT_STRING (str_ptr)	    */
long  print_string (str_ptr,blanks)
char  *str_ptr;
short blanks;

{
short len;
short offset;
short buflen;
short num_put;

len = strlen(str_ptr);
num_put = 0;

buflen = 79 - blanks;
while (len > 0)
    {
    offset = strcspn(str_ptr," \t\n\0");
    if ((offset + num_put) > buflen)
	{
	printf("\n%*s", blanks + 2, BT$SPACE);
	num_put = 2;
	}
    printf("%.*s ",(offset), str_ptr);
    len     -= (offset + 1);
    num_put += (offset + 1);
    if ((len != 0) && (str_ptr[offset] == '\n'))
	{
	printf("\n%*s", blanks, BT$SPACE);
	num_put = 0;
	}
    str_ptr += (offset + 1);
    }
return(UE$SUCC);
}
/*...	ENDROUTINE PRINT_STRING					    */



/*
*	.SBTTL	HEX_DUMP     - function used to print a hex dump
*++
* FUNCTIONAL DESCRIPTION:		
*
*	-  this function receives the pointer to an area in memory
*		and a length and it prints a HEX dump of it.
*
* FORMAL PARAMETERS:		Pointer to area in memory.
*				Length of area to dump.
*
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* COMPLETION STATUS:		NONE
*
* SIDE EFFECTS:			NONE
*
*--
*/
/*...	FUNCTION HEX_DUMP         			*/

#define LINESIZE  60

long  hex_dump(ptr,len,size,addr)
DD$BYTE *ptr;
short   len;
short   size;
long    addr;

{

short  offset = 0;
short  i;
short  j;
char   line[LINESIZE];
static char hex_tbl[17] = "0123456789ABCDEF";

if (size != 2)
    size =  4;

for (;;)
    {
    for (i = 0; i < LINESIZE; i++)
	line[i] = ' ';
    printf("\n%04.4X:   ", (addr+offset));

    for (j = 0; j < (16/size); j++)
	{
	for (i = size; i > 0; i--, offset++)
	    {
	    line[(j*(size*2+2))+(i*2)]   = hex_tbl[(ptr[offset]>>4)&(0x0f)];
	    line[(j*(size*2+2))+(i*2)+1] = hex_tbl[(ptr[offset]   )&(0x0f)];
	    }
	if (len <= offset)
	    {
	    printf("%*.*s", LINESIZE, LINESIZE, line);
	    return;
	    }
	}
    printf("%*.*s", LINESIZE, LINESIZE, line);
    }
}

/*...   ENDFUNCTION HEX_DUMP			*/

#ifndef lint
static char sccsid[]  =  "@(#)eixfrm.c	1.10   (ULTRIX)   3/3/87"; 
#endif  lint

/* DEC/CMS REPLACEMENT HISTORY, Element EIXFRM.C*/
/* *1    20-FEB-1986 13:14:41 EMLICH "Raw to standard transformation engine - first base-level"*/
/* DEC/CMS REPLACEMENT HISTORY, Element EIXFRM.C*/
/*
*	.TITLE	EIXFRM - Transformation system for ERIT
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
* FACILITY:             ERIT - Event Record Input Transformer
*
* ABSTRACT:
*
*	This module contains the code for making transformations
*       between the raw entry and the standard entry. It is called by
*       the ERIT process to build one segment. This module calls the
*       DSD system to get raw and standard definitions and to set the
*       field validity code.
*	
* ENVIRONMENT:	VAX/VMS C,  ULTRIX-32 C, (and hopefully beyond)
*
* AUTHOR:  Larry Emlich,  CREATION DATE:  22-Nov-85 (templates)
*                                          7-Feb-86 (code)
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
#include "erms.h"
#include "eiliterals.h"   /* For return codes  */
#include "generic_dsd.h"  /* For DSD interface */
#include "std_dsd.h" 	  /* For DSD interface */
#include "os_dsd.h" 	  /* For DSD interface */


/*
*  - Module hierarchy chart -
*                                   ERIT entry point
*                                          |
*                                    +-----+-----+
*                                    | EI$filseg |
*                                    +-----+-----+
*                                          |
*               +----------+---------------+--------------+--------+
*               |          |               |              |        |
*            +--+--+              +--------+-------+   +--+--+
*            | ini_|  to DSD for  | transformation |   | chk_|  to DSD to set
*            | all |  raw field   |    matrix      |   | fld |  valid and get
*            +--+--+              |  (type_move)   |   +-----+  next segment
*               |                 +--------+-------+          
*      +--------+-------+                  |
*      |        |       |      +-----------+----------+-----+-------+
*   to DSD for event +--+--+   |           |          |     |       |
*   and segment data | ini_|   
*                    | seg |  simple    string ops  coded  date   special
*                    +-----+   
*                                          |
*                                     +----+----+
*                                     | wrt_str |
*                                     +---------+
*                              
*                              
*/

/*
*	.SBTTL	DSD_DAT - Macros to reference DSD stuff
*++
* FUNCTIONAL DESCRIPTION:		
*
*    These macros allow EIXFRM to look at DSD structures in a way
*    that allows DSD to change without having to change all the
*    code.
*--
*/

#define SEG DD$STD_HEADER         /* Segment header */

/* point to following with ctx. or ctx-> depending on use. */

#define SEG_FIELDS segment_DSD_ptr->ELEMENT_COUNT
#define FLD_LOC item_ptr
#define FLD_ID item_DSD_ptr->ID
#define FLD_TYPE  item_DSD_ptr->TYPE
#define FLD_SIZE item_DSD_ptr->COUNT

/*
*	.SBTTL	ERX_DAT - Internal structures and stuff
*++
* FUNCTIONAL DESCRIPTION:		
*
*     Structures and other things global to this module but not
*     useful outside it.
*--
*/

/* The following keeps track of free character space in the segment */

typedef struct
    {
    long *raw_string;		/* pointer to the raw string	*/
    long *std_str_ptr;		/* pointer to std string pointer*/
    short size;			/* max raw size of string	*/
    }  STR_PTRS;

typedef struct
    {
    DD$BYTE  *end_seg;		/* End of segment		*/
    short    str_cnt;		/* Number of strings in segment */
    STR_PTRS *stptpt;		/* next STR_PTRS structure	*/
    } STR_DAT;

short seg_size[8] = { 
    0,
    ES$EISIZE, /* ES$EIS is 1 */
    ES$DISIZE, /* ES$DIS is 2 */
    ES$SDSIZE, /* ES$SDS is 3 */
    ES$CDSIZE, /* ES$CDS is 4 */
    ES$ADSIZE, /* ES$ADS is 5 */
    ES$SISIZE, /* ES$SIS is 6 */
    ES$CISIZE  /* ES$CIS is 7 */
};

/*
*	.SBTTL	LIB_MATRIX - Definition of routine matrix
*++
* FUNCTIONAL DESCRIPTION:		
*
* The following matrix of routines makes it relatively easy to
* deal with new data types. The rows are "raw" data types and the
* columns are segment data types (which should stay constant). To
* add a "raw" type, just create a new row. The data classes are
* dealt with inside each conversion function.
*--
*/
/***********  DEFINITION OF ROUTINE VALUES FOR MATRIX  **********/
#define shsh	 1
#define shlg	 2
#define lgsh	 3
#define lglg	 4
#define tish	 5
#define tilg	 6
#define sxsx	 7
#define sxlx	 8
#define lxsx	 9
#define lxlx	10
#define txsx	11
#define txlx	12
#define stst	13
#define bvbv	14
#define ilmv	15
#define cvsh	16
#define cvlg	17
#define cvlv	18
#define cvad	19

#define RAW_TYPES	18		/* Possible raw types	*/
#define STD_TYPES	12		/* Possible std types	*/
short type_move[RAW_TYPES][STD_TYPES] = {

/* ------------------------ standard  segment ----------------------------

 SHRT LONG STRG SHRT LONG SHRT LONG DATE BYTE CNT  CNT  CNT
                INDX INDX REG  REG       VECT SHRT LONG ADDR
					      VECT VECT VECT
                                                                RAW
-------------------------------------------------------------------------*/

{shsh,shlg,ilmv,ilmv,ilmv,shsh,shlg,ilmv,ilmv,ilmv,ilmv,ilmv}, /* SHORT     */
{lgsh,lglg,ilmv,ilmv,ilmv,ilmv,lglg,ilmv,ilmv,ilmv,ilmv,ilmv}, /* LONG      */
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv}, /* STRING    */
{ilmv,ilmv,ilmv,sxsx,sxlx,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv}, /* SHORT IDX */
{ilmv,ilmv,ilmv,lxsx,lxlx,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv}, /* LONG IDX  */
{ilmv,ilmv,ilmv,ilmv,ilmv,shsh,shlg,ilmv,ilmv,ilmv,ilmv,ilmv}, /* SHORT REG */
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,lglg,ilmv,ilmv,ilmv,ilmv,ilmv}, /* LONG REG  */
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,lglg,ilmv,ilmv,ilmv,ilmv}, /* DATE      */
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,bvbv,ilmv,ilmv,ilmv}, /* BYTE VECT */
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,cvsh,ilmv,ilmv}, /* CT_SH VECT*/
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,cvlg,ilmv}, /* CT_LG VECT*/
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,cvad}, /* CT_VEC ADD*/
{tish,tilg,ilmv,ilmv,ilmv,tish,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv}, /* TINY      */
{ilmv,ilmv,ilmv,txsx,txlx,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv}, /* TINY IDX  */
{ilmv,ilmv,stst,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv}, /* ASCIIZ    */
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv}, /* BIT VECT  */
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv}, /* SHORT VECT*/
{ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,ilmv,cvlv,ilmv}  /* LONG VECT */

}; /*... End of "type_move" routine value matrix */


/********************   MACROS  ************************/

/******************* SIMPLE_RTN  ***********************/

#define SIMPLE_RTN(rtn_name, src_type, dst_type) \
    case (rtn_name): \
	*((dst_type *) seg_ctx.FLD_LOC) = \
	*((src_type *) raw_ctx.FLD_LOC);\
    break;

/******************** DECODE_RTN ***********************/

#define DECODE_RTN(rtn_name, src_type, dst_type) \
    case (rtn_name): \
	if ((*((dst_type *) seg_ctx.FLD_LOC) = \
	  decode_os_item (&raw_ctx, *((src_type *) raw_ctx.FLD_LOC))) \
	  == DD$UNKNOWN_CODE) \
	    s = DD$N_V$N_A; \
    break; 

/********************* STRING_RTN **********************/

#define STRING_RTN(rtn_name) \
    case (rtn_name): \
	stptrs = (STR_PTRS *)(--str.stptpt); \
	stptrs->raw_string  = (long *) raw_ctx.FLD_LOC; \
	stptrs->std_str_ptr = (long *) seg_ctx.FLD_LOC; \
	stptrs->size        = raw_ctx.FLD_SIZE; \
        str.str_cnt++; \
    break;

/********************** VECTOR_RTN *********************/

#define VECTOR_RTN(rtn_name) \
    case (rtn_name): \
	for (i = 0; i < seg_ctx.FLD_SIZE; i++) \
	     seg_ctx.FLD_LOC[i] = raw_ctx.FLD_LOC[i]; \
    break;

/********************** COUNT_VECTOR *******************/

#define CNTVCT_RTN(rtn_name, src_type, dst_type) \
    case (rtn_name): \
	len  = ((long)(*(src_type *) raw_ctx.FLD_LOC)); \
	size = ((long) (sizeof(src_type))); \
	*((dst_type *) seg_ctx.FLD_LOC) = (len/size); \
	for (i = 0; i < len; i++) \
	     seg_ctx.FLD_LOC[size+i] = raw_ctx.FLD_LOC[size+i]; \
    break;

/******************* VECTOR -> COUNT_VECT **************/

#define VEC_CV_RTN(rtn_name, src_type, dst_type) \
    case (rtn_name): \
	size = ((long) (sizeof(src_type))); \
	len  = ((long) (seg_ctx.FLD_SIZE)) * size; \
	*((dst_type *) seg_ctx.FLD_LOC) = (len/size); \
	for (i = 0; i < len; i++) \
	     seg_ctx.FLD_LOC[size+i] = raw_ctx.FLD_LOC[i]; \
    break;

/****************** COUNT_ADDR_VECTOR ******************/

#define CNADVC_RTN(rtn_name) \
    case (cvad): \
	*((long *) seg_ctx.FLD_LOC) = *((long *) raw_ctx.FLD_LOC); \
	len  = ((long)(*(long *) (raw_ctx.FLD_LOC+4))/4); \
	if ((len > ((long)(seg_ctx.FLD_SIZE)) ) || (len < 0)) \
	    len = (long)seg_ctx.FLD_SIZE; \
	*((long *) (seg_ctx.FLD_LOC+4)) = len; \
	len = len * 4; \
	for (i = 0; i < len; i++) \
	     seg_ctx.FLD_LOC[8+i] = raw_ctx.FLD_LOC[8+i]; \
    break;

/********************** END OF MACROS ******************/



/*
*	.SBTTL	EI$FILSEG - Fill segment with data converted from raw entry
*++
* FUNCTIONAL DESCRIPTION:		
*
*    Controls the transformation of raw data to a standard segment.
*    After initialization, each segment field returned by DSD is found in
*    the raw buffer, validity-checked, moved to the segment buffer, and
*    validity-checked again.
*	
* CALLING SEQUENCE:		CALL EI$FILSEG (..See Below..)
*                                   Called by ERIT once for every segment
*                                   to be built.
*
* FORMAL PARAMETERS:		raw buffer pointer
*                               segment buffer pointer
*
* IMPLICIT INPUTS:              type-move matrix
*
* IMPLICIT OUTPUTS:		NONE
*                  
* COMPLETION CODES:             Transformation successful (success)
*                                   (may have invalid fields)  
*                               Fatal Program error
*
* SIDE EFFECTS:                 Lower routines fill the segment buffer
*              
*--
*/
/*...	ROUTINE EI$FILSEG ()		    */
long ei$filseg (raw_buf,seg_buf)
DD$BYTE *raw_buf;  /* Don't need to know structure. Leave it to DSD */
SEG *seg_buf;      /* This just defines the common header */

{
static DD$OS_DSD_CTX raw_ctx;   /* raw arg block for DSD	*/
static DD$STD_DSD_CTX seg_ctx;  /* segment arg block for DSD	*/

static STR_DAT str;             /* Free character stuff 	*/
STR_PTRS *stptrs;		/* pointer for string ptr struc */

long size;
long len;
short i;
long s;				/* for status			*/
long ini_all();			/* internal functions		*/
long chk_fld();
long save_strings();

long get_item_dsd();		/* external functions		*/
long get_next_item_dsd();
long set_validity_code();

short fld_num;                  /* Track the segment field number */

				/* init by getting entry & segment 
				   definitions. Also init the segment */

if (ini_all(raw_buf, seg_buf, &raw_ctx, &seg_ctx, &str) != EI$SUCC)
    return EI$FAIL;


				/* major loop on next page */


				/* Move all corresponding according 
				   to segment definitions */
				/* The first field is already setup. */

for (fld_num = 1; fld_num <= seg_ctx.SEG_FIELDS; fld_num++) 
    {				/* Find item in raw buffer */
    if ((get_item_dsd(&raw_ctx, seg_ctx.FLD_ID)) != DD$SUCCESS)
	{
	if (seg_ctx.FLD_ID == DD$ostype)
	    {
	    *((short *) seg_ctx.FLD_LOC) = es$ultrix32;
	    s = DD$VALID;
	    }
	else
	    s = DD$N_A;
	}
    else
	{
	if ((raw_ctx.FLD_TYPE > RAW_TYPES) ||
	    (seg_ctx.FLD_TYPE > STD_TYPES))
	    {
	    s = DD$N_V$N_A;
	    }
        else
	    {
	    s = DD$VALID;		/* preset to valid */

	    switch (type_move[raw_ctx.FLD_TYPE-1][seg_ctx.FLD_TYPE-1])
		{
			/*   The following macros expand to the proper
			 *     case statments needed to move the data.
			 *   They are in order of frequency.
			 *
			 * routine  source  dest.
		         *   name    type   type	*/

		SIMPLE_RTN( lglg,    long,  long);
		SIMPLE_RTN( shlg,   short,  long);
		STRING_RTN( stst                );
		DECODE_RTN( sxsx,   short, short);
		SIMPLE_RTN( shsh,   short, short);
		DECODE_RTN( txlx, DD$BYTE,  long);
		DECODE_RTN( txsx, DD$BYTE, short);
		DECODE_RTN( lxlx,    long,  long);
		VECTOR_RTN( bvbv                );
		CNADVC_RTN( cvad                );
		CNTVCT_RTN( cvlg,    long,  long);
		VEC_CV_RTN( cvlv,    long,  long);
		SIMPLE_RTN( tish, DD$BYTE, short);
		SIMPLE_RTN( lgsh,    long, short);
		SIMPLE_RTN( tilg, DD$BYTE,  long);
		DECODE_RTN( sxlx,   short,  long);
		DECODE_RTN( lxsx,    long, short);
		CNTVCT_RTN( cvsh,   short, short);
		    
		default:
		    s = DD$N_V$N_A;
		break;
		}		/* end of switch construct	*/
/********************************************************************/

	    if (s == DD$VALID)
		{
                s = chk_fld(seg_ctx.FLD_LOC /* , seg_id.valid */);
		}
	    }
	}
    set_validity_code(&seg_ctx,s);

    if ((s = get_next_item_dsd(&seg_ctx)) == FALSE)
        if (fld_num < seg_ctx.SEG_FIELDS)
	    {
            printf("\nEIXFRM - Early end-of-segment from DSD\n");
            return EI$FAIL; 	/* number of elements disagress */
            }

    } 				/*... END LOOP ON ELEMENT COUNT */

				/* Account for variable-length strings
				   and return */

if (s != FALSE)
    {
    printf("\nEIXFRM - No end-of-segment from DSD\n");
    return EI$FAIL; 		/* finished "for loop" but no end-of-seg */
    }
    
save_strings(seg_buf, seg_ctx.FLD_LOC, &str);
return EI$SUCC;

}

/*...	ENDROUTINE EI$FILSEG		    */

/*
*	.SBTTL	INI_ALL
*++
* FUNCTIONAL DESCRIPTION:		
*
*    Initialize by calling DSD for entry and segment information and
*    by calling internally to clear the segment buffer. We use both
*    to set up the free-string control structure.
*	
* CALLING SEQUENCE:		CALL INI_ALL (..See Below..)
*                                    Usually called by FILL_SEG
*
* FORMAL PARAMETERS:		Pointer to raw buffer 
*                               Pointer to segment buffer
*                               Pointers to DSD context blocks
*                               Pointer to free string tracking block
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		string tracking block filled
*
* COMPLETION CODES:             Success or program error (from lower level)
*
* SIDE EFFECTS:			Segment buffer is cleared by lower routine
*                               First raw & std data items in context blks
*--
*/
/*...	ROUTINE INI_ALL ()				    */

long ini_all (raw_buf,seg_buf,raw_ctx,seg_ctx,str)
DD$BYTE *raw_buf;  /* Don't need to know structure. Leave it to DSD */
SEG *seg_buf;
DD$OS_DSD_CTX *raw_ctx;
DD$STD_DSD_CTX *seg_ctx;
STR_DAT *str;

{
extern struct segment_info seg_info;
extern short  str_cnt;

int get_std_segment_dsd(), get_os_record_dsd();  /* in DSD */

				/* Get defs and set up internal structure.
				   Also call to init seg buffer */

raw_ctx->event_ptr = raw_buf;
seg_ctx->segment_ptr = seg_buf;
raw_ctx->event_subtype = seg_info.raw_subid_type;
raw_ctx->event_type    = seg_info.raw_subid_class;

if (get_os_record_dsd(raw_ctx))
    if (get_std_segment_dsd(seg_ctx))
	{
				/* initialize string info	*/
	str->end_seg = ((DD$BYTE *)seg_buf + seg_size[seg_buf->type]);
	str->stptpt  = (STR_PTRS *) str->end_seg;
	str->str_cnt = 0;
        return EI$SUCC;
        }
return EI$FAIL;  /* Someone failed & should print error message */

} 
/*...	ENDROUTINE INI_ALL 				    */

/*
*	.SBTTL	SAVE_STRINGS
*++
* FUNCTIONAL DESCRIPTION:		
*
*    Clears segment string area and saves strings in segment.
*	
* CALLING SEQUENCE:		CALL SAVE_STRINGS (see Below..)
*
* FORMAL PARAMETERS:		Pointer to segment buffer
*                               pointer to string tracking structure
*
* IMPLICIT INPUTS:		segment type in segment buffer
*                               segment size (array in module header)
*
* IMPLICIT OUTPUTS:		String area cleared
*
* ROUTINE VALUE:                EI$SUCC or EI$FAIL
*
* SIDE EFFECTS:			None
*
*--
*/
/*...	ROUTINE SAVE_STRINGS()    */

long save_strings(seg_buf, str_loc, str)
SEG *seg_buf;
DD$BYTE *str_loc;		/* end of fixed area in seg	*/
STR_DAT *str;
{
short i;
short len;
short free_bytes;
STR_PTRS *stptrs;		/* pointer for string ptr struc */
extern short str_cnt;

if (str->str_cnt == 0)
    return(EI$SUCC);

free_bytes = (short) ((DD$BYTE *)str->stptpt - str_loc);

for (i = 0; i < free_bytes; i++)
    *(str_loc + i) = 0;

stptrs = str->stptpt;

while (str->str_cnt-- > 0)
    {
    *stptrs->std_str_ptr = (long) "<No room for string>"; /* Be pessimistic */
    len = strlen(stptrs->raw_string);
    if (stptrs->size != 0 && stptrs->size < len)
        len = stptrs->size;
    if (strncmp(stptrs->raw_string,trailer,4) == 0)
	len = 0;
    if (len < free_bytes)
        {
        (void)strncpy(str_loc, stptrs->raw_string, len);
        str_loc[len] = '\0';
        *stptrs->std_str_ptr = (long) str_loc;
        free_bytes -= ++len;   /* reduce free bytes - include null */
        str_loc += len;     /* update free pointer */
	}
    stptrs++;
    }
return (seg_buf->length = 	/* Save length and return it */
		 (long)(str_loc - (DD$BYTE *)seg_buf));
} 
/*...	ENDROUTINE SAVE_STRINGS() */

/*
*	.SBTTL	CHK_FLD
*++
* FUNCTIONAL DESCRIPTION:		
*
*    Called when a field in the raw or segment buffer must be validity-checked.
*    This routine dispatches to another routine which specializes in the
*    particular kind of validity (valid set, within range, etc).
*	
* CALLING SEQUENCE:		CALL CHK_FLD (..See Below..)
*                                   Called before transformation when
*                                   checking raw data and after to validate
*                                   standard.
*
* FORMAL PARAMETERS:		Field value
*                               Validity parameters
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* COMPLETION CODES:		Valid field (success)
*                               Invalid field (error)
*                               Bad validity parameters (fatal)
*
* SIDE EFFECTS:                 none
*--
*/
/*...	ROUTINE CHK_FLD ()					    */

long chk_fld (val_loc,valid_param)
char *val_loc;
int *valid_param;

{

/* If one of list, call routine to do that kind of validity */

/* If "within range", call routine to do that kind of validity */

/* If something else, do that too */

/* Return what we get from specific validity check routine */

    return DD$VALID;

} /*...	ENDROUTINE CHK_FLD					    */


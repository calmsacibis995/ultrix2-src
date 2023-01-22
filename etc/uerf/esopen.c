#ifndef lint
static char sccsid[]  =  "@(#)esopen.c	1.9   (ULTRIX)   12/16/86"; 
#endif  lint
/* DEC/CMS REPLACEMENT HISTORY, Element ESOPEN.C*/
/* *11    5-AUG-1986 10:38:16 ZAREMBA "test for null operands with es$select"*/
/* *10    9-JUN-1986 11:29:38 ZAREMBA "V1_1 changes"*/
/* *9    27-MAR-1986 10:45:33 ZAREMBA "removed DEBUG statements"*/
/* *8     4-MAR-1986 10:48:49 ZAREMBA "set selecttree to NULL"*/
/* *7    26-FEB-1986 10:54:35 ZAREMBA "fix ei$mailbox"*/
/* *6    12-FEB-1986 13:02:04 ZAREMBA "new version of erit"*/
/* *5    24-JAN-1986 17:22:46 ZAREMBA """terminated last comment"*/
/* *4    22-JAN-1986 13:03:17 ZAREMBA "add validate_ctx to moddsn"*/
/* *3    20-JAN-1986 16:44:40 ZAREMBA "remove _L from literals"*/
/* *2    17-JAN-1986 17:11:52 ZAREMBA "added debug statments"*/
/* *1    16-JAN-1986 11:31:44 ZAREMBA "ERMS open function"*/
/* DEC/CMS REPLACEMENT HISTORY, Element ESOPEN.C*/
/*
*	.TITLE	ERMSOPEN - ERMS open and close function for C call interfaces
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
* FACILITY:		[ ERMS - EIMS Record Management System ]
*
* ABSTRACT:
*
*	This module is the ERMS call interface to the OPEN and
*	CLOSE functions.
*	It can be called directly from a C program or as the
*	call from the pre-prosessor. 
*	
* ENVIRONMENT:	VAX/VMS C,  ULTRIX-32 C
*
* AUTHOR:  Don Zaremba,  CREATION DATE:  26-Nov-85
*
* MODIFIED BY:
*
*
*--
*/

#include <stdio.h>
#include "erms.h"
#include "select.h"
#include "eiliterals.h"


/*
*	.SBTTL	ERMS_OPEN - ERMS OPEN function
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This function initializes a particular stream for subsequent
*	operations. No other stream call is legal prior to the
*	successfull execution of this call.
*	
* CALLING SEQUENCE:		status = es$open(..See Below..)
*
* FORMAL PARAMETERS:		
*
*	stream_id		stream identifier
*	    [CTXBLK *stream_id]
*
*	logical_file		must be one of the following:
*	    [int logical_file]	    ES$CONFIG -- Configuration 
*						   information
*				    ES$EVENT -- event log
*				    ES$SUMMARY -- summarized 
*						    event data
*
*	option			must be one of the following:
*	    [int option]	    ES$VIEW -- (default) readonly access
*					     to database
*				    ES$MODIFY -- read and write access
*				    ES$CREATE - creates an ERMS database
*				    ES$MAILBOX - read events from mailbox
*
*	file_spec		optional pointer to a file spec. 
*	    [char *file_spec]	    If null then the system
*				    specific error log file is opened.
*				    Required if option = ES$CREATE
*
*	segments		address of each segment. Must be in the 
*				defined order; EIS,DIS,SDS,CDS,ADS,SIS,CIS.
*				
* IMPLICIT INPUTS: Pointers to the user segment buffer
*
* IMPLICIT OUTPUTS: Data in the users segment buffers is updated
*
* EXTERNAL REFERENCES:
*	    ERIT_OPEN
*
* FUNCTION VALUE:		integer value
*
*	ES$SUCC - OPEN completed successful
*	ES$STRM - undeclared stream id
*	ES$FNOF - File not found
*	ES$EXIS - CREATE option failed because file already exist
*	ES$FAIL - failed for unknown reason
*	
* SIDE EFFECTS:			Opens file specified and intializes
*	internal buffers. CREATE option creates a new file. 
*	
*	
*--
*/
/*...   FUNCTION es$open(stm,logical_file,option,file_spec,segments) */
long
es$open(stm,logical_file,option,file_spec,eis,dis,sds,cds,ads,sis,cis)
CTXBLK *stm;
short logical_file,option;
char *file_spec;
EIS *eis;
DIS *dis;
SDS *sds;
CDS *cds;
ADS *ads;
SIS *sis;
CIS *cis;
{
    long answer, validate_ctx();

    stm->eisflg = ((stm->eis_ptr = eis) != NULL); /* set segment allocated flag
					        and store segment addresses 
						in ctx block */
    stm->disflg = ((stm->dis_ptr = dis) != NULL);
    stm->sdsflg = ((stm->sds_ptr = sds) != NULL);
    stm->cdsflg = ((stm->cds_ptr = cds) != NULL);
    stm->sisflg = ((stm->sis_ptr = sis) != NULL);
    stm->cisflg = ((stm->cis_ptr = cis) != NULL);
    stm->adsflg = ((stm->ads_ptr = ads) != NULL);

    stm->logical_file = logical_file;
    stm->open_mode = ((option == ES$MAILBOX ) ? EI$MAILBOX : option);
    stm->file_spec = file_spec;
    stm->selecttree = NULL;

    answer = validate_ctx(stm);
    if (answer != ES$SUCC)
	return(answer);

    answer = ei$open(stm->file_spec,stm->open_mode); 
    if (answer == EI$OPN) return(ES$SUCC);
    if (answer == EI$FNX) return(ES$FNOF);
    return(ES$FAIL);
/*				*/
/*...	ENDFUNCTION ES$OPEN	*/
}

/*
*	.SBTTL	ERMS_SELECT - ERMS SELECT function
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This function passes selection criteria to ERMS.
*	Subsequent es$get calls will return only those logical
*	records that match these selection criteria.
*	The passed selection tree is passed to ERIT by a call
*       to the erit selection function. ERIT examines the tree
*	and decides if it can do any of the selection. For those
*	selection critria ERIT accepts they are removed from the
*	tree and thus ERMS will never see them. This allows ERIT
*	to change what selection it can do without modifying ERMS.
*
* FUNCTION DECLARATION
*	long es$select();
*	
* CALLING SEQUENCE:	status = es$select(strm,option,tree)
*
* FORMAL PARAMETERS:		
*
*	CTXBLK strm;	pointer to context block
*	short option;	must be one of the following:
*			    ES$REMEMBER -- (default) add these new
*			        selection criteria to the old
*			    ES$FORGET -- old selection criteria
*				are removed and only the new ones
*				applied.
*			    ES$NOT -- old selection criteria are
*				save but are preceeded by a NOT
*				opcode. This call effectively
*				negates all the previous selection
*				criteria. Later criteria are unaffected.
*
*	SELNODE *tree; pointer to selection tree
*
* IMPLICIT INPUTS: NONE
*
* IMPLICIT OUTPUTS: The context block is updated to contain the
*		    selection criteria.
*
* EXTERNAL REFERENCES:
*
* FUNCTION VALUE:		integer value
*
*	ES$SUCC - SELECT completed successful
*	ES$STRM - undeclared stream id
*	ES$FAIL - failed for unknown reason
*	
* SIDE EFFECTS:			
*   This call updates the internal ERMS structures that will later
*   be used to determine if a record should be returned by a later
*   es$get call. If the remember option is used then each new
*   selection criteria is AND'd with the old. AND's are left
*   associative and of higher presidence then OR's.
*   Passing the selection tree through ERIT may result in that
*   module accepting some selection criteria and thusly updating
*   its internal structures.
*	
*	
*--
*/
/* DEFFUN */
long
es$select (strm,option,selection)
CTXBLK *strm;
long option;
SELNODE *selection;
{
    SELNODE *node,*es$mkselectnode(),*ei$sel(),*firstinorder(),*nextinorder();
    short offset;
    short ftype;
    long retstat;

#ifdef DEBUG
    es$dumptree(selection); 
#endif DEBUG

    selection = ei$sel(option,selection);   /* see what erit wants */

/* Now visit the entire tree and fill in standard buffer addresses */

    if (selection != NULL) /* only load address if not NULL nodes */
    {
    node = firstinorder(selection); 
    do
      {
        if (node -> operands != NULL ) /* check for no operands */ 
          {
	    retstat = dd$flddata(node->operands->fldno,&ftype,&offset); /* get the offset */
	    if (node -> operands -> fldno < 29)		       /* is it in the eis or dis */
	       node -> operands -> varaddress = (char*) strm->eis_ptr + offset;
	    else
	       node -> operands -> varaddress = (char*) strm->dis_ptr + offset;
	   }
      }
    while ((node = nextinorder(node)) != NULL);
    }

    if (option == ES$FORGET)
	{
	    deletetree(strm -> selecttree); 
	    strm -> selecttree = selection;
	}
    else
    if (option == ES$REMEMBER)
        {
	    if (strm->selecttree == NULL)
		strm->selecttree = selection;
	    else
		if (selection != NULL) 
		   strm->selecttree = 
		     es$mkselectnode(ES$AND,strm->selecttree,selection,NULL);
	}
    else 
    if (option == ES$NOT)
        {
	 if (strm->selecttree == NULL)
	    {
	    if (selection != NULL)
	      strm->selecttree = es$mkselectnode(ES$NOT,selection,NULL,NULL);
	    }
	  else
	    {
	    if (selection != NULL)
	     strm->selecttree = es$mkselectnode(ES$NOT,
	     es$mkselectnode(ES$AND,strm->selecttree,selection,NULL),NULL,NULL);
	    else
	      strm->selecttree = es$mkselectnode(ES$NOT,strm->selecttree,NULL,NULL);
	    }
        }
    else
	return(ES$BADOPCODE);

#ifdef DEBUG
    es$dumptree(selection); 
#endif DEBUG
    return(ES$SUCC);
}



/*
*	.SBTTL	ERMS_CLOSE - ERMS close function
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This function closes the specifies ERMS stream. It optionally
*	releases any resources tied up in the stream. It must be
*	called once for each open.
*	
* CALLING SEQUENCE:		status = es$close(...See Below..)
*
* FORMAL PARAMETERS:		
*
*	stream_id		stream identifier
*	    [CTXBLK *stream_id]
*
*	option			 must be one of the following:
*	    [int option]	    ES$FORGET - (default) release all
*					    resources prior to closing
*				    ES$REMEMBER - do not release. The 
*					    selection criteria will
*					    be retained also.
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* FUNCTION VALUE:		integer value
*
*	ES$SUCC - CLOSE completed successfully
*	ES$STRM - undeclared stream
*	ES$FAIL - failed for unknown reason
*	
* SIDE EFFECTS:			Closes the specified stream. Must be
*	reopened before any other ERMS calls on that stream.
*	
*--
*/
/*...	FUNCTION es$close(stream_id,option)				    */
long
es$close(str,option)
CTXBLK *str;
short option;
{
long answer;
    answer = ei$close(str->open_mode);
    if (answer == EI$FCL) return(ES$SUCC);
    return(ES$FAIL);
/*									    */
/*...	ENDFUNCTION ERMS_CLOSE						    */
}

/*
*	.SBTTL	VALIDATE_CTX - validate the context block
*++
* FUNCTIONAL DESCRIPTION:		
*	This function validates the content of a context block
*	by checking the open mode and logicalfile.
*
*	status = validate_ctx(ctx)
*	    long status
*	    CTXBLK *ctx
*--
*/
long
validate_ctx(ctx)
CTXBLK *ctx;
{
    if ((ctx->open_mode != ES$VIEW) && 
	(ctx->open_mode != ES$MODIFY) &&
	(ctx->open_mode != ES$CREATE) && 
	(ctx->open_mode != EI$REVERSE) &&
	(ctx->open_mode != ES$MAILBOX) &&
        (ctx->open_mode != EI$MAILBOX)) 
	return(ES$OPENMODE);

    if ((ctx->logical_file != ES$EVENT) &&
	(ctx->logical_file != ES$SUMMARY) &&
	(ctx->logical_file != ES$CONFIG))
	return(ES$LOGICFILE);

    return(ES$SUCC);
}




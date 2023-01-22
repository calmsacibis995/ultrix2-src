#ifndef lint
static char sccsid[]  =  "@(#)eritio.c	1.9   (ULTRIX)   12/16/86"; 
#endif  lint

/*
*	.TITLE	ERITIO - I/O for Event Record Input Transformer
*	.IDENT	/1-001/
*
* COPYRIGHT (C) 1985 DIGITAL EQUIPMENT CORP.,
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
* DEC ASSUMES NO RESPONSIBILITY FOR THE USE OR RELIABILITY OF{* ITS SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DEC.
*
*++
*
* FACILITY:		[ FMA Software Tools - Detail Design ]
*
* ABSTRACT:
*
*	This module contains the functions to open, close and read 
*	raw system events from any operating system.{*	
*	
* ENVIRONMENT:	ULTRIX-32 C
*	
* AUTHOR:  Luis Arce,  CREATION DATE:  19-Nov-85
*
* MODIFIED BY:
*		Bob Winant  12-31-85
*
*--
*/

#include <stdio.h>
#include "erms.h"
#include "eims.h"
#include "eiliterals.h"
#include "select.h"

#define SEGHEADSIZ 8

extern short corrupt_flag;		/* value from ulfile.c */
extern long  rec_len;			/* value from ulfile.c */

static char *event_ptr = NULL;
/*
*	.SBTTL	EI$OPEN - Obtains access to raw error information
*++
* FUNCTIONAL DESCRIPTION:		
*
*	-  Selects user's input location (file or mailbox).
*	-  Opens file or mailbox.
*	
* FORMAL PARAMETERS:		
*
*	filename		Name of the file to be opened
*	mode			Mode to open the file (used as a flag to
*				open mailbox).
*		  
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		A raw system event file
*
* COMPLETION STATUS:		EI$OPN (file or socket open - success)
*				EI$NOP (file or socket not opened - failure)
*				EI$FNX (file doesn't exist - failure)
*
* SIDE EFFECTS:			Raw data will be accessible
*
*--
*/
/*...	ROUTINE ei$open (filename, mode)				*/
long ei$open (filename, mode)
   char *filename;
   short mode;
{
   return open_file(filename, mode);
}
/*...	ENDROUTINE EI$OPEN					*/
/*
*	.SBTTL	EI$CLOSE - Closes the error log file or socket
*++
* FUNCTIONAL DESCRIPTION:		
*
*	-  Closes the input file or socket
*	-  Does necessary clean-up
*	
* FORMAL PARAMETERS:		
*
*	mode			flag specifying whether using a mailbox
*				or not
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* COMPLETION STATUS:		EI$FCL (file or socket closed - success)
*				EI$FNC (file or socket not closed)
*
* SIDE EFFECTS:			Access to raw data ended
*
*--
*/
/*...	ROUTINE EI$CLOSE (mode)				*/
long ei$close (mode)
   short mode;
{
   return close_file(mode);
}
/*...	ENDROUTINE EI$CLOSE					*/
/*
*	.SBTTL	EI$GET - Gets a record from the file or socket
*++
* FUNCTIONAL DESCRIPTION:		
*
*	-  Reads a record from file or socket.
*	-  Transforms raw record to standard record
*	-  Updates File Control Block
*	-  Returns standard record or EOF
*	
* FORMAL PARAMETERS:		
*
*	eisptr			Pointer to an EIS
*	disptr			Pointer to a DIS
*	cdsptr			Pointer to a CDS
*	sdsptr			Pointer to a SDS
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* COMPLETION STATUS:		EI$SUCC - std buffers filled (success)
*				EI$RDR - error encountered
*					during read.
*				EI$EOF - end-of-file encountered (success)
*				EI$CORRUPT - corrupted/invalid entry
*					     found in raw event file
*				EI$FAIL - failure
*				EI$NULL - All ptrs point to null (failure)
*
* SIDE EFFECTS:			An EIMS standard event (minus any ADS's)
*				record will be created.
*
*--
*
*
* DEFINITIONS:
*
*	raw_pkt		Error log packet as read from the
*			error log file or from a socket
*
*	xform_rec	A raw_pkt that has been transformed
*
*	std_rec		One or more xform_rec(s) that are
*			merged to create the ERMS standard
*			record consisting of the standard
*			segments (EIS, DIS, SDS, CDS)
*
*
*				input		output
*				-----		------
*		OS				raw_pkt
*		xform		raw_pkt		xform_rec
*		xform_merge	xform_rec	std_rec
*								*/

/*...	ROUTINE ei$get (eisptr, disptr, cdsptr, sdsptr)	*/
long ei$get (eisptr, disptr, cdsptr, sdsptr)

EIS *eisptr;
DIS *disptr;
CDS *cdsptr;
SDS *sdsptr;
{

/************ Processing routine(s) and variables ***************/

extern long check_selection();
extern char *read_file();
extern void ei$bld();
extern void bld_corrupt_eis();
void zero_out();

long status;

if ((eisptr == NULL) && (disptr == NULL) &&
			(cdsptr == NULL) &&
			(sdsptr == NULL))
    return (EI$NULL);

for (;;)
    {
    event_ptr = read_file(&status);
    if (status != EI$SUCC)
	{
	if (status != EI$CORRUPT)
	    return(status);
	bld_corrupt_eis(eisptr);	/* build eis only           */
	disptr = NULL;
	cdsptr = NULL;
	sdsptr = NULL;
	return (EI$CORRUPT);
        }

    if ((status = check_selection()) == ES$SUCC)
        {				/* good rec - build segs   */
	zero_out(eisptr,sizeof(EIS));
	zero_out(disptr,sizeof(DIS));
	zero_out(cdsptr,sizeof(CDS));
	zero_out(sdsptr,sizeof(SDS));
	ei$bld(eisptr, disptr, cdsptr, sdsptr, event_ptr);
        return (EI$SUCC);
        }
    else if ((status == ES$EOF) || (status == EI$EOF))
	return (EI$EOF);
    }
}
/*...	ENDROUTINE EI$GET					*/

void zero_out (loc,len)
char   *loc;
short   len;
{

short i;
for (i = 0; i < len; i++)
    loc[i] = '\0';
}


/*
*	.SBTTL	EI$ADS_GET - Produces an ads from current event info
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine fills in the buffer space allocated by the caller
*	with any additional information contained in the event record
*	currently being processed.  If the raw event record being processed
*	was found to be corrupt, the adsptr is simply pointed to the actual
*	data, as there is nothing more to do.  Otherwise, the regular
*	segment build process is done.
*	
* FORMAL PARAMETERS:		
*
*	adsptr			Pointer to buffer space to load the
*				information into.
*
* IMPLICIT INPUTS:		A raw system event from which to
*				extract the information.
*
*				EIMS table definition(s) for the ADS
*				associated with the event.
*
* IMPLICIT OUTPUTS:		A complete EIMS standard event.
*
* COMPLETION STATUS:		EI$SUCC - Success always returned
*
* SIDE EFFECTS:			EIMS standard event record created.
*
*--
*/
/*...	ROUTINE EI$ADS_GET (adsptr)				*/
long ei$ads_get(adsptr)
ADS *adsptr;

{
extern long ads_bld();
long i, j, stat;
char *adstmp;

adstmp = (char *)adsptr;
adsptr->type = ES$ADS;
if (corrupt_flag == EI$TRUE)
    {
    adsptr->length = rec_len + ES$ADSVBA + SEGHEADSIZ;
    for (j = 0, i = (ES$ADSVBA + SEGHEADSIZ); i < rec_len; ++i, ++j)
        adstmp[i] = event_ptr[j];
    return(EI$SUCC);
    }
else
    stat = (ads_bld(adsptr, event_ptr));   
return (stat);
}
/*...	ENDROUTINE EI$ADS_GET					*/

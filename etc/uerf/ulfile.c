#ifndef lint
static char sccsid[]  =  "@(#)ulfile.c	1.12   (ULTRIX)   12/16/86"; 
#endif  lint

/*
*	.TITLE	ULFILE - Raw ULTRIX event file handler
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
* FACILITY:		[ FMA Software Tools - Detail Design ]
*
* ABSTRACT:
*
*	This module contains the functions to open, close and read 
*	either the error log file or a socket on an ULTRIX system.
*	
*	
* ENVIRONMENT:	ULTRIX-32 C
*	
* AUTHOR:  Bob Winant,  CREATION DATE:  30-Jan-1986
*
* MODIFIED BY:
*
*--
*/
#include "eiliterals.h"		/* Erit specific literals */
#include "ulliterals.h"		/* Ultrix specific literals */
#include "eims.h"
#include "erms.h"
#include "select.h"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/errlog.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <elwindow.h>

extern SELNODE *startree;
extern SELNODE *endtree;

extern char *ei$errtxt();    		/* error messages rtn */

/****************** Declare module wide globals ******************/

long in_file        = 0;		/* Input file descriptor */
char rec_buff[UL$BUFFSIZE];  		/* location to put record */
char   *trailer_id;			/* Pointer to check for valid trailer */

long   file_offset       =  0;
long   total_bytes       =  0;
long   rec_len           =  0;
struct el_rec *rec_ptr;			/* to directly access info in event */


short socket_flag   = EI$FALSE;
short reverse_flag  = EI$FALSE;
short corrupt_flag  = EI$FALSE;
short newfile_flag  = EI$TRUE;

struct options opts;		/* Options to be read for socket conn */
struct s_params params;		/* Parameters for socket connection */
struct sockaddr_un soc_from;  	/* Socket structure that info comes from */
long   socfrmlen;		/* Length of info returned from a socket read */

/***************************  FUNCTIONS  ***************************/

long  init_file();
long  validate_entry();
long  bld_socket();
struct el_rec  *soc_read();
struct el_rec  *get_next_rec();

/**********************  GET_BUFF_ADDR  ****************************/

struct el_rec *get_buff_addr()

{
return ((struct el_rec *)rec_buff);
}

/******************************************************************/




/*
*	.SBTTL	OPEN_FILE - Obtains access to raw error information
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
/*...	ROUTINE OPEN_FILE (filename, mode)				*/
long open_file (filename, mode)

char *filename;
short mode;

{

if (mode == EI$MAILBOX)
    {
    socket_flag = EI$TRUE;
    if ((in_file = bld_socket()) != EI$NSKT)
        return (EI$OPN);
    else
        {
	printf("\n%s\n", ei$errtxt(EI$, EI$NSKT));
	return (EI$FNX);
        }
    }
else
    {
    if (mode == EI$REVERSE)
        reverse_flag = EI$TRUE;
    if (strcmp(filename, "") != EI$FALSE)
        {
        if ((in_file = open(filename, O_RDONLY, 0)) >= 0)
            return (EI$OPN);
        else
            {
	    printf("\n%s: %s\n", ei$errtxt(EI$,EI$NOP), filename);
            return (EI$NOP);
            }
        }
    else
        {
	printf ("\n%s: %s\n", ei$errtxt(EI$,EI$FNX), filename);
	return (EI$FNX);
        }
    }
}
/*...	ENDROUTINE OPEN_FILE					*/

/*
*	.SBTTL	CLOSE_FILE - Closes the error log file or socket
*++
* FUNCTIONAL DESCRIPTION:		
*
*	-  Closes the input file or socket
*	-  Does necessary clean-up
*	
* FORMAL PARAMETERS:		
*
*	socket_flag		flag to specify socket
*
* IMPLICIT INPUTS:		Input file or socket in fcb_struc
*
* IMPLICIT OUTPUTS:		NONE
*
* COMPLETION STATUS:		EI$FCL (file or socket closed - success)
*				EI$FNC (file or socket not closed)
*
* SIDE EFFECTS:			File or socket will be closed and
*				become unavailable for processing.
*
*--
*/
/*...	ROUTINE CLOSE_FILE (mode)					*/
long close_file(mode)

long mode;

{
long status;

if (socket_flag)
    {
    status = close_wndw(in_file, &params);
    if (status != -1)
	status = 0;
    }
else
    status = close(in_file);

if (!status)
    return (EI$FCL);
else
    {
    printf("\n%s\n", ei$errtxt(EI$,EI$FNC));
    return (EI$FNC);
    }
}
/*...	ENDROUTINE CLOSE_FILE					*/


/*
*	.SBTTL	READ_FILE - reads an entry from the raw event file
*++
* FUNCTIONAL DESCRIPTION:		
*
*	Reads the specified input file or socket one record at a time
*	
* FORMAL PARAMETERS:		event_ptr - A pointer to be assigned to the
*					    read-in buffer.
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		A raw system event record
*
* COMPLETION STATUS:		EI$SUCC - Everything's cool
*				EI$RDR - error encountered
*					during read.
*				EI$EOF - End of file encountered
*				EI$FAIL - File somehow inaccessible
*				EI$CORRUPT - corrupt record found in
*					     raw event file.
*				
*
* SIDE EFFECTS:			NONE
*
*--
*/
/*...	ROUTINE READ_FILE ()				*/

struct el_rec *read_file(status)
long *status;

{
short i;

corrupt_flag = EI$FALSE;
		
if (socket_flag == EI$TRUE)		/* check for socket read  */
    rec_ptr = soc_read(status);
else
    {
    if (newfile_flag == EI$TRUE)
	{
	init_file();
        rec_len = 0;			/* initialize rec len	*/
	}
    rec_ptr = get_next_rec(status);
    }
    
if (*status != EI$SUCC)
    return (NULL);

if ((*status = validate_entry(rec_ptr)) == EI$FAIL)
    return (NULL);

for (i = rec_len; i < UL$BUFFSIZE; i++)
    rec_buff[i] = '\0';		/* zero out remainder of buffer */

if (*status == EI$CORRUPT)
    corrupt_flag = EI$TRUE;

return (rec_ptr);
}

/*...	END ROUTINE READ_FILE()				*/



/*
*	.SBTTL	GET_NEXT_REC   reads entry from the raw event file
*++
* FUNCTIONAL DESCRIPTION:		
*
*	Reads the specified input file for next record.
*	
* FORMAL PARAMETERS:
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		A raw system event record
*
* COMPLETION STATUS:		EI$SUCC - Everything's cool
*				EI$RDR - error encountered
*					during read.
*				EI$EOF - End of file encountered
*				EI$FAIL - File somehow inaccessible
*				EI$CORRUPT - corrupt record found in
*					     raw event file.
*				
*
* SIDE EFFECTS:			NONE
*
*--
*/
/*...	ROUTINE GET_NEXT_REC()				*/

struct el_rec *get_next_rec(status)
long *status;

{
short i, j;
long len;

if (reverse_flag != EI$TRUE)

/*******************  FORWARD READ OF FILE ******************/

    {
    file_offset += rec_len;
    lseek(in_file,file_offset,L_SET);
    if (read(in_file,rec_buff,UL$BUFFSIZE) == 0)
	{
	*status = EI$EOF;
	return(NULL);
	}
    }
else
    
/***********************  REVERSE READ OF FILE ******************/
    
    {
    if (file_offset <= 0)
	{
	*status = EI$EOF;
	return(NULL);
	}
    if (file_offset >= UL$BUFFSIZE)
	{
	len          = UL$BUFFSIZE;
	file_offset -= UL$BUFFSIZE;
	}
    else
	{
	len         = file_offset;
	file_offset = 0;
	}
    lseek(in_file, file_offset, L_SET);
    if (read(in_file, rec_buff, len) != len)
	{
	*status = EI$RDR;
	return(NULL);
	}
    if (strncmp(rec_buff+(len - UL$TRSIZ),trailer,UL$TRSIZ) != 0)
	{
	*status = EI$RDR;
	return(NULL);
	}
    for(i = len - (UL$TRSIZ + 1); i > 0; i--)
	{
        if (strncmp(rec_buff+i,trailer,UL$TRSIZ) == 0)
	    break;
	}
    if (i == 0)
	{
	if (file_offset != 0)	/* cannot find start of rec */
	    {
	    *status = EI$FAIL;
	    return(NULL);
	    }
	}
    else
	{
	i += UL$TRSIZ;
	len -= i;			/* calculated rec len	*/
	for (j = 0; j < len; j++)
	    {
	    rec_buff[j] = rec_buff[i+j];
	    }
	}
    file_offset += i;
    }
*status = EI$SUCC;
return ((struct el_rec *)rec_buff);
}

/*... END ROUTINE GET_NEXT_REC 				*/




/*
*	.SBTTL	VALIDATE_ENTRY - Validates the current raw record
*++
* FUNCTIONAL DESCRIPTION:		
*
*	This routine checks the expected trailer on the end of the
*	raw record to see if a valid entry exists.  It also checks the
*	validation location for the record to see if that is also valid.
*	
* FORMAL PARAMETERS:		
*
*	event_ptr		A pointer to current raw ULTRIX event.
*
* IMPLICIT INPUTS:		A raw event record
*
* IMPLICIT OUTPUTS:		NONE
*
* COMPLETION STATUS:		EI$TRUE (good record)
*				EI$CORRUPT (record has valid trailer
*					but invalid value in rhdr_valid)
*				EI$FAIL (invalid record, and unable
*					to resync)
*
* SIDE EFFECTS:			Return value used by calling
*				routine to decide what to do next
*
*--
*/
/*...	ROUTINE VALIDATE_ENTRY(event_ptr)				*/

long  validate_entry(event_ptr)

struct el_rec *event_ptr;

{
short i;				/* loop ctr */
long   status;

status = EI$SUCC; 			/* assume good record */

/*************** First, make sure it is marked valid *************/

if (event_ptr->elrhdr.rhdr_valid != EL_VALID)
    status = EI$CORRUPT;

/****************** Now check for a valid trailer *****************/

rec_len = rec_ptr->elrhdr.rhdr_reclen;
if (strncmp(((char *)event_ptr)+(rec_len-UL$TRSIZ),trailer,UL$TRSIZ) != 0)
    {
    status = EI$FAIL;
    for (i = 0; i < UL$BUFFSIZE; ++i)
        {
        if (strncmp(rec_buff+i,trailer,UL$TRSIZ) == 0)
            {
            rec_len = i+UL$TRSIZ;
	    status = EI$CORRUPT;
	    break;
            }
        }
    }
return(status);
}
/*...	ENDROUTINE VALIDATE_ENTRY				*/



/*
*	.SBTTL	INIT_FILE       Initialize pointers for read by date.
*++
* FUNCTIONAL DESCRIPTION:		
*
*	Returns the file_offset to the first record (forward or rev)
*	
* FORMAL PARAMETERS:		NONE
*
* IMPLICIT INPUTS:		Module wide declarations for rec_buff,
*				flags, and other integers necessary
*				for keeping track of the possition in
*				the file and direction.
*
* IMPLICIT OUTPUTS:		file-offset
*
* COMPLETION STATUS:		NONE
*
* SIDE EFFECTS:			NONE
*
*--
*/

/*...  ROUTINE  INIT_FILE()				*/

long  init_file()

{

short  i;
long   lo;
long   hi;
long   offset;

newfile_flag = EI$FALSE;

lo = 0;
hi = lseek(in_file, 0, L_XTND);		/* get file size	*/

if (reverse_flag == EI$TRUE)
    {
    file_offset = hi;
    if (endtree == NULL)
	return;
    }
else
    {
    file_offset = lo;
    if (startree == NULL)
	return;
    }

/*****************  DO BINARY SEARCH TO START  ******************/

while ( ((hi - lo)/2) > UL$BUFFSIZE)
    {
    offset = lo + ((hi - lo)/2);
    lseek(in_file, offset, L_SET);	/* move ptr to mid range	*/
    if (read(in_file, rec_buff, UL$BUFFSIZE) != UL$BUFFSIZE)
        return;			/* cannot split any smaller     */

    for (i = 0; i < UL$BUFFSIZE; i++, offset++)
        {
        if (strncmp(rec_buff+i,trailer,UL$TRSIZ) == 0)
	    break;
        }

    if (i == UL$BUFFSIZE)
        return;			/* cannot split any smaller     */
    i      += UL$TRSIZ;
    offset += UL$TRSIZ;

    lseek(in_file, offset, L_SET);	/* move ptr to rec start */
    if (read(in_file, rec_buff, UL$BUFFSIZE) != UL$BUFFSIZE)
        return;			/* cannot split any smaller     */

    rec_ptr = (struct el_rec *)rec_buff;

    if (validate_entry(rec_ptr) != EI$SUCC)
        return;			/* cannot split any smaller     */

    if (reverse_flag == EI$TRUE)
        {
        if (es$eval(endtree) == ES$SUCC)
            {
	    lo = offset;
	    }
        else
	    {
	    hi = offset;
	    file_offset = offset;
	    }
        }
    else
        {
        if (es$eval(startree) == ES$SUCC)
            {
	    hi = offset;
	    }
        else
	    {
	    lo = offset;
	    file_offset = offset;
	    }
        }
    }

/*****************  FINISHED BINARY SEARCH  *********************/

return;
}

/*...  END ROUTINE  INIT_FILE()			*/



/*
*	.SBTTL	BLD_SOCKET - Creates socket path to OS
*++
* FUNCTIONAL DESCRIPTION:		
*
*	Opens a path to the OS buffer for a constant read and
*	translation on wakeup
*	
* FORMAL PARAMETERS:		NONE
*
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		NONE
*
* COMPLETION STATUS:		sock_id - socket descriptor
*				EI$NSKT - failed to open socket
*
* SIDE EFFECTS:			Path to OS opened, opens processing
*				directly from os, continues until user
*				stops it.
*
*--
*/
/*...	ROUTINE BLD_SOCKET ()				*/
long bld_socket()

{
long sock_id;		/* Receives return value from socket open */

/******** Set up options to ALL * (defined in window.h by UEG) ****/
 
opts.class	= ALL;
opts.type	= ALL;
opts.ctldevtyp	= ALL;
opts.num	= ALL;
opts.unitnum	= ALL;

sock_id = open_wndw(&opts, &params);		/* Open socket connection */
if (sock_id != EI$RDR)
    return (sock_id);
else
    return (EI$NSKT);
}
/*...	ENDROUTINE BLD_SOCKET				*/


/*
*	.SBTTL	SOC_READ - reads an entry from a socket
*++
* FUNCTIONAL DESCRIPTION:		
*
*	Reads the specified socket one record at a time
*	
* FORMAL PARAMETERS:		NONE
*
*
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		A raw system event record
*
* COMPLETION STATUS:		EI$TRUE - Everything's cool
*				EI$RDR - error encountered
*					during read.
*				EI$CORRUPT - corrupt record found
*				
*
* SIDE EFFECTS:			Process goes into a wait state
*				until a response is received from
*				the called socket.
*
*--
*/
/*...	ROUTINE SOC_READ ()					*/
struct el_rec  *soc_read(status)
long *status;

{
long   bytes_read;
int    done = 0;

/********** Get a record from the socket connector ***************/

while (!done)
    {
    bytes_read = read_wndw(in_file, rec_buff, UL$BUFFSIZE);

    if (bytes_read == 0)		/* 0 => exit; */
	{
	*status = EI$FAIL;
        return(NULL);
	}
    if (bytes_read == EI$RDR)
        {
        if (errno != EINTR)
	    {
	    *status = EI$FAIL;
            return (NULL);		/* return if not */
	    }
        else 				/* otherwise, try again */
            {
	    errno = 0;
	    continue;
	    }
        }
    done = 1;
    }
*status = EI$SUCC;
return((struct el_rec *)rec_buff);
}  
/*...	ENDROUTINE SOC_READ					*/


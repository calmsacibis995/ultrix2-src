#ifndef lint
static char sccsid[]  =  "@(#)uerf.c	1.11   (ULTRIX)   2/12/87"; 
#endif  lint

/*
*	.TITLE	UERF - ULTRIX Error Report Formatter (UERF)
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
* DEC ASSUMES NO RESPONSIBILITY FOR THE USE OR RELIABILITY OF
* ITS SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DEC.
*
*++
*
* FACILITY:		[ FMA Software Tools - Detail Design ]
*
* ABSTRACT:
*
*	UERF is a process that is used to translate binary log events
*       for certain devices under ULTRIX-32 V2.0 +
*	into a human readable format.
*	
* ENVIRONMENT:	ULTRIX-32 C
*	
* AUTHOR:  Luis Arce,  CREATION DATE:  19-Nov-85
*
* MODIFIED BY:
*
*
*--
*/

#include "uerror.h"
#include "eiliterals.h"
#include "ueliterals.h"
#include "uestruct.h"
#include "btliterals.h"
#include "erms.h"
#include "generic_dsd.h"
#include "std_dsd.h"
#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>


/*
*++
*
*    UERF MODULE HIERARCHY
*
*
*
*
*                                  +-------+
*                                  |       |
*                                  | UERF  |
*                                  |       |
*                                  +---+---+
*                                      |
*        +---------+---------+---------+---------+------+
*        |         |         |         |         |      |
*     +--+--+   +--+--+   +--+--+   +--+--+   +--+--+   |
*     |parse|   |valid|   | dsd |   |valid|   | bt  |   |
*     | cmd |   |tool |   |init |   | gen |   |open |   |
*     +-----+   +-----+   +-----+   +-----+   +-----+   |
*                                                       |
*              +----------------------------------------+
*              |
*              +---+---------+---------+---------+---------+---------+
*                  |         |         |  -ads-  |         |         |
*               +--+--+   +--+--+   +--+--+   +--+--+   +--+--+   +--+--+  
*               | es  |   | bt  |   | es  |   | bt  |   | bt  |   | es  |
*               | get |   | put |   | get |   | put |   |close|   |close|
*               +-----+   +-----+   +-----+   +-----+   +-----+   +-----+   
*
*
*
*
*
*
*--
*/


/*
*	.SBTTL	UERF - ULTRIX-32 Error Report Formatter
*++
* FUNCTIONAL DESCRIPTION:		
*
*	-  Structures are initialized.
*	-  Input command line is parsed and validated.
*	   Input location is selected and opened.
*	   Selection criteria is verified.
*	-  Output file is opened.
*	-  Raw (binary) records are read from input.
*	   Raw records are transformed to standard ERMS segments.
*	   Selection criteria is applied.
*	-  Standard ERMS segments are formatted for output.
*	   Formatted output is written out to std_out device.
*	-  Close and termination process.
*	
* FORMAL PARAMETERS:		
*
*	argc			Number of command line arguments
*	argv			Command line arguments
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
/*...	PROGRAM UERF (argc,argv)				*/

ES$DECLARE(,EIS,ueis);
ES$DECLARE(,DIS,udis);
ES$DECLARE(,SDS,usds);
ES$DECLARE(,CDS,ucds);
ES$DECLARE(,ADS,uads);
ES$STREAM(,uctx);
short kernel_ctl_c;		/* flag to bypass 1st ctl_c if kernel */
short ctl_c;			/* flag for CTL C	*/
struct in_struc     in_st;	/* allocate in  struct	*/
struct segment_info seg_info;

char  err_file[UE$XFF];
char  hlp_file[UE$XFF];

/***************************************************************/

main (argc,argv)
short argc;
char  *argv[];
{
long  stop_uerf();
int   status;
short offset;
short i;
char  search_path[UE$XFF];
char uerf_source[UE$XFF];
static short entno = 0;
static char star[] = "******************************";
struct stat buf;

printf("\t\t\t\t\t\t %s\n",UE$VER_NUM);

kernel_ctl_c = FALSE;

if (strlen(argv[0]) > 4)
    {
    strncpy(search_path,argv[0], strlen(argv[0])-5);
    strcat(search_path,":");
    }
strcat(search_path,"/etc:");
strcat(search_path,getenv("PATH"));
for (offset = 0, i = 0; offset < strlen(search_path); offset++)
    {
    if (search_path[offset] != ':')
	{
	err_file[i++] = search_path[offset];
	}
    else
	{
	err_file[i++] = '/';
	err_file[i] = '\0';
	strcpy(uerf_source,err_file); strcat(err_file,UE$ERR_FILE);
	if ((status = stat(err_file,&buf)) == 0)
	    break;
	i = 0;
	}
    }
for (offset = 0, i = 0; offset < strlen(search_path); offset++)
    {
    if (search_path[offset] != ':')
	{
	hlp_file[i++] = search_path[offset];
	}
    else
	{
	hlp_file[i++] = '/';
	hlp_file[i] = '\0';
	strcpy(uerf_source,hlp_file); strcat(hlp_file,UE$HLP_FILE);
	if ((status = stat(hlp_file,&buf)) == 0)
	    break;
	i = 0;
	}
    }

if ((status = parse_cmd(argc,argv)) != UE$SUCC) /* parse command  */
    {
    if (status == UE$DONE)
	exit(0);		/* successfull return code   */
    print_err(status);
    exit(1);			/* unsuccessfull return code */
    }

if ((status = valid_tool()) != UE$SUCC) /* validate uerf specific */
    {
    print_err(status);
    exit(1);			/* unsuccessfull return code */
    }

dsd_init();

printf("\n");

if ((status = valid_gen()) != UE$SUCC) /* validate generic input */
    {
    print_err(status);
    exit(1);			/* unsuccessfull return code */
    }

if (in_st.kernel)
    {
    kernel_ctl_c = TRUE;
    signal(SIGINT,stop_uerf);
    signal(SIGTERM,stop_uerf);
    }

if ((status = bt$open()) != BT$SUCC)		/* init/open bit to text*/
    {
    print_err(status);
    exit(1);			/* unsuccessfull return code */
    }

while (((status = (es$get(&uctx,ES$NEXT,NULL,NULL,NULL,NULL,NULL,NULL)))
	!= ES$EOF) && (ctl_c != UE$STOP))
    {
    if (in_st.out_form != UE$OUT_TERSE)
	printf("\n%s ENTRY  %5d %s", star, ++entno, star);
    if (seg_info.eis_flag != TRUE)
        {
	printf("\nUnknown record, raw record class = %d, type = %d\n",
			seg_info.raw_subid_class,
			seg_info.raw_subid_type);
	}
    else if (ueis.eventclass == EI$CORRUPT)
	{
	print_err(UE$ERR_CORUPT);
	if ((es$getads(&uctx,&uads)) == ES$SUCC)
	    {
	    hex_dump(&uads,uads.length,4,0);
	    }
	}
    else if (status == ES$FAIL)
	{
	printf("\nUnable to continue processing - Failure on read.\n");
	break;
	}
    else
	{
	if (in_st.out_form != UE$OUT_TERSE)
	    bt$put(&ueis);		/* pass EIS to BTT	*/
	if (seg_info.dis_flag == TRUE)
	    {
	    if (in_st.out_form == UE$OUT_TERSE)
		put_terse(&ueis,&udis);
	    else
		bt$put(&udis);		/* pass DIS to BTT	*/
	    if (seg_info.cds_flag == TRUE)
		{
		bt$put(&ucds);		/* pass CDS to BTT	*/
		if (in_st.out_form != UE$OUT_BRIEF)
		    if (seg_info.sds_flag == TRUE)
			bt$put(&usds);	/* pass SDS to BTT	*/
		}
	    }
	if (in_st.out_form != UE$OUT_BRIEF)
	    {
	    while ((es$getads(&uctx,&uads)) == ES$SUCC)
		{
		bt$put(&uads);		/* pass ADS to btt	*/
		}
	    }
	}
    printf("\n");
    }

bt$close();				/* terminate BTT	*/

if (ctl_c != UE$STOP)
    es$close(uctx,ES$FORGET);		/* terminate ERMS	*/

printf("\n");
exit(0);			/* successfull return code */
}
/*...	ENDPROGRAM UERF						*/



/*
*	.SBTTL	STOP_UERF - function used when ctl c is entered
*++
* FUNCTIONAL DESCRIPTION:		
*
*	-  this is an asyncronous function defined by "signal".
*	   it is entered on the signals SIGINT or SIGTERM.
*
* FORMAL PARAMETERS:		
*
*
* IMPLICIT INPUTS:		NONE
*
* IMPLICIT OUTPUTS:		ctl_c is set to UE$STOP
*
* COMPLETION STATUS:		NONE
*
* SIDE EFFECTS:			NONE
*
*--
*/
/*...	FUNCTION STOP_UERF       			*/

long  stop_uerf()
{

extern short kernel_ctl_c;

/*
extern short ctl_c;

if (kernel_ctl_c)
    kernel_ctl_c = FALSE;
else
*/
    {
    es$close(uctx,ES$FORGET);
    ctl_c = UE$STOP;
    }

}
/*...   ENDFUNCTION STOP_UERF       			*/

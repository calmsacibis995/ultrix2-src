/* 
 * rccopy.c
 */

#ifndef lint
static	char	*sccsid = "@(#)rccopy.c	1.5	(ULTRIX)	4/22/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984, 1986 by			*
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
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************
 *
 *			Modification History
 *
 *	Stephen Reilly, 04-Feb-85
 * 003- More modification to the prompt messages
 *
 *	Stephen Reilly, 21-Jan-85
 * 002- Modified prompt and error messages.
 *
 *	Stephen Reilly, 27-Oct-84
 * 001- Added the restore logic and modified some of the prompting
 *	messages.
 *
 *	Stephen Reilly, 18-Jul-84
 * 000- Module creation.
 *
 ***********************************************************************/
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/time.h"
#include "../h/gnode.h"
#include "../h/devio.h"

#include "savax.h"
#include "saio.h"

/*
 * Parameters for the communications area
 */
#define	NRSPL2	0
#define	NCMDL2	0
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)

#include "../vaxuba/udareg.h"
#include "../vaxuba/ubareg.h"
#include "../vax/mscp.h"

#define TOP_DRIVE 1
#define BOTTOM_DRIVE 0

/*
 * Facility:
 *
 *	Standalone backup image copy routine for rc25 disk drives
 *
 * Abstract:
 *
 *	Since the rc25 is a single spindle one removable pack it became
 *	necessary for an interactive program to backup both disk packs.
 *	This program attempts to backup both disk packs in an interactive
 *	env.  Note: Currently this program will only work standalone.
 *
 * Author:
 *	
 *	Stephen Reilly
 */

/*
 *	Questions use to prompt for the user's backup action.
 */

char *b_question[] =
    {  "  ",
       "\nLoad bottom scratch cartridge into top unit.\n",
       "\Reload original top cartridge into top unit.\n",
       "\nLoad top scratch cartridge into top unit.\n",
       "\nReload bottom scratch cartridge into top unit.\n",
	0
    };

/*
 *	Questions used to prompt user for restoring bottom disk
 */
char *r_b_question[] =
    {  " ",
       "\nLoad bottom backup cartridge into top unit.\n",
	0
    };

/*
 *	Questions used to prompt user to restoring top disk
 */
char *r_t_question[] =
    {  " ",
       "\nLoad bottom scratch cartridge into top unit.\n",
       "\nLoad top backup cartridge into top unit.\n",
       "\nReload original top cartridge into top unit.\n",
       "\nReload bottom scratch cartridge into top unit.\n",
	0
    };

/*
 *	The number of disk blocks per pack.  This information is stored
 *	by the uda standlone driver.
 */
extern uda_drive_size;

/*
 *	This struct is a duplicate of the one found in the uda driver. If
 *	it changes this one must also.  The reason of this struct is so
 *	we can set the unit number that the uda driver will use when
 *	mscp commands are given.
 */
extern struct uda {
		struct udaca	uda_ca;
		struct mscp	uda_rcp;
		struct mscp	uda_cmd;
} uda;		   /* Uda struct for mscp packets */

/*
 *	A struct to keep track on the unit number we are working with
 */
int unit[2];
int *unit_ptr;

main()
{
	register int rcc, wcc;	   /* read char count & write char count */
	register int record;	   
	int top, bottom;	   /* file desc for the top and bottom drives*/
	int tmp, whole_records,	   
	    partial_record;
	char sbuf[50];		   /* buffer for full devices name */
	char buffer[49152];	   /* 48k transfer buffer */
	char **p;		   /* point to string of question asked */
	int error_flag = 0;	   /* 001 did error occur during copying */
	struct mscp *udcmd();	   /* Uda drive routine */
	int backup = 0;		   /* 003 is this a backup */

	unit_ptr = unit;	   /* Initialize the pointer */

	/*	
	 *	Open the bottom and top drives
	 */
	top = getdev("Top (removable) unit number",sbuf, 2, TOP_DRIVE);
	bottom = getdev("Bottom (fixed) unit number", sbuf, 2, BOTTOM_DRIVE);
	/*
	 *	Determine what procedure the user wants
	 */
	printf("\nIs this a backup or restore copy? (b or r) ");
	gets(sbuf);

	/*
	 *	Check user's response
	 */
	if ( strcmp(sbuf, "r") == 0 ) {

	    /*
	     *	User wants to restore, but we must now ask which
	     *  disk is to be restored
	     */
    	    printf("Restore copy top or bottom unit? (t or b) ");
	    gets(sbuf);
	    if ( strcmp(sbuf, "t") == 0 ) {
		printf("\nBeginning restore copy of top unit.\n");
		p = r_t_question;
	    }
	    else {
		printf("\nBeginning restore copy of bottom unit.\n");

		/*
		 *	Because of the copy logic the file desc must
		 *	be reversed
		 */
		tmp = top;
		top = bottom;
		bottom = tmp;
		p = r_b_question;
	    }
	}
	else {
	    printf("\nBeginning backup copy of top and bottom units.\n");
	    backup = 1;		/* 003 indicate a backup */
	    p = b_question;
	}

	/*
	 *	Compute the number of whole records that can be copied
	 */
	whole_records = (uda_drive_size*512)/sizeof(buffer);
	
	/*
	 *	Loop through each of the questions
	 */
        while ( *++p) {

		/*
		 *	Prompt user's action and wait until ready
		 */
		if ( error_flag == 0 ) 			/* 001 */
		    printf("%s",*p);			/* 001 */
		else
		    error_flag = 0;			/* 001 */

		printf("When ready, press the <RETURN> key. ");
		gets(sbuf);

		/*
		 *	With the possiblity of the user turning on and off
		 *	the drive we must put it one line.  Simple calling
		 *	the open routine will not work because the uda 
		 *	driver keeps track of what units were put one line.
		 *	If it believes it was already online it will not
		 *	reissue the online command.
		 */
		uda.uda_cmd.mscp_unit = unit[0] & 7;	/* Unit we want */
		udcmd(M_OP_ONLIN);			/* Put online */
		uda.uda_cmd.mscp_unit = unit[1] & 7;	/* Unit we want */
		udcmd(M_OP_ONLIN);			/* Put online */

		/*
		 *	Get the unit status one of the drives and make sure 
		 *	all is well
		 */
		if ( udcmd(M_OP_GTUNT) == 0 )
		    {
		    printf("Unable to get status info from disk controller.\n");
		    goto error;
		    }

		printf("Starting copy\n");
		/*
		 *	Do the copying
		 */
		for (record = whole_records; record > 0 ; record--) {
			
			rcc = read(bottom, buffer, sizeof (buffer));

			/*
			 *	Check for any read errors
			 */
			if (rcc < 0) {
				printf("Record %d: read error\n",
					whole_records - record);
				goto error;
			}

			/*
			 *	Check for any short records ( We shouldn't get 
			 *	any.
			 */
			if (rcc < sizeof (buffer) || rcc == 0) {
				printf("Record %d: read short; expected %d, got %d\n",
					whole_records - record, sizeof (buffer), rcc);
				goto error;
			}

			wcc = write(top, buffer, rcc);

			/*
			 *	Check for any write errors
			 */
			if (wcc < 0) {
				printf("Record %d: write error\n",
					whole_records - record);
				goto error;
			}

			/*
			 *	Check for any short writes
			 */
			if (wcc < rcc) {
				printf("Record %d: write short; expected %d, got %d\n",
					whole_records - record, rcc, wcc);
				goto error;
			
			}
		}

		
		/*
		 *	We have written all of the whole records, now see if we
		 *	need to write out a partial record
		 */
		 if ( (partial_record = 
			((uda_drive_size * 512) % sizeof(buffer))) != 0 )
		    {
	    	    rcc = read(bottom,buffer, partial_record );
		    /*
		     *	Check for any read errors
		     */
		    if (rcc < 0) {
			printf("Record %d: read error\n",
				++whole_records);
			goto error;
		    }

		    /*
		     *	Check for any short records ( We shouldn't get 
		     *	any.
		     */
		     if (rcc < partial_record || rcc == 0) {
			    	printf("Record %d: read short; expected %d, got %d\n",
				++whole_records, partial_record, rcc);
    				goto error;
		     }

		     wcc = write(top, buffer, rcc);

		     /*
		      *	Check for any write errors
		      */
		     if (wcc < 0) {
			printf("Record %d: write error\n",
				whole_records);
			goto error;
		     }

		     /*
		      *	Check for any short writes
		      */
		     if (wcc < rcc) 
			 {
			 printf("Record %d: write short; expected %d, got %d\n",
				whole_records, rcc, wcc);
			 goto error;
		
		         }
		     }

		printf("Copy completed\n\n");

		/*
		 *   Rewind disk and flip flop the file descriptors for
		 *   the next copy.
		 *    
		 */
		lseek(top,0,0);
		lseek(bottom,0,0);
		tmp = top;
		top = bottom;
		bottom = tmp;		
		continue;

		/*
		 *	An error occurred during the copy. Asked user if he
		 *	want to restart, if so then do so else exit
		 */
error:
		printf("An error occurred, do you want to restart to previous action? (y or n) ");
		gets(sbuf);
		if( strcmp(sbuf,"y") == 0)
		    {
		    /*
		     *	Backup to previous question
		     */
		    --p;

		    /*
		     *   Rewind disk 
		     *    
		     */
		    lseek(top,0,0);
		    lseek(bottom,0,0);
		    error_flag = 1;		/*001 indicate error */
		    continue;
		    }		    
		else
		    exit(1);		/* Exit with error */
	} 

	/*
	 *	Print the finished messages depending on the operation
	 *	we did
	 */
	if ( backup )
		printf("Backup Completed!\n");
	else
		printf("Restore Completed!\n");

	exit(0);			/* Exit */
}

getdev(prompt, buf, mode,flag)
	char *prompt, *buf;
	int mode,flag;
{
	register int i;
	char unit_buf[10],*strcat();

	do {
		/*
		 *	Solicit device number
		 */
		printf("%s: ", prompt);
		gets(unit_buf);
		/*
		 *	Get numeric value of unit number
		 */
		*unit_ptr = atol(unit_buf);

		/*
		 *	If top drive make sure that unit number is even
		 */
		if ( (flag) && ( (*unit_ptr & 1 ) != 0 ) )
		    {
		    printf("Top unit must be an even number.\n");
		    i = -1;
		    }
		else
		    {
		    /*
		     *	If bottom drive make sure that unit number is odd.
		     */
		     if ( (!flag) && (*unit_ptr & 1 ) != 1 ) 
			{
			printf("Bottom unit must be an odd number.\n");
			i = -1;
			}
		    else
			{
			/*
		 	 *	The user specified unit number is okey 
		 	 *	then try to open the file.
	 	 	 */
			strcpy(buf,"ra(");
			buf = strcat(buf,strcat(unit_buf,",2)"));
			i = open(buf, mode);
			}
		    }
	} while (i <= 0);
	unit_ptr++;
	return (i);
}

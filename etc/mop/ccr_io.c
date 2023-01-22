#ifndef lint
static	char	*sccsid = "@(#)ccr_io.c	1.1	(ULTRIX)	9/30/85";
#endif lint

/*
 * MODULE - CCR_IO.C, handles i/o
 *
 * Copyright (C) 1985 by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * 1.00 10-Jul-1985
 *      DECnet-ULTRIX   V1.0
 *
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "ccr.h"
#include "ccr_mesg_codes.h"
#include "ccr_mesg_text.h"
#include "mop_var.h"

char 	break_flag = 0,
		done_read = 1;
u_char	char_buffer[MAXL_CMD_SIZE];
int		char_buf_len;

extern char done_processing;
extern struct sys_id_info sysid_values;

extern int 	ccr_errno;
extern int errno;

/*
 *   p r i n t _ m e s s a g e
 *
 * Called to output message to standard error.
 *
 * Inputs:
 *		msg_code	message code number
 *
 * Outputs: 
 *		None
 *
 * Notes:
 *		None.
 */

print_message(msg_code)
char msg_code;
{

    /*
     * Test to make sure the message code doesn't exceed the defined
     * number of messages. And print appropriate message.
     */
    if (msg_code >= num_ccr_mesgs)
    	fprintf(stderr,"ccr: fatal error, invalid error code %d",msg_code);
    else
    	fprintf(stderr,"ccr: %s\n",ccr_msglist[msg_code]);


}

/*
 *  	r e a d _ u n s o l _ c h a r s
 *
 *	Read unsolicited characters from stdin, when typed by the user.
 *
 * Inputs:
 *		None.
 *
 * Outputs: 
 *		None.
 *
 */

void read_unsol_chars()
{
	char	cc;
	long charcount;

	/*
	 * Set up flag to indicate read processing going on
	 */
	done_read = 0;
	no_eintr(ioctl(0,FIONREAD,&charcount));

	while (charcount--)
	{
		no_eintr(read(0,&cc,1));
		switch (cc) {
	
			case CTRL_B:
				/*
				* Need to flush the current message buffer
				* before indicating ctrl-b occurred.
				* Indicate that read has completed so that
				* message can be sent.
				*/
				done_read = 1;
				send_ccpoll();
				/*
				* Set break flag and reset flag indicating read is 
				* continuing.
				*/
				break_flag = 1;
				done_read = 0;
				break;

			case CTRL_D:
				ccr_errno = SUCCESS;
				done_processing = 1;
				break;

			default:
				/*
				* if number of characters is exceeding maximum
				* specified in the system id message, flush the
				* current buffer first
				*/
				if (sysid_values.max_cmd_size == 0)
				{
					if (char_buf_len >= MAXL_CMD_SIZE)
					{
						done_read = 1;
						send_ccpoll();
						done_read = 0;
					}
				}
				else
				{
					if (char_buf_len >= sysid_values.max_cmd_size)
					{
						done_read = 1;
						send_ccpoll();
						done_read = 0;
					}
				}
				char_buffer[char_buf_len] = cc;
				char_buf_len++;
				break;
		}
	}
	done_read = 1;

}

/*
 *	p r i n t _ l o a d _ e r r o r
 *
 * Called to output a load error message to standard error.
 *
 * Inputs:
 *		err_code	error code number
 *
 * Outputs: 
 *		None.
 */

print_load_error(err_code)
char err_code;
{

	switch ( err_code )
	{
		case EXIT_DBACCESSFAIL:
			fprintf(stderr, "ccr: can't access entry in nodes data base\n");
			break;

		case EXIT_SERDISABL:
			fprintf(stderr, "ccr: not permitted\n");
			break;

		case EXIT_NODBKEY:
			fprintf(stderr, "ccr: no data base key\n");
			break;

		case EXIT_BADEVNAME:
			fprintf(stderr, "ccr: bad device name\n");
			break;

		case EXIT_DLICONFAIL:
			fprintf(stderr, "ccr: unable to make connection\n");
			break;

		case EXIT_BADSWID:
			fprintf(stderr, "ccr: bad software id in program request\n");
			break;

		case EXIT_FOPENFAIL:
			fprintf(stderr, "ccr: file open failure\n");
			break;

		case EXIT_FREADFAIL:
			fprintf(stderr, "ccr: file read failure\n");
			break;

		case EXIT_FWRITEFAIL:
			fprintf(stderr, "ccr: file write failure\n");
			break;

		case EXIT_INVMSGSIZ:
			fprintf(stderr, "ccr: invalid mop message siz\n");
			break;

		case EXIT_INVPROGTYP:
			fprintf(stderr, "ccr: invalid program type in request message\n");
			break;

		case EXIT_INVRQST:
			fprintf(stderr, "ccr: invalid program request message\n");
			break;

		case EXIT_MCAST:
			fprintf(stderr, "ccr: multicast address was used on program request\n");
			break;

		case EXIT_NOAUTHORIZATION:
			fprintf(stderr, "ccr: operation unauthorized\n");
			break;

		case EXIT_NODIAGFILE:
			fprintf(stderr, "ccr: can't find diagnostic file\n");
			break;

		case EXIT_NODUMPFILE:
			fprintf(stderr, "ccr: can't find dump file\n");
			break;

		case EXIT_NOSYSFILE:
			fprintf(stderr, "ccr: can't find system load file\n");
			break;

		case EXIT_NOTCHOSEN:
			fprintf(stderr, "ccr: not chosen for downline load\n");
			break;

		case EXIT_NOTSU:
			fprintf(stderr, "ccr: must be super user\n");
			break;

		case EXIT_RCVFAIL:
			fprintf(stderr, "ccr: network receive failure\n");
			break;

		case EXIT_SENDFAIL:
			fprintf(stderr, "ccr: network transmit failure\n");
			break;

		case EXIT_SIGFAIL:
			fprintf(stderr, "ccr: can't create signal catcher\n");
			break;

		case EXIT_CANTEXEC:
			fprintf(stderr, "ccr: can't exec\n");
			break;

		case EXIT_WAITFAIL:
			fprintf(stderr, "ccr: can't wait\n");
			break;

		case EXIT_CANTDUP:
			fprintf(stderr, "ccr: can't duplicate network socket\n");
			break;

		case EXIT_BADARG:
			fprintf(stderr, "ccr: bad argument passed\n");
			break;

		case EXIT_PARSE_FAIL:
		case EXIT_GOOD:
		case EXIT_GOODSEC:
		case EXIT_GOODTER:
		case EXIT_GOODSYS:
			break;

		default:
			fprintf(stderr, "ccr: unrecognized error code = %d\n",err_code);
			break;
	}

	return;
}



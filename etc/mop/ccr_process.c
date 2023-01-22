#ifndef lint
static	char	*sccsid = "@(#)ccr_process.c	1.1	(ULTRIX)	9/30/85";
#endif lint

/*
 * MODULE CCR_PROCESS.C, handle ccr session once connection established
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
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include "ccr.h"
#include "ccr_mesg_codes.h"

union {
		u_char control_word;
		struct {
				unsigned mesg_number : 1;
				unsigned cmd_brk_flg : 1;
		} bitmask;
} control_flgs;

struct itimerval cp_tvalue, cp_ovalue;

u_char 	ccr_ccmd_poll[MAXL_RECV_BUF],
		ccr_cresp_ack[MAXL_RECV_BUF];

u_short	cresp_ack_len;
int		cur_msg_len,
		ccr_errno = -1;

char	done_processing = 0;

int send_ccpoll();
int stop_process();

extern int 	char_buf_len,
			sock;
extern char break_flag,
			done_read;
extern u_char	char_buffer[];
extern struct sys_id_info sysid_values;
extern int dli_xmtrec();
extern int read_unsol_chars();

/*
*	p r o c e s s _ c o m m a n d	
*
*	This routine handles normal processing after remote console
*	has been reserved.
*
* Inputs:
*		None.
*
* Outputs:
*		ccr_errno which contains SUCCESS if session completes 
*		without any problems otherwise FAIL.
*
*/

int process_command()
	  
{
int status;
	int readfd; 
	struct timeval tv;

	/*
	 * Set up to catch the SIGALRM for sending console command
	 * and poll messages.
	 */
	signal(SIGALRM,send_ccpoll);

	/*
	 * 	Initialize message number for console command and poll 
	 * 	message
	 */
	control_flgs.bitmask.mesg_number = 1;

	/*
	 * set up timer for read.
	 */
	bzero(&cp_tvalue,sizeof(struct itimerval));
	cp_tvalue.it_interval.tv_usec = CCPOLL_TIMEOUT;
	cp_tvalue.it_value.tv_usec = CCPOLL_TIMEOUT;
	setitimer(ITIMER_REAL, &cp_tvalue, &cp_ovalue);

	readfd = (1 << 0);
	bzero(&tv,sizeof(struct timeval));
	tv.tv_sec = 1 ; 
	tv.tv_usec = 0; 
	while (!done_processing)
	{
		if(select(2,&readfd,0,0,&tv) <= 0)
		{
			readfd = (1 << 0);
			tv.tv_sec = 1 ; 
			tv.tv_usec = 0; 
		}
		else
		{
			read_unsol_chars();
			readfd = (1 << 0);
			tv.tv_sec = 1 ; 
			tv.tv_usec = 0; 
		}
	}
	/* 
	 * Turn off timer and reset signal handling to default
	 */
	bzero(&cp_tvalue,sizeof(struct itimerval));
	setitimer(ITIMER_REAL, &cp_tvalue, &cp_ovalue); 
	signal(SIGALRM,SIG_DFL);

	/*
	 * Return global error number indicating success or failure
	 */	
	return(ccr_errno);
}

/*
*	s e n d _ c c p o l l
*
*	Is called when a SIGALRM is caught. The routine builds the MOP
*	Console Command and Poll message and appends user data if any
*	is present. This message is tranmitted to the remote console and
*	a Console Response and Acknowledgement message is expected and
*	processed.
*
* Inputs:
*	- sig: signal
*	- code: signal parameter
*	- scp: pointer to signal context
*
* Outputs:
*	None
*
* Note:
*	This routine is triggered off of a timer so that NULL Console Command
*	and Poll messages will be sent regularly. Otherwise, the remote console
*	will break the link.
*/
int send_ccpoll(sig,code,scp)
int sig, code;
struct sigcontext *scp;
{

	int status;

	if ((!done_read) || (ccr_errno == FAIL))
		return;
	/* 	
	 * Initialize message number and console command and poll
	 * message 
	 */

	control_flgs.bitmask.mesg_number = ~control_flgs.bitmask.mesg_number;
	if (break_flag)
	{
		control_flgs.bitmask.cmd_brk_flg = 1;
		break_flag = 0;
	}
	else
		control_flgs.bitmask.cmd_brk_flg = 0;

	ccr_ccmd_poll[MOP_LEN_OVRHD] = MOP_CCR_CCPOLL;
	ccr_ccmd_poll[CONTRL_FLG_IDX] = control_flgs.control_word;

	cur_msg_len = MOP_LEN_OVRHD;

	if (char_buf_len != 0)
	{
		bcopy(char_buffer,&ccr_ccmd_poll[CONTRL_FLG_IDX + 1],char_buf_len);
		cur_msg_len += char_buf_len;
		char_buf_len = 0;
	}
	ccr_ccmd_poll[0] = cur_msg_len;

	/*
	 * Initialize status to indicate that command not sent. The routine
	 * process_resp_mesg() will return a -1 to indicate need to retransmit
	 * the last message because it was lost. Otherwise it will return
	 * a SUCCESS or FAIL code.
	 */
	status = TRY_AGAIN;
	while (status == TRY_AGAIN)
	{
		if (status = dli_xmtrec(sock,ccr_ccmd_poll,cur_msg_len+2,ccr_cresp_ack))
		{
			/*
			 * Calculate actual length of message and process teh
			 * response message.
			 */
			cresp_ack_len = *(u_short *)ccr_cresp_ack;
			if((status = process_resp_mesg()))
				return;
		}
	}
	/* 
	 * If error in either, the network i/o or return message processing,
	 * set flag to indicate done processing.
	 * ccr_errno will contain a value indicating FAILure.
	 */
	ccr_errno = FAIL;
	done_processing = 1;
	/* 
	 * Turn off timer 
	 */
	bzero(&cp_tvalue,sizeof(struct itimerval));
	setitimer(ITIMER_REAL, &cp_tvalue, &cp_ovalue); 
	return;

}

/*
* 	p r o c e s s _ r e s p _ m e s g	
*
*	Validate the Console Response and Acknowledgement message and
*	outputs any data returned in this message.
*
* Inputs:
*	None
*
* Outputs:
*	None
*
*/
int process_resp_mesg()
{
	int i;

	/*
	 * Check to make sure valid response message
	 */
	if((cresp_ack_len > sysid_values.max_rsp_size) || 
	   (cresp_ack_len < MOP_LEN_OVRHD) ||
	   (ccr_cresp_ack[MOP_LEN_OVRHD] != MOP_CCR_CCRESP))
	{
		print_message(ccr_PROGERR);
		return(FAIL);
	}
	else
		if(ccr_cresp_ack[CONTRL_FLG_IDX] & FLG_COMND_LOST)
			return(TRY_AGAIN);
		else
		{
			if (cresp_ack_len > 2)
				for(i = 4; i < cresp_ack_len + 2; i++)
					putchar(ccr_cresp_ack[i]); 
			return(SUCCESS);
		}


}

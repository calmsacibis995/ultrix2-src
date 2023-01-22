#ifndef lint
static char *sccsid = "@(#)mop_dlload.c	1.5	ULTRIX	10/3/86";
#endif lint

/*
 * Program mop_dumpload.c,  Module MOP 
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
#include <signal.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#include <strings.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include "/sys/net/if.h"
#include "/sys/netinet/in.h"
#include "/sys/netinet/if_ether.h"
#include <netdnet/dli_var.h>
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#include <netdnet/node_params.h>
#include "nmgtdb.h"
#include "mopdb.h"
#include "mop_var.h"
#include "mop_proto.h"

#define STDIN	0
#define STDOUT	1

extern int exit_status, file_format, aout_text, aout_gap;
extern u_char label_type;
extern errno;

/*
*		m o p _ d o w n l i n e _ l o a d
*
*
* Description:
*	This subroutine downline loads a secondary, tertiary or system
*	image as a function of the program type found in the program
*	request message.  
*
* Inputs:
*	node			= structure containing DECnet node address
*				  and name of target node
*	host			= structure containing DECnet node address
*				  and name of host node
*	parm			= Pointer to structure where parsed elements
*				  of mop load request are to be stored.
*	task			= Pointer to structure containing task
*				  label parameters.
*	fp			= Pointer to file descriptor of load module
*	dest			= Ethernet physical address used to address
*				  host node.
*
*
* Outputs:
*	exit_status		= written into only if error.
*
* Notes:
*
*/
mop_downline_load( node, host, parm, task, fp, dest )
register struct node_id *node, *host;
register struct mop_parse_dmpld *parm;
register struct task_image *task;
register FILE *fp;
register u_char *dest;
{
	int error = NULL;
	u_short multicast;
	void catch_sigalrm();

	signal(SIGALRM, catch_sigalrm);

	sscanf(dest, "%x", &multicast);
	switch ( parm->mop_pgm_type )
	{
		case MOP_SECONDARY:
			mop_syslog("mop_dumpload", "sending secondary loader", 0, LOG_INFO);
			if(file_format != FF_AOUTNEW && file_format != FF_AOUTOLD)
				{
				load_secondary( parm, task, fp );
				break;
				}
		case MOP_TERTIARY:
			if (multicast & 1)
			{
				mop_syslog("mop_dumpload", "sending volunteer assistance for tertiary load", 0, LOG_INFO);
				error = tx_volunteer_assist(parm);
			}
			if ( ! error )
			{
				mop_syslog("mop_dumpload", "sending tertiary loader", 0, LOG_INFO);
				if ( label_type == LABEL_TYPE_RSX )
					load_ter_or_sys( node, host, parm, task, fp );
				else
					load_ccr_sys(parm, fp);
			}
			break;

		case MOP_SYSTEM:
			if (multicast & 1)
			{
				mop_syslog("mop_dumpload", "sending volunteer assistance for system load", 0, LOG_INFO);
				error = tx_volunteer_assist(parm);
			}
			if (  ! error )
			{
				mop_syslog("mop_dumpload", "sending system image", 0, LOG_INFO);
				if ( label_type == LABEL_TYPE_RSX )
					load_ter_or_sys( node, host, parm, task, fp );
				else
					load_ccr_sys(parm, fp);
			}
			break;

		default:
			mop_syslog("mop_dumpload", "unrecognized MOP program type", 0, LOG_INFO);
			exit_status = EXIT_INVPROGTYP;
			break;
	}
	close(fp);
	return(NULL);
}

/*
*		t x _ v o l u n t e e r _ a s s i s t
*
* Version 1.0 - 1/10/85
*
*
* Description:
*	This subroutine transmit a volunteer assistance MOP message.
*
* Inputs:
*	old_parm		= Pointer to structure containing parsed 
*				  elements of mop load request. 
*
*
* Outputs:
*	returns			0 if success, -1 if failure.
*	exit_status		= written into only if error.
*
* Notes:
*
*/
tx_volunteer_assist( old_parm )
	struct mop_parse_dmpld old_parm;
{
	struct mop_parse_dmpld new_parm;
	int rsiz;
	u_char in_msg[MOM_MSG_SIZE];
	u_char msg[3];

	msg[0] = 1;
	msg[1] = 0;
	msg[2] = 3;

	if ( write(STDOUT, msg, sizeof(msg)) <= 0 )
	{
		mop_syslog("mop_dumpload", "can't send volunteer assistance", 0, LOG_INFO);
		exit_status = EXIT_SENDFAIL;
		return(-1);
	}
	if ((rsiz = read_mop(MOP_RTIMOUT, in_msg, sizeof(in_msg))) <= 0 )
	{
		mop_syslog("mop_dumpload", "no program request received after volunteer assistance sent", 0, LOG_DEBUG);
		exit_status = EXIT_NOTCHOSEN;
		return(-1);
	}
	bzero(&new_parm, sizeof(struct mop_parse_dmpld));
	rqst_parse(&new_parm, in_msg);
	if ( bcmp(&new_parm, &old_parm, sizeof(struct mop_parse_dmpld)) == NULL )
	{
		mop_syslog("mop_dumpload", "program requests do not match", 0, LOG_INFO);
		exit_status = EXIT_INVRQST;
		return(-1);
	}
	return(NULL); 

}

/*
*		l o a d _ s e c o n d a r y
*
* Version: 1.0 - 1/10/85
*
*
* Description:
*	This is the secondary loader.
*
* Inputs:
*	parm			= Pointer to structure where parsed elements
*				  of mop load request are to be stored.
*	task			= Pointer to structure containing task
*				  label parameters.
*	fp			= Pointer to file descriptor of load module
*
*
* Outputs:
*	returns			= 0 if success, -1 if failure.
*	exit_status		= status of load completion.
*
* Notes:
*
*/
load_secondary( parm, task, fp )
register struct mop_parse_dmpld *parm;
register struct task_image *task;
FILE *fp;
{
	u_char buf[MOM_MSG_SIZE];
	u_short siz;
	u_long tsiz = task->task_size;
	if((file_format!=FF_AOUTNEW) && (file_format != FF_AOUTOLD))
		tsiz = task->task_size * TSK_IBLKSIZE;

	if ( tsiz > MAX_SECONDARY_SIZE )
	{
		mop_syslog("mop_dumpload", "secondary load file too large", 0, LOG_INFO);
		exit_status = EXIT_FREADFAIL;
		return(-1);
	}

	close(STDIN);

	bzero( buf, sizeof(buf));
	buf[MOP_CODE_IDX] = MOP_MLOAD_XFER;
	buf[MOP_MLDNUM_IDX] = NULL;
	*(u_long *) &buf[MOP_MLDADR_IDX] = task->base_addr;
	if (( siz = read(fileno(fp), buf+MOP_MLDIMG_IDX, tsiz) ) != tsiz)
	{
		mop_syslog("mop_dumpload", "unexpected secondary load file EOF", 0, LOG_INFO);
		exit_status = EXIT_FREADFAIL;
		return(-1);
	}

	siz += MOP_MLDIMG_IDX;
	*(u_long *) &buf[siz] = task->task_xferaddr;
	siz += 2;
	*(u_short *) buf = siz;
	if ( write(STDOUT, buf, (siz+2)) < 0 )
	{
		mop_syslog("mop_dumpload", "secondary load failed; can't transmit", 0, LOG_INFO);
		exit_status = EXIT_SENDFAIL;
		return(-1);
	}
	close(STDOUT);
	exit_status = EXIT_GOODSEC;
	return(0);

}

/*
*		l o a d _ t e r _ o r _ s y s
*
* Version: 1.0 - 1/10/85
*
*
* Description:
*	This subroutine loads either the tertiary or system load module.
*
* Inputs:
*	node			= structure containing DECnet node address
*				  and name of target node
*	host			= structure containing DECnet node address
*				  and name of host node
*	parm			= Pointer to structure where parsed elements
*				  of mop load request are to be stored.
*	task			= Pointer to structure containing task
*				  label parameters.
*	fp			= Pointer to file descriptor of load module
*
*
* Outputs:
*	returns			= 0 if success, -1 if failure.
*	exit_status		= contains final status of load.
*
* Notes:
*
*/
load_ter_or_sys( node, host, parm, task, fp )
struct node_id *node, *host;
struct mop_parse_dmpld *parm;
struct task_image *task;
register FILE *fp;
{
	int i, j;
	short siz;
	u_short host_addr;
	u_char host_name[16];
	u_char msg[MOM_MSG_SIZE];
	register u_char load_number = 0377;
	register u_char done = 0, flag=0;
	u_long load_addr = 0;
	register u_long bytes_xferd = 0;
	int null = 0;
	u_long tmpsiz = 0;
	u_long tmp = 0;
	u_long max_bytes = task->task_size; 
	register u_long load_size = parm->mop_dl_bufsiz - MOP_MLDIMG_IDX;
	u_long load_xfer = task->task_xferaddr;
	char	errbuf[256];

	bcopy(&task->base_addr, &load_addr, sizeof(task->base_addr));

	if((file_format!=FF_AOUTNEW) && (file_format != FF_AOUTOLD))
		max_bytes = task->task_size * TSK_IBLKSIZE; 

	/*
	 * Start downline load
	 */
	while ( ! done )
	{
		if ( (bytes_xferd + load_size) >= max_bytes )
		{
			load_size = max_bytes - bytes_xferd;
			done = 1;
		}
		msg[MOP_CODE_IDX] = MOP_MLOAD;
		msg[MOP_MLDNUM_IDX] = ++load_number;
		bcopy(&load_addr, &msg[MOP_MLDADR_IDX], sizeof(load_addr));
		if(file_format==FF_AOUTNEW && aout_gap && ((bytes_xferd + load_size) > aout_text) && !flag) /* skip text and data gap*/
		{
			mop_syslog("mop_dumpload", "processing the AOUT file gap", 0, LOG_DEBUG);
			sprintf(errbuf,"bytes_xferd=%d max_bytes=%d aout_text=%d load_size=%d\n",bytes_xferd,max_bytes,aout_text,load_size);
			mop_syslog("mop_dlload",errbuf,0,LOG_DEBUG);
			flag=1;
			siz=0;
			if(bytes_xferd < aout_text)
			{
				tmpsiz = aout_text - bytes_xferd;
				if((siz = read(fileno(fp), msg+MOP_MLDIMG_IDX, tmpsiz)) <= 0 )
				{
					mop_syslog("mop_dumpload", "load file read error", 1, LOG_INFO);
					exit_status = EXIT_FREADFAIL;
					return(-1);
				}
			}
			tmpsiz= fseek(fp, -aout_gap,1);
			if ((tmp = read(fileno(fp), msg+MOP_MLDIMG_IDX+siz, load_size - siz)) <= 0 )
			{
				mop_syslog("mop_dumpload", "load file read error", 1, LOG_INFO);
				exit_status = EXIT_FREADFAIL;
				return(-1);
			}
			siz+= tmp;
		}
		else if ( (siz = read(fileno(fp), msg+MOP_MLDIMG_IDX, load_size)) <= 0 )
		{
			if ( siz < 0 )
			{
				mop_syslog("mop_dumpload", "load file read error", 1, LOG_INFO);
			}
			else
			{
				mop_syslog("mop_dumpload", "load file read error, unexpected end of file", 0, LOG_INFO);
			}
			exit_status = EXIT_FREADFAIL;
			return(-1);
		}
		bytes_xferd += siz;
		load_addr += siz;
		siz += MOP_MLDIMG_IDX - 2;
		bcopy(&siz, msg, sizeof(siz));


		if ( send_mopdw_msg( msg, siz+2, MOP_TX_POSTRX ) < 0 )
		{
			return(-1);
		}
	}

	/*
	 * finish downline load
	 */
	if ( parm->mop_pgm_type == MOP_TERTIARY && file_format != FF_AOUTOLD)
	{
		msg[MOP_CODE_IDX] = MOP_MLOAD_XFER;
		msg[MOP_MLDNUM_IDX] = ++load_number;
		bcopy(&null, &msg[MOP_MLDADR_IDX], sizeof(null));
		bcopy(&load_xfer, &msg[8], sizeof(load_xfer));
		siz = 10;
		bcopy(&siz, msg, sizeof(siz));
		exit_status = EXIT_GOODTER;
		return( send_mopdw_msg( msg, siz+2, MOP_TX_NOPOSTRX ) );
	}
	else
	{
		msg[MOP_CODE_IDX] = MOP_PLOAD_XFER;
		msg[MOP_MLDNUM_IDX] = ++load_number;
		siz = MOP_MLDNUM_IDX + 1;

		if ( node->node_name[0] != NULL )
		{
			msg[siz++] = MOP_PLOAD_TSNAME;
			if ( (i = toupcase(node->node_name)) > 16 )
			{
				i = 16;
			}
			msg[siz++] = i;
			bcopy(node->node_name, msg+siz, i);
			siz += i;
		}
		msg[siz++] = MOP_PLOAD_TSADDR;
		msg[siz++] = sizeof(node->node_dnaddr);
		bcopy(&node->node_dnaddr, &msg[siz], sizeof(node->node_dnaddr));
		siz += sizeof(node->node_dnaddr);

		if ( i = gethostnode( host, host_name, &host_addr ) )
		{
			msg[siz++] = MOP_PLOAD_HSNAME;
			msg[siz++] = i;
			bcopy(host_name, msg+siz, i);
			siz += i;
			msg[siz++] = MOP_PLOAD_HSADDR;
			msg[siz++] = sizeof(host_addr);
			bcopy(&host_addr, &msg[siz], sizeof(host_addr));
			siz += sizeof(host_addr);
		}

		msg[siz++] = MOP_PLOAD_HSTIME;
		msg[siz++] = MOP_PLOAD_HSTIMSIZ;
		if ( gethstime(&msg[siz], (MOM_MSG_SIZE - siz)) < 0)
		{
			siz -= 2;
		}
		else
		{
			siz += MOP_PLOAD_HSTIMSIZ;
		}
		msg[siz++] = NULL;
		bcopy(&load_xfer, msg+siz, sizeof(load_xfer));
		siz += sizeof(load_xfer) - 2;
		bcopy(&siz, msg, sizeof(siz));
		exit_status = EXIT_GOODSYS;
		return( send_mopdw_msg( msg, siz+2, MOP_TX_POSTRX ) );
	}

}

/*
*		s e n d _ m o p d w _ m s g
*
* Version: 1.0 - 1/10/85
*
*
* Description:
* 	This subroutine forwards a downline load message to requester.
*
* Inputs:
*	out_msg			= Pointer to message being transmitted.
*	tsiz			= size of message to be transmitted.
*	post_rx			= flag indicating whether or not a receive
*				  should be posted after transmission.
*
*
* Outputs:
*	returns			0 if success, -1 if failure
*	exit_status		= written into only if error.
*
* Notes:
*
*/
send_mopdw_msg( out_msg, tsiz, post_rx )
register u_char *out_msg;
register int tsiz, post_rx;
{
	static u_char ack_received = 0;
	u_char in_msg[MOM_MSG_SIZE];
	register int rsiz;
	u_char perform_write = 1;
	int retry = MOP_RETRY;
	while ( retry > 0 )
	{
		if ( perform_write && write(STDOUT, out_msg, tsiz) < 0 )
		{
			mop_syslog("mop_dumpload", "load failed; can't transmit ", 1, LOG_INFO);
			exit_status = EXIT_SENDFAIL;
			return(-1);
		}
		else if ( ! post_rx )
		{
			return(NULL);
		}
		else if ((perform_write = 1) && (rsiz = read_mop(MOP_RTIMOUT, in_msg, sizeof(in_msg))) <= 0 )
		{
			if ( errno == ETIMEDOUT )
			{
				mop_syslog("mop_dumpload", "load timeout, retry attempted", 0, LOG_INFO);
				retry--;
			}
			else
			{
				if ( rsiz == 0 )
					mop_syslog("mop_dumpload", "load failed, read error", 0, LOG_INFO);
				else
				{
					mop_syslog("mop_dumpload", "load failed, read error", 1, LOG_INFO);
				}
				exit_status = EXIT_RCVFAIL;
				return(-1);
			}
		}
		else if ( in_msg[MOP_CODE_IDX] != MOP_RQST_MEM )
		{
			if ( in_msg[MOP_CODE_IDX] == MOP_RQST_PGM && ! ack_received )
			{
				perform_write = 0;
				continue;
			}
			mop_syslog("mop_dumpload", "load failed at destination - invalid mop code received", 0, LOG_INFO);
			exit_status = EXIT_RCVFAIL;
			return(-1);
		}
		else if ( in_msg[MOP_MLDERR_IDX] != 0 )
		{
			if ( in_msg[MOP_MLDERR_IDX] == 1 && in_msg[MOP_MLDNUM_IDX] == ((out_msg[MOP_MLDNUM_IDX] + 1) & 0xff) )
			{
				ack_received = 1;
				return(NULL);
			}
			if ( in_msg[MOP_MLDERR_IDX] == 1 && in_msg[MOP_MLDNUM_IDX] == out_msg[MOP_MLDNUM_IDX] )
			{
				retry--;
				continue;
			}
			mop_syslog("mop_dumpload", "load failed at destination - error reported", 0, LOG_INFO);
			exit_status = EXIT_RCVFAIL;
			return(-1);
		}
		else if ( in_msg[MOP_MLDNUM_IDX] == ((out_msg[MOP_MLDNUM_IDX] + 1) & 0xff) )
		{
			ack_received = 1;
			return(NULL);
		}
		else
		{
			--retry; 
		}
	}
	mop_syslog("mop_dumpload", "load failed", 0, LOG_INFO);
	exit_status = EXIT_RCVFAIL;
	return(-1);
}

/*
*		l o a d _ c c r _ s y s
*
* Version: 1.0 - 8/14/85
*
*
* Description:
*	This subroutine loads the ccr system image, which is in an
*	Absolute Loader Record format with a header for each record as follows:
*
*			Byte			Contents
*			----			--------
*			0,1				always a binary word of 1
*			2,3				number of bytes in record (can be odd), from 
*								byte 0
*			4,5				PDP-11 load address
*			6-last byte		data to be loaded starting at load address
*			last byte + 1	checksum (XOR ?), can be ignored for our
*								purposes.
*
*
* Inputs:
*	mop_parameters		= Pointer to structure where parsed elements
*				  			of mop load request are to be stored.
*	fp			= Pointer to file descriptor of load module
*
*
* Outputs:
*	returns			= 0 if success, -1 if failure.
*
* Notes:
*
*/
load_ccr_sys( parm, fp )
struct mop_parse_dmpld *parm;
register FILE *fp;

{
	int i = 0;
	struct header_fmt header, *ptr_header;
	short siz;
	u_char temp_buffer[MOM_MSG_SIZE];
	u_char msg[MOM_MSG_SIZE];
	register u_char load_number = 0377;
	int load_addr = 0;
	int null = 0;
	int data_length = 0;
	register int load_size = parm->mop_dl_bufsiz - MOP_MLDIMG_IDX;
	int load_xfer;



	ptr_header = &header;
	/*
	 * Start downline load
	 */
	while (1)
	{

		/* Start building Memory Load message */

		msg[MOP_CODE_IDX] = MOP_MLOAD;
		msg[MOP_MLDNUM_IDX] = ++load_number;


		/* Read the header of the record */

		if ( (siz = read(fileno(fp),ptr_header,sizeof(*ptr_header))) <= 0 )
		{
			if ( siz < 0 )
			{
				mop_syslog("mop_dumpload", "load file read error", 1, LOG_INFO);
			}
			else
			{
				mop_syslog("mop_dumpload", "load file read error, unexpected end of file", 0, LOG_INFO);
			}
			exit_status = EXIT_FREADFAIL;
			return(-1);
		}


		/* If the length of the record is 6, it is the last record in
		 * the file, by definition. So exit the while loop and
		 * continue with the final stage of the load */

		if (ptr_header->record_length == 6)
		{
			load_xfer = ptr_header->load_address;
			break;
		}

		/* Otherwise, copy the load address into the Memory Load message */

		bcopy(&ptr_header->load_address,&msg[MOP_MLDADR_IDX],sizeof(u_short));

		/* Calculate the actual data length, have to subtract the
		 * overhead of the first 2 bytes in the record. */

		data_length = ptr_header->record_length - sizeof(header);

		/* And until the complete record has been sent, send the maximum
		 * number of bytes possible ina single transfer. Loop until a
		 * single record has been sent */


		while( data_length > 0)
		{
			if (data_length <= load_size)
			{

				/* Always read one additional byte at the end of a record,
				 * because this contains a checksum, which can be
				 * ignored. */

				if ( (siz = read(fileno(fp),temp_buffer,data_length + 1 )) <= 0 )
				{
					if ( siz < 0 )
					{
						mop_syslog("mop_dumpload", "load file read error", 1, LOG_INFO);
					}
					else
					{
						mop_syslog("mop_dumpload", "load file read error, unexpected end of file", 0, LOG_INFO);
					}
					exit_status = EXIT_FREADFAIL;
					return(-1);
				}
				bcopy(temp_buffer,msg+MOP_MLDIMG_IDX,data_length);
				*(u_short *)msg = --siz + MOP_MLDIMG_IDX - sizeof(siz);
				data_length = 0;
			}
			else
			{
				if ( (siz = read(fileno(fp), msg+MOP_MLDIMG_IDX, load_size)) <= 0 )
				{
					if ( siz < 0 )
					{
						mop_syslog("mop_dumpload", "load file read error", 1, LOG_INFO);
					}
					else
					{
						mop_syslog("mop_dumpload", "load file read error, unexpected end of file", 0, LOG_INFO);
					}
					exit_status = EXIT_FREADFAIL;
					return(-1);
				}
				data_length -= load_size;
				*(u_short *)msg = siz + MOP_MLDIMG_IDX - sizeof(siz);
			}


			if ( send_mopdw_msg( msg, *(u_short *)msg + 2, MOP_TX_POSTRX ) < 0 )
			{
				return(-1);
			}
		}
	}

	/*
	 * Finish downline load, by building and sending a Parameter Load
	 * with Transfer Address.
	 */
		

		msg[MOP_CODE_IDX] = MOP_PLOAD_XFER;
		msg[MOP_MLDNUM_IDX] = load_number;
		msg[MOP_MLDNUM_IDX + 1] = NULL;
		bcopy(&load_xfer, msg+(MOP_MLDNUM_IDX + 2), sizeof(load_xfer));
		siz = 7;
		bcopy(&siz, msg, sizeof(siz));
		exit_status = EXIT_GOODSYS;
		return( send_mopdw_msg( msg, siz+2, MOP_TX_NOPOSTRX ) );


}

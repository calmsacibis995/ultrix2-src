#ifndef lint
static char *sccsid = "@(#)mop_parsrqst.c	1.4	ULTRIX	10/3/86";
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

extern int exit_status;

/*
*		r q s t _  p a r s e
*
* Version 2.0 - 7/18/85
*
*
* Description:
*	This subroutine parses the mop load/dump request message, and
*	places the information in the mop_parameters structure.
*
* Inputs:
*	mop_parameters		= Pointer to structure where parsed elements
*				  of mop load request are to be stored.
*	message			= the actual mop message itself.
*
*
* Outputs:
*	mop_parameters		= structure where parsed elements
*				  of mop load request are stored.
*	exit_status		= written into only if error.
*
* Notes:
*
*/
rqst_parse( mop_parameters, message )
register struct mop_parse_dmpld *mop_parameters;
register u_char *message;
{
	u_short msg_size;
	register int i;

	/*
	 * parse common part of dump/load request message
	 */
	bcopy(message, &msg_size, sizeof(msg_size));
	if ( msg_size > MOM_MSG_SIZE )
	{
		mop_syslog("mop_dumpload", "invalid request message size", 0, LOG_INFO);
		exit_status = EXIT_INVRQST;
		return(-1);
	}
	i = sizeof(msg_size);
	mop_parameters->mop_dmpld_code = message[i++];
	mop_parameters->mop_devtyp = message[i++];
	if ( (mop_parameters->mop_format_vsn = message[i++]) != 1 )
	{
		mop_syslog("mop_dumpload", "invalid format version in request message", 0, LOG_INFO);
		exit_status = EXIT_INVRQST;
		return(-1);
	}
	
	/*
	 * parse uncommon parts of request message
	 */
	switch ( mop_parameters->mop_dmpld_code )
	{
		case MOP_RQST_PGM:
			if ( i == msg_size )
			{
				mop_parameters->mop_pgm_type = NULL;
			}
			else
			{
				rqst_load_parse(mop_parameters, message, i, msg_size);
			}
			break;

		case MOP_RQST_DMPSRV:
			rqst_dump_parse(mop_parameters, message, i, msg_size);
			if ( mop_parameters->mop_compat_bits != 2 )
			{
				mop_syslog("mop_dumpload", "invalid dump request message format", 0, LOG_INFO);
				exit_status = EXIT_INVRQST;
				return(-1);
			}
			break;

		default:
			mop_syslog("mop_dumpload", "invalid MOP message code", 0, LOG_INFO);
			exit_status = EXIT_INVRQST;
			return(-1);
			break;
	}
	return(0);
}

/*
*		r q s t _  l o a d _ p a r s e
*
* Version 2.0 - 7/18/85
*
*
* Description:
*	This subroutine finishes parsing the mop load request message.
*
* Inputs:
*	mop_parameters		= Pointer to structure where parsed elements
*				  of mop load request are to be stored.
*	message			= the mop dump/load message.
*	i			= index into load parameters
*	msg_size		= size of mop message
*
*
* Outputs:
*	mop_parameters		= Pointer to structure where parsed elements
*				  of mop load request are stored.
*
* Notes:
*
*/
rqst_load_parse( mop_parameters, message, i, msg_size)
register struct mop_parse_dmpld *mop_parameters;
register u_char *message;
register int i;
register int msg_size;
{
	int j;
	int bad_swid = 0;

	mop_parameters->mop_pgm_type = message[i++];
	if ((mop_parameters->mop_swid_form = message[i++]) > 0)
	{
		j = -1;
		while( (++j < mop_parameters->mop_swid_form) && j < sizeof(mop_parameters->mop_swid_id) )
		{
			if( (mop_parameters->mop_swid_id[j] = message[i++]) < '!' || mop_parameters->mop_swid_id[j] > '}' )
				bad_swid = 1;
		}
	}
	if ( bad_swid )
	{
		mop_syslog("mop_dumpload", "request message contains invalid software id - using data base", 0, LOG_INFO);
		mop_parameters->mop_swid_form = MOP_SWID_SOS;
	}
	mop_parameters->mop_processor = message[i++];
	parse_info_field(mop_parameters, message, i, msg_size);
	return;
}

/*
*		r q s t _  d u m p _ p a r s e
*
* Version 2.0 - 7/18/85
*
*
* Description:
*	This subroutine finishes parsing the mop dump service request message.
*
* Inputs:
*	mop_parameters		= Pointer to structure where parsed elements
*				  of mop load request are to be stored.
*	message			= the mop dump/load message.
*	i			= index into load parameters
*	msg_size		= size of mop message
*
*
* Outputs:
*	mop_parameters		= Pointer to structure where parsed elements
*				  of mop load request are stored.
*
* Notes:
*
*/
rqst_dump_parse( mop_parameters, message, i, msg_size)
register struct mop_parse_dmpld *mop_parameters;
register u_char *message;
register int i;
register int msg_size;
{

	mop_parameters->mop_dump_size = *(int *) &message[i];
	i += 4;
	mop_parameters->mop_compat_bits = message[i++];
	parse_info_field(mop_parameters, message, i, msg_size);

	/*
	 * Account for data link buffer size in Pluto dumper.
	 */
	if ( mop_parameters->mop_dl_bufsiz >= 263 )
	{
		mop_parameters->mop_dl_bufsiz = 263;
	}
	else
	{
		mop_parameters->mop_dl_bufsiz = 135;
	}

	return;
}

/*
*		p a r s e _ i n f o _ f i e l d
*
* Version 2.0 - 7/18/85
*
*
* Description:
*	This subroutine parses the info field in a mop message.
*
* Inputs:
*	mop_parameters		= Pointer to structure where parsed elements
*				  of mop load request are to be stored.
*	message			= the mop dump/load message.
*	i			= index into load parameters
*	msg_size		= size of mop message
*
*
* Outputs:
*
* Notes:
*
*/
parse_info_field( mop_parameters, message, i, msg_size)
register struct mop_parse_dmpld *mop_parameters;
register u_char *message;
register int i;
register int msg_size;
{
	u_short info_type;
	u_short j;

	/*
	 * Set data link buffer size to default - 20 for Ethernet overhead
	 * and padded byte count.
	 */
	mop_parameters->mop_dl_bufsiz = MOP_PLOAD_DLBSDEF - 20;

	bcopy(&message[i], &info_type, sizeof(info_type));
	i += 2;
	while ( i < msg_size )
	{
		j = message[i++];
		switch (info_type)
		{
			case SYSID_SYSPROC:
				mop_parameters->mop_sys_processor = message[i++];
				break;

			case SYSID_HWADDR:
				bcopy(&message[i], mop_parameters->mop_hw_addr, sizeof(mop_parameters->mop_hw_addr));
				i += sizeof(mop_parameters->mop_hw_addr);
				break;

			case SYSID_DLBUFSIZ:
				bcopy(&message[i], &mop_parameters->mop_dl_bufsiz, sizeof(mop_parameters->mop_dl_bufsiz));
				if ( (mop_parameters->mop_dl_bufsiz -= 20) > MOP_MAXLD_SIZ )
				{
					mop_parameters->mop_dl_bufsiz = MOP_MAXLD_SIZ;
				}
				i += sizeof(mop_parameters->mop_dl_bufsiz);
				break;
			default:
				i += j;
				break;
		}
		bcopy(&message[i], &info_type, sizeof(info_type));
		i += 2;
	}
	return;
}

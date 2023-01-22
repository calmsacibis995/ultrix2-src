#ifndef	lint
static char *sccsid = "@(#)dli_open.c	1.8	ULTRIX	1/9/87";
#endif	lint

/*
 * Program dli_opent.c,  Module DLI 
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

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/protosw.h"
#include "../h/errno.h"

#include "../net/if.h"


#include "../netinet/in.h"
#include "../netinet/if_ether.h"

#include "../netdnet/dli_var.h"

extern struct dli_line dli_ltable[];



/*
 *		d l i _ o p e n
 *
 * Process a DLI open line request.
 *
 * Returns:		error code if error, otherwise NULL.
 *
 * Inputs:
 *	so		= Pointer to the socket for this request.
 */
dli_open( so )
register struct socket *so;
{
	
	register i = 0;

	while ( dli_ltable[i].dli_so != NULL && i < DLI_MAXLINE ) i++;
	if ( i == DLI_MAXLINE )
	{
		return(ENOBUFS);
	}
	dli_ltable[i].dli_so = so;
	so->so_pcb = ( caddr_t ) &dli_ltable[i];
	return(NULL);

}

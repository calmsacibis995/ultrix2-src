#ifndef	lint
static char *sccsid = "@(#)dli_usrreq.c	1.9	ULTRIX	1/9/87";
#endif	lint

/*
 * Program dli_usrreq.c,  Module DLI 
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
 * DLI protocol interface to socket abstraction.
 */

/*
 *		d l i _ u s r r e q
 *
 * Process a DLI request.
 *
 * Returns:		Nothing
 *
 * Inputs:
 *	so		= Pointer to the socket for this request.
 *	req		= Request function code.
 *	m		= Pointer to an MBUF chain.
 *	nam		= Pointer to an MBUF chain for addressing.
 *	rights		= Pointer to an MBUF chain for access rights.
 */
dli_usrreq( so,req,m,nam,rights )
register struct socket *so;
int req;
struct mbuf *m,*nam,*rights;
{
    int s = splnet();
    register struct sockaddr_dl *to_whom;
    register int i = 0;
    int error = 0;
    struct ifnet *ifp;



    /*
     * Service request.
     */
    switch (req)
    {
	/*
	 * DLI attaches to a socket via PRU_ATTACH, reserving space.
	 */
		case PRU_ATTACH:
			if( ! suser() )
			{
				return(EPERM);
			}
			error = soreserve(so, DLI_SENDSPACE, DLI_RECVSPACE);
			if (!error)
			{
				error = dli_open(so);
			}
			if (!error)
			{
				soisconnected(so);
			}
			break;

	/*
	 * PRU_DETACH detaches the DLI protocol from the socket.
	 */
		case PRU_DETACH:
			dli_close((struct dli_line *) so->so_pcb);
			so->so_pcb = NULL;
	   		sbflush(&so->so_rcv);
	    		sbflush(&so->so_snd);
	    		soisdisconnected(so);
			break;


	/*
	 * Bind name to socket
	 */
		case PRU_BIND:
			if (nam->m_len != sizeof(struct sockaddr_dl))
			{
				error = EINVAL;
				break;
			}
			to_whom = mtod(nam, struct sockaddr_dl *);
			error = dli_bind((struct dli_line *) so->so_pcb, to_whom);
			break;
	

	/*
	 * Data is available for transmission.
	 */
		case PRU_SEND:
			if ( mbuf_len(m) > DLI_MAXPKT )
			{
				m_freem(m);
				error = EMSGSIZE;
				break;
			}
			to_whom = NULL;
			if ( nam != NULL)
			{
				if (nam->m_len != sizeof(struct sockaddr_dl))
				{
					m_freem(m);
					error = EINVAL;
					break;
				}
				to_whom = mtod(nam, struct sockaddr_dl *);
			}
			error = dli_output((struct dli_line *) so->so_pcb, m, to_whom);
			break;
	
	
	/*
	 * Return value of user's bound socket address.
	 */
		case PRU_SOCKADDR:
				error = dli_fetchbind((struct dli_line *) so->so_pcb, mtod(nam, struct sockaddr_dl *), &nam->m_len);
			break;
	

	/*
	 * A getsockopt() was issued.
	 */
		case PRU_GETSOCKOPT:
			if (m)
			{
				error = dli_getopt((struct dli_line *) so->so_pcb, mtod(m, u_char *), &m->m_len, (int)nam);
			}
			else
			{
				error = EFAULT;
			}
			break;
	



	/*
	 * A setsockopt() was issued.
	 */
		case PRU_SETSOCKOPT:
			if (m)
			{
				error = dli_setopt((struct dli_line *) so->so_pcb, mtod(m, u_char *), m->m_len, (int)nam);
			}
			else
			{
				error = EFAULT;
			}
			if (m) m_free(m);
			break;
	
	/*
	 * An ioctl() was issued
	 */
		case PRU_CONTROL:
			if ( ( ifp = ((struct dli_line *) so->so_pcb)->dli_if ) &&  suser() )
			{
				return( (*ifp->if_ioctl)(ifp, (int)m, (caddr_t)nam) );
			}
			else
			{
				return( EACCES );
			}
			break;

		default:
		    error = EOPNOTSUPP;
		    break;
	
    }
    splx(s);
    return (error);
}

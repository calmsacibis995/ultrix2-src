#ifndef lint
static	char	*sccsid = "@(#)ildr.c	1.1	(ULTRIX)	1/8/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
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

/*	
 *	ilwrite() : write driver
 *		1. copy Lock request info to lockbuf
 *		2. follow action in l_act
 *		3. Error return conditions
 *			-1: lockrequest fails(only on act=1)
 *			-2: attempt to release a lock not set
 *			    by calling program
 *			-3: illegal action requested
 */

# include	<stdio.h>
# include	<sys/param.h>
# include	<sys/socket.h>
# include	<netinet/in.h>
# include	<sys/ioctl.h>
# include	<errno.h>
# include	<ildr.h>
# include	<sys/time.h>
# include	<netdb.h>

# define	TRUE	1
# define	FALSE	0

# ifdef	DEBUG
static int ildebug = TRUE;
# endif	DEBUG

char	Sccsid[] = "@(#)ildr.c	1.1 (Ingres) 1/8/85";

int	From_server;		/* read connection from socket server */
long	Read_fmt;		/* bit mask for select */
char	*Prog_name;		/* program name for abnormal errors */
extern	int	errno;		/* error number */

/*
** main
** initilize the socket to the socket server, and then sit and on
** a select waiting for input.
*/
main(ac,av)
int	ac;
char	**av;
{
	long		read_fd,		/* bit mask of useable descriptors*/
			num_des;		/* number of readable descriptors */
	int		abnormal();		/* error function */
	register	int	i;		/* index */


	/*
	** close all the files descriptors, so we can have more INGRES
	** processes. We are lmited by file descriptors.
	*/
# ifdef	DEBUG
	setbuf(stdout,NULL);
	printf("DRIVER: starting up\n");
	close(0);				/* guarantee that 0 is what is attached to the socket server */
	for ( i = 3 ; i < NOFILE ; i++ )
# else
	for ( i = 0 ; i < NOFILE ; i++ )
# endif	DEBUG
		close(i);

	/*
	** set up all the signals. Catching any signal
	** is an error. We do a "warm start" in this condition
	*/
	for ( i = 0 ; i < NSIG ; i++ )
		signal(i,abnormal);
	signal(SIGPIPE,SIG_IGN);	/* ignore this one, in case a process simply dies in mid stride */
	Prog_name = *av;
	/*
	** if ac == 2, then we are restarted from the the lock driver
	** itself, and we don't have to reattach ourselves to the magic
	** ingres socket.
	*/
	if ( ac == 2 )
		From_server = 0;
	else
		From_server = init_socket();
	Read_fmt = (1<<From_server);

	/*
	** infinite loop waiting for something to happen
	*/
	for ( ;; )
	{
		read_fd = Read_fmt;

		/*
		** wake up whenever something happens
		*/
		while ( (num_des = select(NOFILE,&read_fd,0,0,0)) == 0 )
		{
# ifdef	DEBUG
		printf("select returns 0 (%o)\n",read_fd);
# endif	DEBUG
			read_fd = Read_fmt;
		}
# ifdef	DEBUG
		printf("select returns %d (%o)\n",num_des,read_fd);
# endif	DEBUG
		if ( num_des == -1 )
		{
# ifdef	DEBUG
			perror("DRIVER:num_des = -1");
# endif DEBUG
			/*
			** a bit of defensive programming.
			** If there is an EBADF (bad file descriptor) error 
			** then we assume that a file descriptor has shut down,
			** with out tellng us. We go to a function to figure
			** out what has died.
			*/
			if ( errno == EBADF )
				close_up(Read_fmt);
			sleep(1);
			continue;
		}
		if ( (read_fd & (1<<From_server)) )
		{
			num_des--;
			new_proc();
			read_fd &= ~(1<<From_server);
		}
		if ( num_des > 0 )
		{
			for ( i = 0 ; i < NOFILE ; i++ )
				if ( (read_fd & (1<<i)) )
					ilwrite(i);
		}
	}
}/* main */

/*
** new_proc
** start up a new connection to an Ingres process
*/
new_proc()
{
	register	int	fd;
	auto		int	to_ioctl = 1;
	struct	sockaddr_in	addr;
	auto		int	len;

	len = sizeof (addr);
	if ( (fd = accept(From_server,&addr,&len)) != -1 )
	{
		Read_fmt |= (1<<fd);
		ioctl(fd,FIONBIO,&to_ioctl);
	}
# ifdef	DEBUG
	else
	{
		perror("accept");
		sleep(1);
	}
	printf("DRIVER: new file %d (%o)\n",fd,(1<<fd));
# endif	DEBUG
}/* new_proc */



ilwrite(read_desc)
register	int	read_desc;
{
	struct Lockreq	lockbuf;
	register int i;
	register int blockflag;
	extern	int	errno;

	errno = 0;
# ifdef	DEBUG
	printf("DRIVER: entering ilwrite, read_desc = %d\n",read_desc);
# endif	DEBUG
	if ( read(read_desc,&lockbuf, sizeof ( lockbuf)) != sizeof ( lockbuf ) )
	{
# ifdef	DEBUG
		printf("Read error, errno = %d\n",errno);
# endif	DEBUG
		if ( errno == EWOULDBLOCK )
			return;
		if ( errno == ECONNRESET )
		{
			ilrma(read_desc,TRUE);
			close(read_desc);
			Read_fmt &= ~(1<<read_desc);
			return;
		}
		send_info(read_desc,-5);
		return;
	}

# ifdef	DEBUG
	if (ildebug)
		printf("ildr: act %d, type %d, mode %d, read_desc %d\n",
			lockbuf.lr_act, lockbuf.lr_type, lockbuf.lr_mod, read_desc);
# endif	DEBUG
	if (( lockbuf.lr_act < A_RLS1)
	&& ((lockbuf.lr_type < T_CS) || (lockbuf.lr_type > T_DB )
	   || (lockbuf.lr_mod < M_EXCL) || (lockbuf.lr_mod > M_SHARE )))
	{
# ifdef	DEBUG
		printf("Illegal request\n");
# endif	DEBUG
		send_info(read_desc,-5);
		return;
	}
/*
 *		follow action from lock request
 */
	switch(lockbuf.lr_act)
	{
	  case A_RTN:
# ifdef	DEBUG
		if ( ildebug )
			printf("Driver: A_RTN\n");
# endif	DEBUG

		/*
		** attempt to set lock.
		** error return if failure.
		*/
		for ( i = 0; i <= lockbuf.lr_type; i++) 
		{
			if (Lockset[i] == 0) 
			{
# ifdef	DEBUG
				if (ildebug)
					printf("ildr: lock %d not available\n", i);
# endif	DEBUG
				send_info(read_desc,-1);
				return;
			}
		}
		if (ilunique(&lockbuf) >= 0) 
		{
			send_info(read_desc,-1);
			return;
		}
# ifdef	DEBUG
		if ( ildebug )
			printf("Driver: lock assigned\n");
# endif	DEBUG
		ilenter(&lockbuf,read_desc);
		break;

	  case A_SLP:
# ifdef	DEBUG
		if ( ildebug )
			printf("Driver: A_SLP\n");
# endif	DEBUG
		if ( set_lock(read_desc,lockbuf) == -1 )
			return;
# ifdef	DEBUG
		if ( ildebug )
			printf("Driver: got lock\n");
# endif	DEBUG
		break;
	  case A_RLS1:
				/* remove 1 lock */
# ifdef	DEBUG
		if ( ildebug )
			printf("Driver: A_RLS1\n");
# endif	DEBUG
		if ((i = ilfind(&lockbuf,read_desc)) >= 0)
		{
			ilrm(i,read_desc);
		}
		else
		{
			send_info(read_desc,-2);
			return;
		}
# ifdef	DEBUG
		if ( ildebug )
			printf("Driver: released\n");
# endif	DEBUG
		break;

	  case A_RLSA:
				/* remove all locks for this process id*/
# ifdef	DEBUG
		if ( ildebug )
			printf("Driver: A_RLSA\n");
# endif	DEBUG
		ilrma(read_desc,FALSE);
		break;

	  case A_ABT:		/* remove all locks */
# ifdef	DEBUG
		if ( ildebug )
			printf("Driver: A_ABT\n");
# endif	DEBUG
		ilclose();
		break;

	  default :
# ifdef	DEBUG
		if ( ildebug )
			printf("DRIVER: garbage\n");
# endif	DEBUG
		send_info(read_desc,-3);
	}
	send_info(read_desc,0);
}
/*
 *	ilunique- check for match on key
 *	
 *	return index of Locktab if match found
 *	else return -1
 */
static
ilunique(ll)
register struct Lockreq *ll;
{
	register int	k;
	register struct Lockform	*p;
	register struct Lockreq	*q;

	for (k = 0; k < NLOCKS; k++)
	{
		p = &Locktab[k];
		if ((p->l_mod != M_EMTY)
		&& (ilcomp(p->l_key,ll->lr_key) == 0)
		&& (p->l_type == ll->lr_type)
		&& ( (p->l_mod == M_EXCL) || (ll->lr_mod == M_EXCL)) ) {
# ifdef	DEBUG
			if (ildebug) {
				register int i;

				printf("ildr: lock ");
				for (i = 0; i < KEYSIZE; i++)
					printf("%c", ll->lr_key[i]);
				printf(" busy\n");
			}
# endif	DEBUG
			return(k);
		}
	}
	return(-1);
}

static
ilfind(ll,key)
register struct Lockreq *ll;
int	key;
{
	register int	k;
	register struct Lockform	*p;
	register struct Lockreq	*q;

	for (k = 0; k < NLOCKS; k++)
	{
		p = &Locktab[k];
		if ((p->l_mod != M_EMTY)
		&& (ilcomp(p->l_key,ll->lr_key) == 0)
		&& (p->l_type == ll->lr_type)
		&& (p->l_pid == key))
			return(k);
	}
	return(-1);
}/* ilfind */

/*
 *	remove the lth Lock
 *		if the correct user is requesting the move.
 */
static
ilrm(l,key,remove_all)
int l;
int	key;
int	remove_all;
{
	register struct Lockform *a;
	register	k;


	a = &Locktab[l];
	if (a->l_pid == key && a->l_mod != M_EMTY)
	{
		if ( !remove_all && a->l_type == T_DB )
			return;
		a->l_mod = M_EMTY;
		a->l_pid = 0;
		if (a->l_wflag == W_ON)
		{
			a->l_wflag = W_OFF;
			wakeup(&Locktab[l]);
		}
		for (k = 0; k <= a->l_type; k++)
		{
			Lockset[k]++;
			if (Lockset[k] == 1)
				wakeup(&Lockset[k]);
		}
	}
}/* ilrm */

/*
 *	ilrma releases all locks for a given process id(pd)
 *	-- called from sys1.c$exit() code.
 */
ilrma(key,remove_all)
int key;
int	remove_all;
{
	register int	i;

# ifdef	DEBUG
	printf("DRVIER: Removing all, key = %d\n",key);
# endif	DEBUG
	for ( i = 0; i < NLOCKS; i++ )
		ilrm(i,key,remove_all);
}

/*
 *	enter Lockbuf in locktable
 *	return position in Locktable
 *	error return of -1
 */
static
ilenter(ll,key)
register struct Lockreq *ll;
int key;
{
	int	k,l;
	register char	*f,*t;
	register struct Lockform	*p;

	for (k = 0; k < NLOCKS; k++)
	{
		p = &Locktab[k];
		if (p->l_mod == M_EMTY)
		{
			p->l_pid = key;
			p->l_type = ll->lr_type;
			Locktab[k].l_mod = p->l_mod = ll->lr_mod;
			f = ll->lr_key;
			t = p->l_key;
			for (l = 0; l < KEYSIZE; l++)
				*t++ = *f++;
			for (l = 0; l <= ll->lr_type; l++)
				Lockset[l]--;
# ifdef	DEBUG
			if ( ildebug )
				printf("DRIVER: ilenter %d, mod %d, omod = %d\n",k,p->l_mod,Locktab[k].l_mod);
# endif	DEBUG
			return(k);
		}
	}
# ifdef	DEBUG
	if ( ildebug )
		printf("DRIVER: ilenter -1\n");
# endif	DEBUG
	return (-1);
}

/*
 *	ilcomp- string compare
 *	  	returns 0 if strings match
 *		returns -1 otherwise
 */
static
ilcomp(s1,s2)
register char *s1,*s2;
{
	register int	k;

	for (k = 0; k < KEYSIZE; k++)
		if ( *s1++ != *s2++)
# ifdef	DEBUG
		{
			if ( ildebug )
				printf("DRIVER: ilcomp returning -1\n");
			return ( -1 );
		}
# else	DEBUG
			return (-1);
# endif	DEBUG
	return (0);
}

/*
 *	ilclose- releases all locks
 */
static
ilclose()
{
	register int	k;
	register caddr_t c;
# ifdef	DEBUG
	printf("DRIVER: entered close\n");
# endif	DEBUG

	for (k = 0; k < NLOCKS; k++)
		wakeup( &Locktab[k] );
	for (k = 0; k < 4; k++)
		wakeup( &Lockset[k]);
	for (c = (caddr_t)&Locktab[0].l_pid; c < (caddr_t)&Locktab[NLOCKS];)
		*c++ = 0;
	Lockset[0] = NLOCKS;
	Lockset[1] = PLOCKS;
	Lockset[2] = RLOCKS;
	Lockset[3] = DLOCKS;
}/* ilclose */

/*
** set_lock
** attempt to set a lock. If we can't, block the process and
** return -1, if we can than set the lock and return 0.
*/
set_lock(read_desc,lockbuf)
register	int	read_desc;
struct	Lockreq	lockbuf;
{
	register	int	blockflag;
	register	int	i;

	/*
	** attempt to set lock.
	** sleep on blocking address if failure.
	*/

	do
	{
		do
		{
			blockflag = TRUE;
			for ( i = 0; i <= lockbuf.lr_type; i++)
				if (Lockset[i] == 0)
				{
# ifdef	DEBUG
					if (ildebug)
						printf("ildr: lock %d not available\n", i);
# endif	DEBUG
					wait_on(read_desc,&Lockset[i],lockbuf);
					return(-1);
				}
		} while (!blockflag);

		if (( i = ilunique(&lockbuf)) >= 0 )
		{
			blockflag = FALSE;
			Locktab[i].l_wflag = W_ON;
			wait_on(read_desc,&Locktab[i],lockbuf);
			return(-1);
		}
	} while (!blockflag);
	ilenter(&lockbuf,read_desc);
	return ( 0 );
}/* set_lock */

/*
** send_info
** Send the data down the socket. Don't do it if it would cause the driver
** to block.
*/
send_info(fd,data)
register	int	fd;
int	data;
{
	auto	int	wdes = ( 1<<fd );
	struct	timeval	time;

	errno = 0;
	time.tv_sec = 10;
	time.tv_usec = 0;

	if ( select(NOFILE,0,&wdes,0,&time) != 1 )
	{
		Read_fmt &= ~(1<<fd);
		ilrma(fd,TRUE);
		close(fd);
	}
	else
		if ( write(fd,&data,sizeof (int)) != sizeof (int) )
		{
			if ( errno == 0 )
				return;
			Read_fmt &= ~(1<<fd);
			ilrma(fd,TRUE);
			close(fd);
		}
}/* send_info */

struct	Wait {
	int	wait_fd;		/* file descriptor to send lock info to off of */
	int	wait_lock;		/* what lock we are waiting for */
	struct	Lockreq	wait_req;	/* the lock request */
	struct	Wait	*next;
};
struct	Wait	*Wait_queue = NULL;

/*
** wait_on
** Set up to wait for a free lock.
*/
wait_on(fd,lock,req)
register	int	fd, lock;
struct		Lockreq	req;
{
	register	struct	Wait	*ptr;
	char	*calloc();

	ptr = (struct Wait *)calloc(1,sizeof (struct Wait));
	ptr->wait_fd = fd;
	ptr->wait_lock = lock;
	ptr->wait_req = req;
	ptr->next = Wait_queue;
	Wait_queue = ptr;
}/* wait_on */

/*
** wakeup
** See if there is anythng waiting on the newly freed lock. If there is,
** tell it it can have the lock now.
*/
wakeup(lock)
register	int	lock;
{
	register	struct	Wait	*ptr,*back;

	for ( back = NULL, ptr = Wait_queue ; ptr != NULL ; back = ptr, ptr = ptr->next )
	{
		if ( ptr->wait_lock == lock )
		{
			if ( set_lock(ptr->wait_fd,ptr->wait_req) == 0 )
			{
				send_info(ptr->wait_fd,0);
				if ( back != NULL )
					back->next = ptr->next;
				else
					Wait_queue = Wait_queue->next;
				cfree(ptr);
				return;
			}
		}
	}
}/* wakeup */

/*
** abnormal
** a signal has come down and hit us. We restart the entire
** program, and hope it goes away
*/
abnormal(sig)
int	sig;
{
	extern	int	errno;

# ifdef	DEBUG
	printf("DRIVER: error %d, restarting\n",sig);
# endif

	execl("/etc/lock_driver","lock_driver","restart",0);
	execl(Prog_name,Prog_name,"restart",0);
	execlp("lock_driver","lock_driver","restart",0);
	exit(4);
}/* abnormal */

/*
** close_up
** try and find a closed up file descriptor.
*/
close_up(fmt)
long	fmt;
{
	long	rdesc;
	register	int	i;
	struct	timeval	time;

	errno = 0;
	time.tv_sec  = 0;
	time.tv_usec = 0;

	for ( i = 0 ; i < NOFILE ; i++ )
	{
		if ( (1<<i) & fmt )
		{
			rdesc = (1<<i);
			if ( select(NOFILE,&rdesc,0,0,&time) == -1 )
			{
				/*
				** the server socket has closed down.
				** BOY ARE WE IN TROUBLE
				*/
				if ( i == From_server )
				{
					sleep(1);
# ifdef	DEBUG
					printf("Restarting socket\n");
# endif	DEBUG
					init_socket();
				}
				if ( errno == EBADF )
				{
					shutdown(i,2);
					close(i);
					Read_fmt &= ~(1<<i);
					ilrma(i,TRUE);
				}
			}
		}
	}
}/* close_up */

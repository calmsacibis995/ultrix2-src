/*	invcutter.c --
 *		generate a full inventory file from a scrawny one.
 *
 *			Copyright (c) 1985 by
 *		Digital Equipment Corporation, Maynard, MA
 *			All rights reserved.
 *								
 *	This software is furnished under a license and may be used and
 *	copied  only  in accordance with the terms of such license and
 *	with the  inclusion  of  the  above  copyright  notice.   This
 *	software  or  any  other copies thereof may not be provided or
 *	otherwise made available to any other person.  No title to and
 *	ownership of the software is hereby transferred.		
 *								
 *	This software is  derived  from  software  received  from  the
 *	University    of   California,   Berkeley,   and   from   Bell
 *	Laboratories.  Use, duplication, or disclosure is  subject  to
 *	restrictions  under  license  agreements  with  University  of
 *	California and with AT&T.					
 *								
 *	The information in this software is subject to change  without
 *	notice  and should not be construed as a commitment by Digital
 *	Equipment Corporation.					
 *								
 *	Digital assumes no responsibility for the use  or  reliability
 *	of its software on equipment which is not supplied by Digital.
 *
 *	Mods:
 *		2.0,1	ccb	14-jan-1987
 *		Unify directory type to compensate for tar change leaving
 *			empty dirs off tape.
*/

#ifndef lint
static char *sccsid = "@(#)invcutter.c	1.4	ULTRIX	1/15/87";
#endif lint

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/file.h>
#include	<sys/stat.h>
#include	<sys/dir.h>
#include	<stdio.h>
#include	<errno.h>

#define	Usage()	fprintf(stderr,"Usage: %s [vcode]\n",prog)
#define	FACLEN	16

#define	INO	f_st.st_ino
#define	DEV	f_st.st_dev
#define	MODE	f_st.st_mode

typedef struct fnode {
	struct fnode 	*f_next;		/* next node pointer */
	char		f_path[MAXPATHLEN];	/* file path name */
	char		f_fac[FACLEN];		/* facility name */
	char		f_sub[FACLEN];		/* subset name */
	char		f_type;			/* file type */
	char		f_linkto[MAXPATHLEN];	/* linkto pathname */
	unsigned	f_icnt;			/* hard link count */
	unsigned	f_sum;			/* checksum */
	struct stat	f_st;			/* stat data for this file */
} fnode;

#define	READSIZ	BUFSIZ*8
#define	OUTFMT "%s\t%d\t%05u\t%u\t%u\t%6o\t%d/%d/%d\t%s\t%c\t%s\t%s\t%s\n"

extern char *sys_errlist[];
extern int errno;

/* Function Types */
char		*emalloc();
unsigned	getsum();
char		*malloc();

/* Global Variables */
fnode		*the_list;	/* the link sorted output list */
fnode		*newp;
char 		*prog;		/* program name */
struct tm	*mtime;		/* time structure */
char		buf[READSIZ];	/* generic io buffer */
int		error;
int		needmem;	/* memory allocation flag */
char		*version;	/* version code */
char		fullpath[MAXPATHLEN];

/*MAIN*/
main(argc,argv)
int argc;
char *argv[];
{
	fnode	*nptr, *symp, *symprev;
	int	msgs, icount;
	char	mntpath[MAXPATHLEN];

	prog = *argv;

	version = "V1.0";
	*mntpath = '\0';
	while( ++argv, --argc )
	{
		if( !strcmp(*argv,"-v" ) )
		{
			version = *++argv;
			--argc;
		}
		else if( *mntpath != '\0' )
		{
			Usage();
			exit(1);
		}
		else
			strcpy(mntpath,*argv);
	}

	if( !strcmp(mntpath, "/") )
		*mntpath = '\0';
#ifdef DEBUG
	fprintf(stderr,"mntpath is %s\n",mntpath);
#endif


	the_list = (fnode *) 0;
	needmem = 1;
	error = 0;

	/* for all the lines in the input file */
	while( gets(buf) != NULL )
	{
		/* get new node to stuff */
		if( needmem )
		{
			newp = (fnode *) emalloc( sizeof(fnode) );
			needmem = 0;
		}

		icount = sscanf(buf,"%s\t%s\t%s", newp->f_fac,
			newp->f_path, newp->f_sub);

		if( icount != 3 )
		{
			fprintf(stderr,"bad input: %s\n", buf);
			++error;
			continue;
		}

		/* lstat the file, if fac != "zero", prepend mntpath */
		if( strcmp(newp->f_fac,"zero") )
			sprintf(fullpath,"%s%s",mntpath,newp->f_path+1);
		else
			strcpy(fullpath,newp->f_path);

		if( lstat(fullpath, &newp->f_st) != 0 ) 
		{	/* file error */
			fprintf(stderr,"%s: can't stat %s (%s)\n",
				prog, fullpath, sys_errlist[errno]);
			++error;
			continue;
		}

		if( ((newp->MODE & S_IFMT) == S_IFCHR) ||
			((newp->MODE & S_IFMT) == S_IFBLK) ||
			((newp->MODE&S_IFMT) == S_IFSOCK) )
		{
			/* error, no sockets, special files permitted */

			fprintf(stderr,"%s: %s: illegal file type code %o\n",
				prog, fullpath, newp->f_st.st_mode&S_IFMT);
			++error;
			continue;
		}

		else if( (newp->MODE & S_IFMT) == S_IFLNK )
		{
			int	nchars;

			newp->f_type = 's';

			nchars = readlink(fullpath,newp->f_linkto,BUFSIZ);
			if( nchars == -1 )
			{
				fprintf(stderr,
					"%s: readlink %s failed, (%s)\n",
					prog, newp->f_path, sys_errlist[errno]);
				++error;
				return;
			}
			/* readlink doesnt null terminate result buffer */
			(newp->f_linkto)[nchars] = '\0';
			newp->f_icnt = newp->f_st.st_nlink - 1;
			newp->f_sum = 0;
			newp->f_next = the_list;
			the_list = newp;
		}

		else if( newp->MODE & S_IFDIR )
		{
			/* link in and forget about it */
			newp->f_type = 'd'; /* (char)getdtype(fullpath); 2.0,1*/
			strcpy(newp->f_linkto,"none");
			newp->f_icnt = 0;
			newp->f_next = the_list;
			the_list = newp;
		}
		else if( newp->f_st.st_nlink > 1 )
			do_hardlink();

		else if( newp->MODE & S_IFREG )
		{
			/* regular, no special attributes. */
			newp->f_type = 'f';
			newp->f_icnt = newp->f_st.st_nlink - 1;
			if( newp->f_st.st_size == 0 )
				newp->f_sum = 0;

			newp->f_sum = getsum(fullpath);

			strcpy( newp->f_linkto, "none" );
			newp->f_next = the_list;
			the_list = newp;
		}

		needmem = 1;
	}
	
	/* any pending hard links? */
	for( nptr = the_list, msgs = 0; nptr != (fnode *) 0; nptr=nptr->f_next )
	{
		if( nptr->f_icnt != 0 )
		{
			if( !msgs )
			{
				fprintf(stderr,"%s: Unresolved hard links!\n",
					prog);
				++msgs;
			}
			fprintf(stderr,"File %s, icnt %d\n", nptr->f_path,
				nptr->f_icnt);
			++error;
		}
	}
	for( nptr = the_list; nptr != (fnode *) 0; nptr = nptr->f_next )
	{
		/* output formatted information */
		mtime = localtime(&nptr->f_st.st_mtime);
		printf(OUTFMT, nptr->f_fac, nptr->f_st.st_size, nptr->f_sum,
			nptr->f_st.st_uid, nptr->f_st.st_gid,
			nptr->f_st.st_mode, mtime->tm_mon+1, mtime->tm_mday,
			mtime->tm_year, version, nptr->f_type, nptr->f_path,
			nptr->f_linkto, nptr->f_sub );
	}
	exit(error);

}


/* hard link code */
do_hardlink()
{
	register fnode	*nptr;	/* ptr for crawling down lists */

	/* hard link, check if a name for this device/inode
	 *  combination exists check the_list first.
	 *
	 * buzz down the_list looking for dev/inode matching that for
	 * the file @ newp.
	*/
	for( nptr = the_list; nptr != (fnode *) 0; nptr = nptr->f_next )
		if( nptr->DEV == newp->DEV && nptr->INO == newp->INO )
		{
			h_insert_after(nptr);
			break;
		}

	if( nptr == (fnode *) 0 )	/* did not find desired inode */
	{
		/* new: linkto none, get link count, get checksum. */
		newp->f_type = 'f';
		strcpy(newp->f_linkto,"none");
		newp->f_icnt = newp->f_st.st_nlink - 1;
		if( newp->f_st.st_size == 0 )
			newp->f_sum = 0;
		else
			newp->f_sum = getsum(fullpath);
		newp->f_next = the_list;
		the_list = newp;
	}
}


/*	h_insert_after() -
 *		insert a hard-linked node after a given node and do all
 *		associated bookkeeping.
 *
 *	given:	a pointer to an fnode.
 *	does:	links the fnode into the_list, copying all size, link to
 *		and link count information from the major (first found)
 *		file of the link set, makes sure that all hard links in
 *		this link cluster have the icnts decremented to account
 *		for the arrival of this file.
 *	return:	nothing, all done with side effects.
*/

h_insert_after(nptr)
register fnode *nptr;
{
	register fnode	*xptr;	/* pointer into the_list */
	int		icheck;	/* unused hard node counter */

	/* set sum and link count */
	newp->f_sum = nptr->f_sum;
	newp->f_icnt = --nptr->f_icnt;

	/* drop link counts on all the neighbors too! */
	for( xptr = nptr->f_next; xptr != (fnode *) 0; xptr = xptr->f_next )
	{
		if( xptr->DEV == newp->DEV && xptr->INO == newp->INO )
			--xptr->f_icnt;
		else
			break;
	}	

	/* set linkto name on new node */
	newp->f_type = 'l';
	strcpy( newp->f_linkto, nptr->f_path );
	
	/* insert the node. */
	newp->f_next = nptr->f_next;
	nptr->f_next = newp;
}


/*	getdtype() -
 *		return directory type code 'd' or 'e'.
*/

/* COMMENT OUT DETDTYPE() 2.0,1

getdtype(s)
char *s;
{
	struct direct	*slot;
	register DIR	*dirp;
	int		empty;

	if( (dirp = opendir(s)) == NULL )
	{
		fprintf(stderr,"%s: can't open dir %s (%s)\n", prog,
			s, sys_errlist[errno]);
		exit(1);
	}
	if( (slot = readdir(dirp)) == NULL )
	{
		fprintf(stderr,"%s: error reading '.' from %s (%s)\n",
			prog, s, sys_errlist[errno]);
		exit(1);
	}
	if( strcmp(slot->d_name,".") )
	{
		fprintf(stderr, "%s: directory %s: '.' expected.\n",
			prog, s);
		exit(1);
	}
	if( (slot = readdir(dirp)) == NULL )
	{
		fprintf(stderr,"%s: error reading '..' from %s (%s).\n",
			prog, s, sys_errlist[errno]);
		exit(1);
	}
	if( strcmp(slot->d_name,"..") )
	{
		fprintf(stderr, "%s: directory %s: '..' expected.\n",
			prog, s);
		exit(1);
	}
	empty = (readdir(dirp) == NULL);
	closedir(dirp);
	return( (int) (empty ? 'e' : 'd') );
}

* GETDTYPE() COMMENTED OUT 2.0,1 */

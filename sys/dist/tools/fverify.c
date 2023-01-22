/* fverify.c
 *
 * Name:	fverify
 * Purpose:	Verify ULTRIX installation.
 * Usage:	fverify [-f][yn] <inventory
 *			-f specifies fast mode, do not check sizes
 *			and checksums.
 *			-y quietly repair everything
 *			-n just report inconsistencies
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
 *	The information in this software is subject to change  without
 *	notice  and should not be construed as a commitment by Digital
 *	Equipment Corporation.					
 *								
 *	Digital assumes no responsibility for the use  or  reliability
 *	of its software on equipment which is not supplied by Digital.
 *
 * Environment:	VAX/ULTRIX
 * Compile:	cc fverify.c getsum.c
 * Date:	1/11/84
 * Author:	Alan Delorey
 * Remarks:
 *  Verifies the existence and attributes of files.
 *  It reads input lines(using scanf) from the inventory until EOF is reached.
 *  It calls lstat to get info about each file, and compares to master list.
 *  The master attributes file should be stdin.
 *
 *	HISTORY:
 *	001	ccb 	09-03-1985
 *		Fix indirection bug preventing correct verification
 *		of directories and symbolic links.
 *		Increased performance in checksum().
 *		Overall 25% performance increase.
 *
 *	002	ccb	1986.03.15
 *		Add interactive repair, force repair, no repair.
 *		Add fast mode that ignores sizes and sums.
 *		Accelerate I/O.
 *		Stop repetitive checks for files hard linked together.
 *		Add mode checking for soft links and directories.
 *		Add time stamps for the output log.
 *
 *	003	ccb	1986.09.16
 *		drop in code for interrupt handling
 *		drop in code for correction of symbolic links
 *		clean up informational messages.
 *
*/
#ifndef lint
static	char	*sccsid = "@(#)fverify.c	1.8 (ULTRIX) 2/12/87";
#endif lint


#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/file.h>
#include	<sys/errno.h>
#include	<sys/time.h>
#include	<signal.h>
#include	<stdio.h>

#include	"invdist.h"

#define	VFYLOG	"./usr/adm/fverifylog"


#define	UID	0
#define	GID	1
#define	MODE	2

#define	Usage()	fprintf(stderr,"Usage: %s [-f][yn]\n",prog)
#define	Logmsg(S)	printf("%s LOGGING AT %s\n", S,\
				asctime( localtime( (time(&t),&t) ) ) );

extern errno;
extern char *sys_errlist[];

FILE		*freopen();
FILE		*fopen();
long		time();
char		*asctime();
struct tm	*localtime();

char		*prog;		/* program name pointer */
char		buf[BUFSIZ];	/* generic buffer */
int		eco;		/* error counter */
int		fixed;		/* repair counter */
long		t;		/* time pointer */
int		onintr();	/* interrupt trap */

static char	errfmt[] = "\t%s: incorrect %s\n";

struct invq {
	struct invq	*q_next;
	INV		*q_data;
};

main(argc,argv)
int argc;
char *argv[];
{
	FILE		*tt;	/* file descriptor for terminal input */
	int		fast;	/* ignore size, sum flag */
	int		hush;	/* don't ask about repairs */
	int		fix;	/* repair */
	int		line;	/* input line number */
	INV		i;	/* holds input lines */
	struct invq	*dirq;	/* directories to be created */
	struct invq	*qp;	/* pointer for fresh ones */
	struct stat	st;	/* holds stat info about actual file */
	char		*cp;	/* pointer for decoding args */
	int		ret;	/* temp for sscanf return */


	signal(SIGINT,onintr);
	prog = *argv;

	/* flag defaults, be careful if modifying! */
	fast = hush = 0; fix = 1;

	dirq = (struct invq *) 0;
	/* get command flags */
	while( ++argv, --argc )
	{
		if( **argv == '-' )
		{
			for(cp = *argv; *++cp != '\0';)
			{
				switch( *cp )
				{
				case 'f':
					++fast;
					break;
				case 'n':
					--fix;
				case 'y':
					++hush;
					break;
				default:
					Usage();
					exit(1);
				}
			}
		}
		else
		{
			Usage();
			exit(1);
		}
	}



	if( freopen( VFYLOG, "a", stdout) == (FILE *) 0 )
	{
		fprintf(stderr, "%s: cannot open log file (%s)\n",
			prog, sys_errlist[errno]);
		errno=0;
	}

	Logmsg("BEGIN");

	if( !hush )	/* need to talk to tty */
	{
		if( eco += (tt = fopen("/dev/tty", "r")) == (FILE *) 0 )
		{
			sprintf(buf,"%s: cannot open /dev/tty (%s)\n",
				prog, sys_errlist[errno]);
			puts(buf);
			fputs(buf,stderr);
		}
		/* literal 2 for stderr */
		if( !isatty(2) )
		{
			sprintf(buf,"%s: error output is not a tty\n",
				prog);
			puts(buf);
			fputs(buf,stderr);
			++eco;
		}
			
		if(eco)
		{
			sprintf(buf,"%s: error reporting only, no fixes.\n",
				prog);
			puts(buf);
			fputs(buf,stderr);
			++hush; --fix;
		}
	}

	/* note: eco starts fresh in the loop below. */

	/* read lines and check file attributes */
	for( fixed = eco = 0, line = 1; gets(buf) != NULL; ++line )
	{
		ret = sscanf(buf, INVIFMT, i.i_fac, &i.i_size, &i.i_sum,
			&i.i_uid, &i.i_gid, &i.i_mode, i.i_date,
			i.i_rev, &i.i_type, i.i_path, i.i_linkto,
			i.i_sub);

		if(ret != INV_NFLDS)
		{
			sprintf(buf,
				"%s: Not enough input values on line %d\n",
				prog, line);
			/* look at next if for prerr */
			goto prerr;
		}

		if( lstat(i.i_path,&st) )
		{
		
			if( i.i_type == 'd' )
			{
				qp = (struct invq *)
					malloc( sizeof(struct invq) );
				qp->q_data = (INV *) malloc(sizeof(INV));
				bcopy((char *)&i,(char *)qp->q_data,sizeof(INV));
				/* insert at head becuase inventory is
				 *  in reverse ascii collating seq. :. we must
				 *  queue so that any missing parent dirs which
				 *  may appear later get created first.
				*/
				qp->q_next = dirq;
				dirq = qp;
			}
			else
			{
				sprintf(buf, "\t%s: %s\n",
					i.i_path, sys_errlist[errno]);
prerr:				puts(buf);
				fputs(buf,stderr);
			}
			++eco;
			continue;
		}

		if(( (i.i_type == 'd' || i.i_type == 'e') &&
			(S_IFMT & st.st_mode) != S_IFDIR) ||
			(i.i_type == 's' && (S_IFMT & st.st_mode) != S_IFLNK))
		{
			sprintf(buf,errfmt,i.i_path,"type");
			puts(buf);
			fputs(buf,stderr);
			++eco;
		}

		/* only regular file get summed and sized */
		if( !fast && i.i_type == 'f' )
		{
			if( i.i_sum != getsum(i.i_path) )
			{
				sprintf(buf,errfmt,i.i_path,"checksum");
				puts(buf);
				fputs(buf,stderr);
				++eco;
			}
			if( st.st_size != i.i_size )
			{
				sprintf(buf,errfmt,i.i_path,"size");
				puts(buf);
				fputs(buf,stderr);
				++eco;
			}
		}

		/* these are potentially correctable */
		if( st.st_gid != i.i_gid )
		{
			sprintf(buf,errfmt,i.i_path,"gid");
			puts(buf);
			fputs(buf,stderr);
			++eco;
			if(fix)
				fixed += ffix(GID,hush,tt,&st,&i);
		}
		if( st.st_uid != i.i_uid )
		{
			sprintf(buf,errfmt,i.i_path,"uid");
			puts(buf);
			fputs(buf,stderr);
			++eco;
			if(fix)
				fixed += ffix(UID,hush,tt,&st,&i);

		}
		/* check all mode bits */
		if( st.st_mode != i.i_mode )
		{
			sprintf(buf,errfmt,i.i_path,"mode");
			puts(buf);
			fputs(buf,stderr);
			++eco;
			/* only lower mode bits are fixable */
			if( fix && (st.st_mode&~S_IFMT) != (i.i_mode&~S_IFMT) )
				fixed += ffix(MODE,hush,tt,&st,&i);
		}
	}
	/* attempt to create directories queued from lstat failure */
	umask(0);
	for( qp = dirq; qp != (struct invq *) 0; qp = qp->q_next )
	{
		INV	*ip;
		ip = qp->q_data;
		sprintf(buf,"\tCreating directory %s\n",ip->i_path);
		puts(buf);
		fputs(buf,stderr);
		if( mkdir(ip->i_path, ip->i_mode) )
		{
			sprintf(buf,"\tCannot create dir %s (%s)\n",
				ip->i_path, sys_errlist[errno]);
			puts(buf);
			fputs(buf,stderr);
			continue;
		}
		chown(ip->i_path,ip->i_uid,ip->i_gid);
		chmod(ip->i_path,ip->i_mode);
		++fixed;
	}
	fvexit();
}



/*	ffix() --
 *		attempt to correct system discrepancies.
 *
 *	given:	fix code in {UID,GID,MODE}
 *		hush flag
 *		terminal FILE pointer.
 *		pointer to stat structure.
 *		pointer to inv structure.
 *	does:	[interactively] fixes inconsistencies between the
 *		inventory and the files on the system.
 *	return:	1 if fix succeeded, 0 if not. Main() uses return code to
 *		bump a counter of successful fixes.
*/

ffix(code,h,fp,st,ip)
int code, h;
FILE *fp;
struct stat *st;
INV *ip;
{
	char	*cp;
	int	err, omask, bits;

	if(!h)
	{
		fprintf(stderr,"Fix (y/n/q) [n]? ");
		fgets(buf,BUFSIZ,fp);
		/* elide begining spaces */
		for(cp=buf; *cp == ' ' || *cp == '\t' && *cp != '\0';)
			++cp;

		/* no action, return */
		if( *cp == 'n' || *cp == 'N' || *cp == '\n' || *cp == '\0' )
			return(0);

		/* quit!, exit */
		if( *cp == 'q' || *cp == 'Q' )
			fvexit();
	}
	switch( code )
	{
	case UID:
		if( !(err = chown( ip->i_path, ip->i_uid, st->st_gid )) )
			st->st_uid = ip->i_uid;
		break;
	case GID:
		if( !(err = chown( ip->i_path, st->st_uid, ip->i_gid )) )
			st->st_gid = ip->i_gid;
		break;
	case MODE:
		bits = ip->i_mode & ~S_IFMT;
		if( (ip->i_mode & S_IFLNK) == S_IFLNK ) /* symbolic link */
		{
			if( (err=readlink(ip->i_path,buf,sizeof(buf))) == -1 )
				break;
			buf[err] = '\0';
			omask = umask( ~bits );
			if( err = unlink(ip->i_path) )
				break;
			err = symlink(buf,ip->i_path);
		}
		else
			err = chmod( ip->i_path, ip->i_mode & ~S_IFMT );
		break;
	default:
		sprintf(buf,
			"%s: Internal Error in ffix(), code %d unknown.\n",
			prog, code);
		puts(buf);
		fputs(buf,stderr);
		return(0);
	}
	if(err)
		sprintf(buf,"\tCannot correct %s (%s)\n", ip->i_path,
			sys_errlist[errno]);
	else
		sprintf(buf,"\t%s corrected.\n", ip->i_path);

	puts(buf);
	fputs(buf,stderr);
	return(!err);
}


/*	bcopy() -
 *		transfer bytes.
*/

bcopy(s,t,n)
register char *s, *t;
register int n;
{
	while(n--)
		*t++ = *s++;
}


/*	onintr() -
 *		log interrupt and exit.
*/

onintr()
{
	Logmsg("INTERRUPT!");
	fvexit();
}


/*	fvexit() -
 *		log exit information and exit
*/

fvexit()
{
	sprintf(buf,"\t%d %s\n\t%d %s\n",
		eco, "verification errors encountered.",
		fixed, "corrections performed.");

	puts(buf);
	fputs(buf,stderr);

	Logmsg("END");
	exit(eco-fixed);
}

/*	tarsets.c -
 *		write media image production scripts.
 *
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
 *	000	afd	1984
 *	001	ccb	1986.03.14
 *		massive cleanup.
 *		add incorporation of volume files.
 *		place inventory files on front of first floppy.
 *	002	ccb	05-may-1986
 *		fix to allow empty directories
 *		add copyright info
 *	003	ccb	27-may-1986
 *		fix to disallow non-empty directories
 *		left justify volume description
 *		make > 400kb floppies non-fatal.
 *	2.0,1	add 'o' flag to tar commands.
*/

#ifndef lint
static	char *sccsid = "@(#)tarsets.c	1.14		1/15/87";
#endif lint

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

#define BLKSIZ		512	/* blocksize */
#define RX_SIZ		797	/* size in blocks (2 less for tar eof) */
#define TARSIZE		80	/* # of (512 byte) blocks for tar xfers */
#define TARPAD		1	/* tar overhead per file */
#define	FACLEN		16	/* max length of subset and facility names */
#define	REG		'f'	/* regular file type code */
#define	DIR		'd'	/* directory file type code */
#define HLINK		'l'	/* hard link type code */
#define	SLINK		's'	/* soft link type code */

/* bytes to blocks macro */
#define	BYTOBL(S)	( ( (S)->i_size + BLKSIZ ) / BLKSIZ + TARPAD)
#define	Usage()		fprintf(stderr,"Usage: %s master-dir\n",prog);

extern	errno;
extern	*sys_errlist[];


typedef struct inv	/* record for list of distribution files */
{
	struct inv	*i_next;		/* link to next file record */
	char		i_path[MAXPATHLEN];	/* file name */
	char		i_linkto[MAXPATHLEN];	/* link to name */
	struct inv	*i_hpl;		/* ptr to hard links */
	char		i_used;			/* file allocated flag */
	char		i_type;			/* type from [lsdf] */
	off_t		i_size;			/* size of the file */
} invnode;

static char *move_files[] =
{				/* files that get moved to floppy 1 */
	"/usr/bin/ex",		/* for decrypt reasons */
	"/bin/e",
	"/lib/libcg.a",		/* G-float version of libc */
	NULL
};

/* Define these so C will know what type they are. */
invnode	*look_ahead();
char	*emalloc();

char	buf[BUFSIZ];
char	subset[FACLEN];
char	*prog;
char	mntpath[MAXPATHLEN];
struct stat stb;

main (argc,argv)
int argc;
char *argv[];
{
	invnode			*head;		/* main list */
	register invnode	*newp;		/* current node */
	int			needmem;	/* memory allocation flag */

	if(argc != 2 )
	{
		Usage();
		exit(1);
	}
	strcpy(mntpath,argv[1]);
	fprintf(stderr, "Stderr is redirected to diagnostics file `stderr'\n");
	freopen("stderr", "a", stderr);


	/* While not end of input file:
	 *   read lines and store files in linked list.
	*/
	head = (invnode *) 0;
	for(needmem = 1;gets(buf) != NULL;)
	{
		if(needmem)
		{
			newp = (invnode *) emalloc(sizeof(invnode));
			needmem=0;
		}

		/*! check return val !*/
		sscanf(buf,"%*s%d%*d%*hd%*hd%*ho%*s%*s%1s%s%s%s",
			&newp->i_size, &newp->i_type, newp->i_path,
			newp->i_linkto, subset );

		/* DIR == directory */
		if( newp->i_type == DIR )
			continue;

		if( newp->i_type == HLINK && strcmp(newp->i_linkto, "none") )
		{
			/* hard link is not the first, call h_append
			 *  to make sure it gets pointed to along the
			 *  i_hlp chain.
			*/
			h_append(newp,head);
		}
		else
		{
			newp->i_next = head;
			head = newp;
		}
		needmem = 1;
	}   /* end while getting input lines */

	bld_script(head);
	exit(0);

}



/*	bld_script() -
 *		write the output script.
 *
 *	given:	a pointer to the head of the inv list.
 *	does:	arbitrates the output of tar commands such that no
 *		command would overflow a single diskette.
 *	return:	no explicit return value.
*/

bld_script(head)
invnode *head;
{
	int		nf;		/* current floppy number */
	int		blks;		/* number of blocks consumed */
	int		bcnt;		/* number of blocks in current file */
	int		done;		/* floppy is full flag */
	invnode		*ip;		/* place holder in processing list */
	invnode		*xp;		/* look ahead pointer */
	invnode		*hp;		/* ptr to count hlink overhead */
	char		*cmdfmt;	/* tar cmd printf format string */

	nf = 1;
	done = blks = 0;
	cmdfmt = "tar cpobf %d ${TO}/%s%d \\\nVolume%d\\\n -C %s";

	printf(cmdfmt, TARSIZE, subset, nf, nf, mntpath);

	blks = floppy1(head);	/*! current impl, first floppy mgmt !*/

	/* Loop until end of linked list is found */

	for(ip = head; ip != (invnode *) 0; )
	{
		if(ip->i_used)
		{
			ip = ip->i_next;
			continue;
		}
		if( (bcnt = BYTOBL(ip)) > RX_SIZ )
		{
			fprintf(stderr, "Warning: file %s is %d blocks",
				ip->i_path, ip->i_size);
			fprintf(stderr, "\ttoo large for diskette\n\n");
			ip = ip->i_next;
			continue;
		}

		/* Count overhead for hard links */
		for( hp = ip->i_hpl; hp != (invnode *) 0; hp = hp->i_hpl)
			bcnt += TARPAD;

		/* Determine if current file fits on current floppy */
		if(blks + bcnt > RX_SIZ)
		{
			/* It doesn't fit so find one that will fit */
			xp = look_ahead(ip->i_next, blks);
			if(xp != (invnode *) 0)
				blks += output(xp);

			else
				done = 1;
		}
		else
		{
			/* file fits */
			blks += output(ip);
			ip = ip->i_next;
		}
		if (done || blks >= RX_SIZ)
		{
			fprintf(stderr, "%d Blocks on floppy %s%d\n",
				blks, subset, nf);

			/* set blocks counting volume file size */
			sprintf(buf,"Volume%d",++nf);
			stat(buf,&stb);
			blks = (stb.st_size + BLKSIZ ) / BLKSIZ + TARPAD;

			puts(";\n");
			printf(cmdfmt, TARSIZE, subset, nf, nf, mntpath);
			done = 0;
		}
	}

	fprintf(stderr, "%d Blocks on floppy %s%d\n", blks, subset, nf);
	fprintf(stderr, "Writing oversize floppies...\n");

	/* kludge for tapes, create records for oversized floppies. */
	for( ip = head; ip != (invnode *) 0; ip=ip->i_next)
	{
		/* all oversize are currently not marked used */
		if( !(ip->i_used) )
		{
			sprintf(buf,"Volume%d",++nf);
			stat(buf,&stb);
			blks = (stb.st_size + BLKSIZ ) / BLKSIZ + TARPAD;
			
			puts(";\n");
			printf(cmdfmt, TARSIZE, subset, nf, nf, mntpath);
			fprintf(stderr,"%d Blocks on floppy %s%d\n",
				output(ip), subset, nf);
		}
	}
	puts("\n");
}



/*	floppy1() -
 *		seek out nodes for volume one.
 *
 *	given:	pointer to the head node of the list.
 *	does:	seeks out files listed in the global table move_files
 *		and has the paths put on output, flagging them as used.
 *	return:	the number of media blocks consumed.
 *	note:	uses globals move_files.
*/

floppy1(p)
register invnode *p;
{
	register char	**cp;
	int		blks;
	struct stat	stb;

	/* account for size of volume file */
	stat("Volume1",&stb);
	blks = (stb.st_size + BLKSIZ ) / BLKSIZ + TARPAD;

	for(; p != (invnode *) 0; p = p->i_next)
	{
		for(cp=move_files; *cp != NULL; ++cp)
			if( !strcmp( p->i_path, *cp ) )
				blks += output(p);
	}
	return(blks);
}



/*	output() -
 *		print pathname from node.
 *
 *	given:	pointer to a node.
 *	does:	print a line for the script containing line
 *		line continuation and the node->path.
 *		track down all hard links to this node and
 *		do likewise for them. track tar padding
 *		for the hard link records.
 *	return:	number of blocks consumed by files and links for which
 *		output records were created.
*/

output(p)
invnode *p;
{
	register invnode	*hp;	/* hard link chain pointer */
	int			bc;	/* block count */

	printf(" \\\n%s", p->i_path);
	bc = BYTOBL(p);
	p->i_used = 1;
	for(hp = p->i_hpl; hp != (invnode *) 0; hp = hp->i_hpl)
	{
		printf(" \\\n%s", hp->i_path);
		bc += TARPAD;
		hp->i_used = 1;
	}
	return(bc);
}



/*	look_ahead() -
 *		scan for next node that fits the floppy.
 *
 *	given:	the current node pointer
 *		the current fill total for the volume.
 *	does:	scan the list for unused file that won't overflow
 *		the remaining media space. Backpacking.
 *	return:	if a node that fits is found, a pointer to that node.
 *		otherwise (invnode *) 0.
*/

invnode *look_ahead (p, blks)
register invnode *p;	
int blks;			
{
	for (; p != NULL; p = p->i_next)
	{
		if(!p->i_used)
			if(blks + BYTOBL(p) <= RX_SIZ)
				break;
	}
	return(p);
}


/*	h_append() -
 *		add hard link node to list
*/

h_append(n,l)
register invnode *n, *l;
{
	/* find node in l to which p should be linked */
	for(; l != (invnode *) 0; l = l->i_next)
	{
		if(!strcmp(l->i_path, n->i_linkto))
		{
			/* got it, link into hlink chain */
			n->i_hpl = l->i_hpl;
			l->i_hpl = n;
			return;
		}
	}
	fprintf(stderr, "%s: %s -> %s link reference unresolved.\n",
		prog, n->i_path, n->i_linkto);
}



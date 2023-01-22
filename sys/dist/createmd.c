/*
 * createmd.c
 *
 * Name:	createmd
 * Purpose:	Generate Bourne Shell script which builds the disk
 *		distribution kit in TAR format.
 * Usage:	createmd [/mntpath] <inventory >makedist
 *		    Where inventory is the inventory from getstat.
 *		Note: run as super-user for file access privilege!
 *		      and with path set to Root.
 * Environment:	Ultrix-32
 * Compile:	cc createmd.c
 * Date:	1/11/84
 * Author:	Alan Delorey
 * Dependencies:
 *    MicroVAX Inventory.
 *    File containing list of boot files.
 * Remarks:
 *   This utility needs the MicroVAX inventory (w/ out the descriptions).
 *
 *   It generates the shell script which in turn will build the
 *   distribution kit on RX50 disks in TAR format.
 *
 *   The list of files which went on the boot disks is read in so that
 *   they will NOT be put on the distribution disks a second time.
 *
 * MODIFICATION HISTORY:
 *   March 28, 1984  aps
 *	Added error recovery options for failing tar's.
 *
 *   March 28, 1984  aps
 *	Fixed the program so that the shell file made is
 *	  for the cshell and it parameterized.  Also fixed
 *	  a little bug (rb0a to rb0c).
 *
 *   27-Mar-84 afd
 *	Made this version for Rl02 distribution.
 *
 *   20-Feb-84 afd
 *	Added solution to knapsack problem.
 *	It moves several files to the first disk.  Those that have
 *	    decryption code in them.  These files are specified in the
 *	    static structure "move_files".
 */

#define RL1 "/dev/rrb0c"	/* 1st rx drive */
#define TARG "kbf"		/* default arguments to tar */
#define TARSIZE "80"		/* # of (512 byte) blocks for tar xfers */
#define BLOCKSIZE 512		/* blocksize */
#define DISKSIZE 20400		/* size in blocks */
#define OVERHEAD 1		/* tar overhead per file */
#define PATHSIZE 100		/* max size to allocate for pathnames */
#define OUTSCRIPT "makedist"	/* the output script name */
#define HARDLINK "l"		/* type designation for hard link */
#define true 1
#define false 0
#define BUFSIZE 1024		/* max cmd line size (allow space below 10K)*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>

/* Define these so C will know what type they are. */
char *calloc();
char *malloc();

struct bfiles		/* record for list of boot files */
    {
    char *pathname;
    struct bfiles *next;
    };

struct inv		/* record for list of distribution files */
    {
    char *facility, *date, *rev, *type, *pathname, *links_to;
    short uid;
    short gid;
    u_short mode;
    long blocks;
    struct inv *next;		/* link to next file record */
    struct inv *hlinks;		/* ptr to records of hard links */
    dev_t dev;
    ino_t ino;
    char used;			/* set true when this file allocated
				       to a disk */
    };

struct file_struct		/* struct for moving files to a diff disk */
    {
    char *pathname;
    };

static struct file_struct move_files[] =
    {				/* files that get moved to disk 1 */
    "/usr/bin/ex",		/* for decrypt reasons */
    "/bin/e",
    "/lib/libc.a"		/* libc */
    };
int move_num = 3;		/* number of entries in "move_files" */
int cmdsize;			/* count of bytes in cmd line */

main (argc,argv)
    int argc;
    char *argv[];
{
    struct inv *invptr, *head, *endptr, *tmpptr; /* current, head, end, temp */
    char first;			/* flag to set head to first record */
    char s[PATHSIZE];		/* pathname to check w/ mnt device prepended */
    int mntpath=0;		/* flag for whether mount path was given */
    struct tm *time;		/* ptr to time struct returned by localtime */
    extern errno;

    /* variables for input lines from inventory */
    char inline[1024];
    long blocks;
    short uid, gid;
    u_short mode;
    char date[12], rev[12], type[10];
    char pathname[PATHSIZE];
    char links_to[PATHSIZE];

if (argc > 2)
    {
    fprintf(stderr,"usage is createmd [/mntpath] <inventory >%s\n",OUTSCRIPT);
    return(-1);
    }
if (argc == 2)
   mntpath = 1;

fprintf (stderr, "Createmd: Run as super-user.\n");
fprintf (stderr, "Createmd: Stderr is redirected to file `stderr' & contains diagnostics\n");
freopen ("stderr", "w", stderr);

/* While not end of input file:
 *   read lines and store files in linked list.
 */

first = 1;

while(gets(inline) != NULL)
    {
    sscanf(inline,"%d%hd%hd%ho%s%s%s%s%s", &blocks, &uid, &gid, &mode,
	date, rev, type, pathname, links_to);

    /* If current file is a directory then skip it, unless its a special one
     *   i.e. an empty one.
     */
    if (!strcmp (type, "d") &&
	(strcmp (pathname, "/tmp")) &&
	(strcmp (pathname, "/mnt")) &&
	(strcmp (pathname, "/usr/preserve")) &&
	(strcmp (pathname, "/usr/spool/lpd")) &&
	(strcmp (pathname, "/usr/spool/mail")) &&
	(strcmp (pathname, "/usr/spool/mqueue")) &&
	(strcmp (pathname, "/usr/spool/uucp")) &&
	(strcmp (pathname, "/usr/spool/rwho")) &&
	(strcmp (pathname, "/usr/spool/at")) &&
	(strcmp (pathname, "/usr/spool/at/past"))
	)
	continue;

    if (mntpath)
	{
	strcpy(s,argv[1]);
	strcat(s,pathname);
	}
    else
	strcpy(s,pathname);

     /* Store file in linked list. */

    invptr = (struct inv *) calloc (1, sizeof (struct inv));
    if (invptr == 0)
	{
	fprintf (stderr, "Createmd: calloc failed!\n");
	exit (1);
	}
    invptr->blocks = blocks;
    invptr->uid = uid;
    invptr->gid = gid;
    invptr->mode = mode;
    invptr->date = malloc (strlen (date)+1);
    strcpy (invptr->date, date);
    invptr->rev = malloc (strlen (rev)+1);
    strcpy (invptr->rev, rev);
    invptr->type = malloc (strlen (type)+1);
    strcpy (invptr->type, type);
    invptr->pathname = malloc (strlen (pathname)+1);
    strcpy (invptr->pathname, pathname);
    invptr->links_to = malloc (strlen (links_to)+1);
    strcpy (invptr->links_to, links_to);

    if (first)
	{
	first = 0;
	head = invptr;
	endptr = invptr;
	}
    else
	if (!strcmp(type,HARDLINK))
	    {
	    /* Path is a hard link so find file group that it goes with */

	    for (tmpptr = head; tmpptr != NULL; tmpptr = tmpptr->next)
		if (!strcmp (tmpptr->pathname, invptr->links_to))
		    {
		    /* Find end of hard links, & add this record onto end */

		    for (; tmpptr->hlinks != NULL; tmpptr = tmpptr->hlinks)
			continue;
		    tmpptr->hlinks = invptr;
		    break;
		    }
	    if (tmpptr == NULL)
		fprintf (stderr, "File not found to link %s to.\n", pathname);
	    }
	else
	    {
	    endptr->next = invptr;
	    endptr = invptr;
	    }
    }   /* end while getting input lines */

bld_script (head);
}  /* end main */


bld_script (head)
    struct inv *head;		/* ptr to head of linked list */

    /* Process the linked list and create the output script.
     */
    
    {
    int fnum;		/* current disk number */
    int blocks;		/* number of blocks in current disk so far */
    int blktmp;		/* number of blocks in current file */
    char done;		/* set true when current disk is full */
    struct inv *invptr;	/* place holder in processing list */
    struct inv *tmpptr;	/* look ahead pointer */

    fnum = 1;
    blocks = 0;
    done = false;

    /* Output script header */
    printf("\n#!/bin/csh\n");
    printf("# Script to create TAR format floppies.\n");
    printf("# invoke as %s\n\n",OUTSCRIPT);
    printf("\n");
    printf("# Parameters.\n");
    printf("# The following is the destination disk drive name.\n");
    printf("set DEST = \"%s\"\n", RL1);
    printf("# The following is the flag arguments to the tar commands.\n");
    printf("set TARG = \"%s\"\n", TARG);
    printf("# The following is the block size for tar to use.\n");
    printf("set TBLOCK = \"%s\"\n", TARSIZE);

    printf("\n");
    printf("echo \"\"\n");
    printf("echo \"Building the main distribution disks.\"\n");

    printf("echo \"\"\n");
    printf("echo \"Insert disk %d in RL drive 0.\"\n", fnum);
    printf("disk%d:\n", fnum);
    printf("echo -n \"Type a return when ready.   \"\n");
    printf("set xxx = $<\n\n");
    printf("echo \"\"\n");
    printf("tar c$TARG $TBLOCK $DEST");
    cmdsize = 22;

    /* Special case to move files that use decryption to disk #1
     * before any other files assigned to floppies.
     */

    blocks = disk1 (head);

    /* Loop until end of linked list is found */

    for (invptr = head; invptr != NULL; )
	{
	if (invptr->used)
	    {
	    invptr = invptr->next;
	    continue;
	    }
	blktmp = invptr->blocks + OVERHEAD;

	/* Determine if current file fits on current disk */
	if (blocks + blktmp > DISKSIZE)
	    {
	    /* It doesn't fit so find one that will fit */
	    tmpptr = invptr->next;
	    look_ahead (&tmpptr, blocks);
	    if (tmpptr != NULL)
		{
		blocks += tmpptr->blocks + OVERHEAD;
		tmpptr->used = true;
		output (tmpptr, &blocks, fnum);
		}
	    else
		{
		/* No more files that will fit on this disk.
		 * If the 1st one which was to large (at invptr) will not fit
		 * on a single disk, Yell and go on to the next.
		 */
		if (blktmp > DISKSIZE)
		    {
		    fprintf(stderr, "Warning file %s is %d blocks",
			    invptr->pathname, invptr->blocks);
		    fprintf(stderr, "   which will NOT fit on one disk.\n\n");
		    invptr = invptr->next;
		    }
		done = true;
		}
	    }
	else
	    {
	    /* The file fits: add its size, mark it used, output it, point to next */
	    blocks += blktmp;
	    invptr->used = true;
	    output (invptr, &blocks, fnum);
	    invptr = invptr->next;
	    }
	if (blocks >= DISKSIZE)
	    done = true;

	if (done)
	    {
	    fprintf(stderr, "%d Blocks on disk %d\n", blocks, fnum);

	    printf("\n\nif ($status != 0) then\n");
	    printf("    echo \"Error in making disk %d.\"\n", fnum);
	    printf("    echo -n \"Do you wish to retry? (y or n)   \"\n");
	    printf("    set ans = $<\n");
	    printf("    if (($ans == \"y\") || ($ans == \"yes\")) then\n");
	    printf("    	echo Retrying.\n");
	    printf("            echo \"\"\n");
	    printf("    	goto disk%d\n", fnum);
	    printf("    else\n");
	    printf("    	echo Aborting the build.\n");
	    printf("    	exit\n");
	    printf("    endif\n");
	    printf("endif\n");
	    printf("echo \"disk %d completed\"\n\n", fnum);
	    printf("echo \"\"\n");

	    done = false;
	    blocks = 0;
	    fnum++;

	    printf("echo \"\"\n");
	    printf("echo Insert disk %d in RL drive 0.\007\007\007\n", fnum);
	    printf("disk%d:\n", fnum);
	    printf("echo  -n \"Type a return when ready.   \"\n");
	    printf("set xxx = $<\n\n");
	    printf("echo \"\"\n");
	    printf("tar c$TARG $TBLOCK $DEST");
	    cmdsize = 22;
	    }
	}  /* end process for loop */

    fprintf(stderr, "%d Blocks on disk %d\n", blocks, fnum);
    printf("\necho \"disk %d completed\"\n\n", fnum);
    printf("echo \"\"\n");
    printf("echo \"All disks completed.\"\n\n");
    printf("# end\n");
    }  /* bld_script */

disk1 (ptr)
    struct inv *ptr;		/* ptr to head of list */

    /* Look thru entire linked list for files that are to be put on
     * disk 1, because the contain decryption stuff.
     *
     * Uses globals move_files and move_num.
     */

    {
    int blocks = 0;		/* block count on disk 1 */
    int i;

    for (; ptr != NULL; ptr = ptr->next)
	for (i = 0; i < move_num; i++)
	    if (!strcmp (ptr->pathname, move_files[i].pathname))
		{
		output (ptr, &blocks, 1);
		blocks += ptr->blocks + OVERHEAD;
		ptr->used = true;
		}
    return (blocks);
    }

output (ptr, blocks, fnum)
    struct inv *ptr;		/* ptr to file to output */
    int *blocks;		/* ptr to block count, add OVERHEAD for
				   each hard link. */
    int fnum;			/* current disk number being written */

    /* Output the pathname in this record.
     * Return block count; updated if any hard links are present.
     */

    {
    struct inv *tptr;		/* to move through possible hard links */
    for (tptr = ptr; tptr != NULL; tptr = tptr->hlinks)
	{
	if ((cmdsize + sizeof (tptr->pathname)) > BUFSIZE)
	    {
	    printf("\n\nif ($status != 0) then\n");
	    printf("    echo \"Error in making disk %d.\"\n", fnum);
	    printf("    echo -n \"Do you wish to retry? (y or n)   \"\n");
	    printf("    set ans = $<\n");
	    printf("    if (($ans == \"y\") || ($ans == \"yes\")) then\n");
	    printf("    	echo Retrying.\n");
	    printf("            echo \"\"\n");
	    printf("    	goto disk%d\n", fnum);
	    printf("    else\n");
	    printf("    	echo Aborting the build.\n");
	    printf("    	exit\n");
	    printf("    endif\n");
	    printf("endif\n");
	    printf("echo \"continuing with disk %d.\"\n\n", fnum);
	    printf("echo \"\"\n");
	    printf("tar r$TARG $TBLOCK $DEST");
	    cmdsize = 22;
	    fprintf (stderr, "Started next tar set on disk %d\n", fnum);
	    }
	else
	    cmdsize += sizeof (tptr->pathname);
	printf(" \\\n.%s", tptr->pathname);
	if (tptr != ptr)
	    *blocks += OVERHEAD;
	}
    }

look_ahead (tmpptr, blocks)
    struct inv **tmpptr;	/* passed address of ptr, return modified ptr */
    int blocks;			/* current block total on the disk */

    /* Looks ahead through the linked list of files to find one that will
     * fit into the present disk.  This is how we fill in the little
     * spaces at the end of each disk (i.e. solving the knapsack problem).
     */
    {
    int blktmp;

    for (; *tmpptr != NULL; *tmpptr = (*tmpptr)->next)
	{
	if ((*tmpptr)->used)
	    continue;
	blktmp = (*tmpptr)->blocks + OVERHEAD;
	if (blocks + blktmp <= DISKSIZE)
	    break;
	}
    }

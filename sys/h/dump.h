/*	"@(#)dump.h	1.2	(ULTRIX)	5/22/86"	*/

/*
 * dump.h
 */

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/* Number of structures to save at end of dump device */
#define NUM_TO_DUMP 2

struct dumpinfo {
	int	size_to_dump;		/* Num blocks to dump */
	int	blkoffs;		/* offset where dump device starts */
	int	partial_dump;		/* Partial dump switch */
	struct	partdump {
		int	num_blks;	/* Number of blocks to dump */
		int	start_addr;	/* Starting physical address */
	} pdump[NUM_TO_DUMP];
};


/* 	@(#)kmalloc.h	1.6	(ULTRIX)	9/11/86 	*/

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
 *
 *   Modification History:
 *
 *  29 Apr 86 -- depp
 *	Added KM_CONTG option for kmemall().  This option will attempt
 *	to allocate contiguous physical memory for an allocation (should
 *	only be used at startup).
 *
 *  18 Mar 86 -- depp
 *	Added memfree flag constants (MF_)
 *
 *  24 Feb 86 -- depp
 *	New File
 *
 */

/*
 *	kmalloc.h
 *
 * This file provides the constants and macros for the the kernel level
 * memory allocation routines (/sys/sys/vm_mem.c).
 * Two of these routines are derived from the malloc/free routines of 4.2BSD.
 * These two are km_alloc/km_free.
 */


#define NULL_PTR	((caddr_t) NULL)

/*
 * memfree "flg" parameter options
 */
#define MF_NODETACH	0		/* PFs are reclaimable */
#define MF_DETACH	1		/* PFs are probably not reclaimable*/
#define MF_UAREA	2		/* PFs are to be placed on "u" list*/

/*
 * Option flags for the "options" input parameters of kmemall and km_alloc
 */
#define KM_NULL		0x0000		/* no options */
#define	KM_CLRSG	0x0001		/* clear segment on allocation */
#define KM_SLEEP	0x0002		/* if allocation not possible, sleep*/
#define KM_CONTG	0x0004		/* contiguous physical memory */


#ifdef KM_DEBUG

/*
 *	The following two macros are used in kmemall/kmemfree to insure
 *	that deallocation requests are valid during integration.  In a
 *	production environment they are not used.
 */

#define KM_DEBUG_S	256

#define INS_KMEMDEBUG(a,s)	 {register ii; \
	for (ii = 0; ii < KM_DEBUG_S; ii++) \
		if (kmemdebug[ii].va == 0) \
			break; \
	if (ii == KM_DEBUG_S) { \
		printf("kmemall: table overflow\n"); \
		return(0); \
	} \
	kmemdebug[ii].va = a; \
	kmemdebug[ii].npg = s; \
}

#define CHK_KMEMDEBUG(a,s)	{register ii; \
	if (a == 0) { \
		printf("kmemfree: va (0x0) or npg (%d) not found\n",s); \
		return; \
	} \
	for (ii = 0; ii < KM_DEBUG_S; ii++) \
		if (kmemdebug[ii].va == a && kmemdebug[ii].npg == s) { \
			kmemdebug[ii].va = 0; \
			break; \
		} \
	if (ii == KM_DEBUG_S) { \
		printf("kmemfree: va (0x%x) or npg (%d) not found\n", a, s); \
		panic("kmemfree"); \
	} \
}

#endif KM_DEBUG



/*
 *	The following constants are for km_alloc and km_free
 *
 * MAX_POWER_2 controls the largest block which can be allocated.  This 
 * constant can not be less than (10) since that is the minimum page size
 * for large block allocation.  It can be higher, up to (30), effectively
 * allocating all memory requests via this mechanism.
 *
 * INT_POWER_2 controls where the algorithm begins to add one page cluster for
 * the header.
 *
 * MIN_POWER_2 controls the smallest block which can be allocated.
 *
 * ALIGN_MASK controls the alignment of the returned pointer.  The
 *	size of the "overhead" structure MUST be smaller than
 *	ALIGN_MASK + 1.
 *
 * NBUCKETS controls the number of hash buckets needed and is in turn
 * controlled by MAX_POWER_2 and MIN_POWER_2.
 *
 * MAX_SB_SIZE defines the break between small and large block size.
 *
 * INT_SB_SIZE defines the break point in small block size buffers where
 * the algorithm begins to treat headers some what differently.  Buffers 
 * with size  <= INT_SM_SIZE, simply add the header to the requested size.
 * If this means the the next power of 2 bucket is required, then so be it.
 * Buffers with size > INT_SM_SIZE will simply have an extra page cluster
 * added to account for the header.  This assumes that most allocations from 
 * here to MAX_SB_SIZE will fall on a power of 2 boundary.
 * 
 * MIN_ALLOC_SIZE is the minimum size (in bytes) of allocation via kmemall
 * for small block allocation.
 *
 * WARNING:  
 *   MIN_ALLOC_SIZE must be between 0 and INT_SB_SIZE (truncated down to next
 *   power of 2).
 */

#define MAX_POWER_2 	(16)
#define INT_POWER_2	(12)
#define MIN_POWER_2 	(6)
#define NBUCKETS   	((MAX_POWER_2 - MIN_POWER_2) + 1)	
#define ALIGN_MASK	(0xf)
#define INT_SB_SIZE	((1<<INT_POWER_2) - (ALIGN_MASK + 1))
#define MAX_SB_SIZE	((1<<MAX_POWER_2) - (ALIGN_MASK + 1))
#define MIN_ALLOC_SIZE	(CLBYTES * 2)



/*
 * The overhead on a block is at least 4 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The overhead block can expand upto the alignment
 * size (ALIGN_MASK + 1).
 */
union	overhead {
	union	overhead *ov_next;	/* when free */
	struct {
		short	ovu_magic;	/* magic number */
		short	ovu_index;	/* bucket # */
	} ovu;
};
#define	ov_magic	ovu.ovu_magic
#define	ov_index	ovu.ovu_index

#define	MAGIC		((short) 0x0318) /* magic # on allocated buffer */

#ifdef KM_STATS
struct km_stats {
	int tot_pfalloc;
	int tot_pffree;
	int tot_kmemall;
	int tot_kmemfree;
	int tot_km_alloc;
	int tot_km_free;
	int tot_morecore;
	int tot_sb_allocs[NBUCKETS];	/* Allocations of each SB size */
	int tot_sb_deallocs[NBUCKETS];	/* Deallocations of each SB size */
};
#endif KM_STATS

extern char *km_alloc();

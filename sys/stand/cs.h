/*
 *	@(#)cs.h	1.2 (ULTRIX) 6/28/85
 */
/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/* ------------------------------------------------------------------------
 * Modification History: /sys/stand/cs.h
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 17 Jun 85 -- jaw
 *	add support for VAX8200 console floppy.
 *
 * ------------------------------------------------------------------------
 */

#define RX_FILL	1
#define RX_EMPTY 0

#define UNITNUM 0

/* Control bits for control and status register rx5cs0 */
#define GO	01	/* go - drive initiates command - 01 */
#define MAINT	020	/* maintenance test of drive -	  10 */
#define RESTORE 040	/* restore drive -		  20 */
#define RX_READ	0100	/* read a sector -		  40 */
#define RX_WRITE	0160	/* write a sector -		  70 */
#define ERROR	0200	/* error indicator -		  80 */

/* register structure of the rx50 controller in the XT-100 */
struct	v8200rx50
	{
	u_short rx5id	  ,	/* identification register   -	00 */
		reserved  ,	/* reserved to hardware      -	02 */
		rx5cs0	  ,	/* csr0 command func. reg.   -	04 */
		rx5cs1	  ,	/* csr1 track register	     -	06 */
		rx5cs2	  ,	/* csr2 sector register      -	08 */
		rx5cs3	  ,	/* csr3 current sector reg.  -	0A */
		rx5cs4	  ,	/* csr4 incorrect track reg. -	0C */
		rx5extcmd ,	/* extend command register   -	0E */
		rx5edb	  ,	/* empty data buffer ; silo  -	10 */
		rx5ca	  ,	/* clear addr of data buffer -	12 */
		rx5go	  ,	/* go ; execute command      -	14 */
		rx5fdb	  ;	/* fill data buffer ;  silo  -	16 */
	} ;


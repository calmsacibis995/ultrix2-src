/*
 * sccsid  =  @(#)ciadapter.h	1.1	ULTRIX	10/3/86
 */

/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1986 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************
 *
 *
 *   Facility:	Systems Communication Architecture
 *		Computer Interconnect Port Driver
 *
 *   Abstract:	This module contains all the data structure definitions
 *		constants and macros required to link CI ports and their
 *		driver with lower level machine specific portions of the
 *		ULTRIX kernel.
 *
 *   Creator:	Todd M. Katz	Creation Date:	January 31, 1986
 *
 *   History:
 *
 */
/**/

/*
 * Constants.
 */
#define	CI_ADAPSIZE		 9	/* Size adapter I/O space ( pages )  */

/* Data Structures.
 *
 * Adapter Interface Block Definition.
 *
 * NOTE: This structure must NOT be changed without changing locore which has
 *	 dependencies on:
 *
 *	 1. The size of the structure.
 *	 2. The position of "isr" within the structure.
 *	 3. The position of "pccb" within the structure.
 */
typedef	struct _ciadap	{		/* CI Adapter Interface Block	     */
    void	   ( *isr )(); 		/* Interrupt Service Routine address */
    struct _pccb   *pccb;		/* PCCB pointer			     */
    void	   ( *mapped_isr )();	/* Mapped CI port ISR address	     */
    unsigned char  *phyaddr;		/* Adapter I/O space physical address*/
    unsigned char  *viraddr;		/* Adapter I/O space virtual address */
    struct pte	   *iopte;		/* Adapter I/O space PTE pointer     */
    unsigned short npages;		/* Size adapter I/O space ( pages )  */
					/* CIBCI/CIBCA only fields	     */
    char	   binum;		/* BI number  	   		     */
    char	   binode;		/* BI node number 		     */
    unsigned long  biic_int_ctrl;	/* BIIC user int control reg contents*/
    unsigned long  biic_int_dst;	/* BIIC int dst mask reg contents    */
    } CIADAP;

/*
 *		@(#)ovqp.h	1.1	(ULTRIX)	1/8/85
 */

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

#

/*	@(#)ovqp.h	7.1	2/5/81	*/

/*
**	This header file contains the external (global) declarations
**	of variables and structures as well as the manifest constants
**	particular to OVQP.
**
**	By convention global variable identifiers are spelled with 
**	an initial capital letter; manifest constants are in caps
**	completely.
*/



/*
**	Manifest constants
*/
   
  
# define	tTFLAG		'O'	/* trace flag */

# define	LBUFSIZE	850	/* buffer size for holding query list */
					/* and concat and ascii buffers */
# define	NSIMP		15	/*maximum no. of "simple clauses" 
					 * allowed in Qual list
					 * (see "strategy" portion) */
# ifndef	STACKSIZ
# define 	STACKSIZ	20
# endif
# define	MAXNODES	(2 * MAXDOM) + 50	/* max nodes in De.ov_qvect */

/* symbolic values for GETNXT parameter of fcn GET */
# define	CURTUP	0	/* get tuple specified by tid */
# define	NXTTUP	1	/* get next tuple after one specified by tid */
  

/* symbolic values for CHECKDUPS param of fcn INSERT */
# define	DUPS	0	/* allow a duplicate tuple to be inserted */
# define	NODUPS	1	/* check for and avoid inserting 
				 * a duplicate (if possible)*/


# define	TIDTYPE		INT
# define	TIDLEN		4

# define	CNTLEN 		4	/* counter for aggregate computations */
# define	CNTTYPE 	INT	/* counter type */

# define	OANYLEN		2	/* length for opANY */
# define	OANYTYPE	INT	/* type for opANY */

/* error codes for errors caused by user query ie. not syserrs */

# define	LISTFULL	4100	/* postfix query list full */
# define	BADCONV		4101	/* */
# define	BADUOPC		4102	/* Unary operator not allowed on char fields */
# define	BADMIX		4103	/* can't assign, compare or operate a numberic with a char */
# define	BADSUMC		4104	/* can't sum char domains (aggregate) */
# define	BADAVG		4105	/* can't avg char domains (aggregate) */
# define	STACKOVER	4106	/* shorterpreter stack overflow */
# define	CBUFULL		4107	/* not enough space for concat or ascii operation */
# define	BADCHAR		4108	/* arithmetic operation on two character fields */
# define	NUMERIC		4109	/* numeric field in a character operator */
# define	FLOATEXCEP	4110	/* floating poshort exception */
# define	CHARCONVERT	4111	/* bad ascii to numeric conversion */
# define	NODOVFLOW	4112	/* node vector overflow */
# define	COMPNOSP	4113	/* code table overflow */
# define	COMPNOREGS	4114	/* no more registers */

# define	BADSECINDX	4199	/* found a 6.0 sec index */







					 /* (i.e. either De.ov_srcdesc or Ov.ov_indesc) */








typedef char	i1type;
typedef short	i2type;
typedef long	i4type;
typedef float	f4type;
typedef double	f8type;
typedef char	c0type[];


typedef char	*stringp;

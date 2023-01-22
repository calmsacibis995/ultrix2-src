#ifndef	lint
static char *sccsid = "@(#)bbrstab.c	1.2	ULTRIX	10/3/86";
#endif	lint

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
 *	Facility:  Dynamic Bad Block Replacement
 *									
 *	Abstract:  This module contains the state table for 
 *		   the Dynamic BBR state machine.
 *		   
 *									
 *	Creator:	Mark Parenti	Creation Date:	May 6, 1986
 *
 *	Modification history:
 *
 * 	10-Jun-86 - map
 *		Initial version.
 */
/**/

#include "../h/types.h"
#include "../h/bbr.h" 

extern	int	bbr_inval_step(), step1_multi(), step3(), step4_start(),
		step4_cont(), step5_start(), step6_start(), step7_start(),
		step7_b(), step7_c(), step8(), step9(), step10(), step11(),
		step12_a(), step12_b(), step12_c(), step13_start(), 
		step14(), step15(), step16(), step17(), step18(), 
		multi_read_cont(), multi_write_cont(), search_cont(),
		bbr_linear();

BBR_STAB				/* BBR state transition table*/
	bbr_stab[ BBR_MAX_STEP + 1 ][ BBR_MAX_SUBSTEP + 1 ] = {
				  /* Step = 0 - Invalid			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { bbr_inval_step	},		/* 	     1			*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/* 	     3			*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/* 	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_1			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step1_multi	},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { step1_multi	},		/*	     READ_END		*/
    { bbr_inval_step	},		/*	     4			*/
    { step1_multi	},		/*	     READ_END_2		*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_3			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step3		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_4			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step4_start	},		/* 	     SUB_0		*/
    { step4_cont	},		/*	     SUB_1		*/
    { bbr_inval_step	},		/*	     3			*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_5			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step5_start	},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { step5_start	},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_6			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step6_start	},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { step6_start	},		/*	     READ_END		*/
    { step6_start	},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_7			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step7_start	},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { step7_start	},		/*	     READ_END		*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_7B			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step7_b		},		/* 	     SUB_0		*/
    { step7_b		},		/*	     SUB_1		*/
    { step7_b		},		/*	     READ_END		*/
    { step7_b		},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_7C			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step7_c		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     3			*/
    { step7_c		},		/*	     READ_END		*/
    { step7_c		},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_8			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step8		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     3			*/
    { step8		},		/*	     READ_END		*/
    { step8		},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_9			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step9		},		/* 	     SUB_0		*/
    { step9		},		/*	     SEARCH_END		*/
    { bbr_inval_step	},		/*	     READ_END		*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_10			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step10		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     READ_END		*/
    { step10		},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_11			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step11		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { step11		},		/*	     READ_END		*/
    { step11		},		/*	     WRITE_END		*/
    { step11		},		/*	     READ_END_2		*/
    { step11		},		/*	     WRITE_END_2	*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_12			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step12_a		},		/* 	     SUB_0		*/
    { step12_a		},		/*	     REPLACE_END	*/
    { step12_a		},		/*	     READ_END		*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_12_B		*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step12_b		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { step12_b		},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_12_C		*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step12_c		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { step12_c		},		/*	     READ_END		*/
    { step12_c		},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_13			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step13_start	},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     READ_END		*/
    { step13_start	},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_14			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step14		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_15			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step15		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { step15		},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { step15		},		/*	     WRITE_END_2	*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_16			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step16		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { step16		},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_17			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step17		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { step17		},		/*	     WRITE_END		*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = BBR_STEP_18			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { step18		},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { step18		},		/*	     STUNT_END		*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = MULTI_READ			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { multi_read_cont	},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = MULTI_WRITE			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { multi_write_cont	},		/* 	     SUB_0		*/
    { bbr_inval_step	},		/*	     2			*/
    { bbr_inval_step	},		/*	     3			*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_inval_step	},		/*	     7			*/
    { bbr_inval_step	},		/*	     8			*/
				  /* Step = RCT_SEARCH			*/
    { bbr_inval_step	},		/* Substep = 0			*/
    { bbr_inval_step	},		/* 	     1			*/
    { bbr_inval_step	},		/*	     2			*/
    { search_cont	},		/*	     READ_END		*/
    { bbr_inval_step	},		/*	     4			*/
    { bbr_inval_step	},		/*	     5			*/
    { bbr_inval_step	},		/*	     6			*/
    { bbr_linear	},		/*	     BBR_LIN_READ	*/
    { bbr_linear	},		/*	     BBR_LINEAR		*/

    };

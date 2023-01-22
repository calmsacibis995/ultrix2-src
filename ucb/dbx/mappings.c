/*#@(#)mappings.c	1.3	Ultrix	5/2/86*/

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

/************************************************************************
 *									*
 *			Modification History				*
 *									*
 *	002 - Merged in 4.3 changes.					*
 *	      (vjh, April 29, 1986)					*
 *									*
 *	001 - Removed 1001-function limit.  Function table is now	*
 *	      dynamically allocated.					*
 *	      (Victoria Holt, June 14, 1985)				*
 *									*
 ************************************************************************/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)mappings.c	5.1 (Berkeley) 5/31/85";
#endif not lint

static char rcsid[] = "$Header: mappings.c,v 1.4 84/12/26 10:40:25 linton Exp $";

/*
 * Source-to-object and vice versa mappings.
 */

#include "defs.h"
#include "mappings.h"
#include "symbols.h"
#include "source.h"
#include "object.h"
#include "machine.h"

#ifndef public
#include "machine.h"
#include "source.h"
#include "symbols.h"

typedef struct {
    Address addr;
    String filename;
    Lineno lineindex;		/* index to first linetab entry */
} Filetab;

typedef struct {
    Lineno line;
    Address addr;
} Linetab;

typedef struct {
    Symbol func;
    Address addr;
} Functab;

Filetab *filetab;
Linetab *linetab;
Functab *functab;

#define NOADDR ((Address) -1)	/* no address for line or procedure */

#endif

/*
 * Get the source file name associated with a given address.
 */

public String srcfilename(addr)
Address addr;
{
    register Address i, j, k;
    Address a;
    Filetab *ftp;
    String s;

    s = nil;
    if (nlhdr.nfiles != 0 and addr >= filetab[0].addr) {
	i = 0;
	j = nlhdr.nfiles - 1;
	while (i < j) {
	    k = (i + j) / 2;
	    ftp = &filetab[k];
	    a = ftp->addr;
	    if (a == addr) {
		s = ftp->filename;
		break;
	    } else if (addr > a) {
		i = k + 1;
	    } else {
		j = k - 1;
	    }
	}
	if (s == nil) {
	    if (addr >= filetab[i].addr) {
		s = filetab[i].filename;
	    } else {
		s = filetab[i-1].filename;
	    }
	}
    }
    return s;
}

/*
 * Find the line associated with the given address.
 * If the second parameter is true, then the address must match
 * a source line exactly.  Otherwise the nearest source line
 * below the given address is returned.
 *
 * Return the index of the line table entry or -1 if none suitable.
 */

private integer findline (addr, exact)
Address addr;
Boolean exact;
{
    register Address i, j, k;
    register Lineno r;
    register Address a;

    if (nlhdr.nlines == 0 or addr < linetab[0].addr) {
	r = -1;
    } else {
	i = 0;
	j = nlhdr.nlines - 1;
	if (addr == linetab[i].addr) {
	    r = i;
	} else if (addr == linetab[j].addr) {
	    r = j;
	} else if (addr > linetab[j].addr) {
	    r = exact ? -1 : j;
	} else {
	    do {
		k = (i + j) div 2;
		a = linetab[k].addr;
	    if (a == addr) break;
		if (addr > a) {
		    i = k + 1;
		} else {
		    j = k - 1;
		}
	    } while (i <= j);
	    if (a == addr) {
		r = k;
	    } else if (exact) {
		r = -1;
	    } else if (addr > linetab[i].addr) {
		r = i;
	    } else {
		r = i - 1;
	    }
	}
    }
    return r;
}

/*
 * Lookup the source line number nearest (from below) to an address.
 *
 * It is possible (unfortunately) that the compiler will generate
 * code before line number for a procedure.  Therefore we check
 * to see that the found line is in the same procedure as the given address.
 * If it isn't, then we walk forward until the first suitable line is found.
 */

public Lineno srcline(addr)
Address addr;
{
    integer i;
    Lineno r;
    Symbol f1, f2;

    i = findline(addr, false);
    if (i == -1) {
	r = 0;
    } else {
	r = linetab[i].line;
	if (linetab[i].addr != addr) {
	    f1 = whatblock(addr);
	    if (nosource(f1)) {
		r = 0;
	    } else {
		f2 = whatblock(linetab[i].addr + 1);
		if (f1 != f2) {
		    do {
			++i;
		    } while (linetab[i].addr < addr and i < nlhdr.nlines);
		    r = linetab[i].line;
		}
	    }
	}
    }
    return r;
}

/*
 * Look for a line exactly corresponding to the given address.
 */

public Lineno linelookup(addr)
Address addr;
{
    integer i;
    Lineno r;

    i = findline(addr, true);
    if (i == -1) {
	r = 0;
    } else {
	r = linetab[i].line;
    }
    return r;
}

/*
 * Lookup the object address of a given line from the named file.
 *
 * Potentially all files in the file table need to be checked
 * until the line is found since a particular file name may appear
 * more than once in the file table (caused by includes).
 */

public Address objaddr(line, name)
Lineno line;
String name;
{
    register Filetab *ftp;
    register Lineno i, j;
    Boolean foundfile;

    if (nlhdr.nlines == 0) {
	return NOADDR;
    }
    if (name == nil) {
	name = cursource;
    }
    foundfile = false;
    for (ftp = &filetab[0]; ftp < &filetab[nlhdr.nfiles]; ftp++) {
	if (streq(ftp->filename, name)) {
	    foundfile = true;
	    i = ftp->lineindex;
	    if (ftp == &filetab[nlhdr.nfiles-1]) {
		j = nlhdr.nlines;
	    } else {
		j = (ftp + 1)->lineindex;
	    }
	    while (i < j) {
		if (linetab[i].line == line) {
		    return linetab[i].addr;
		}
		i++;
	    }
	}
    }
    if (not foundfile) {
	error("source file \"%s\" not compiled with -g", name);
    }
    return NOADDR;
}

/*
 * Return the function that begins at the given address.
 */

public Symbol whatblock(addr)
Address addr;
{
    register int i, j, k;
    Address a;

    i = 0;
    j = nlhdr.nfuncs - 1;
    if (addr < functab[i].addr) {
	return program;
    } else if (addr == functab[i].addr) {
	return functab[i].func;
    } else if (addr >= functab[j].addr) {
	return functab[j].func;
    }
    while (i <= j) {
	k = (i + j) / 2;
	a = functab[k].addr;
	if (a == addr) {
	    return functab[k].func;
	} else if (addr > a) {
	    i = k+1;
	} else {
	    j = k-1;
	}
    }
    if (addr > functab[i].addr) {
	return functab[i].func;
    } else {
	return functab[i-1].func;
    }
    /* NOTREACHED */
}

/*
 * Order the function table.
 */

private int cmpfunc(f1, f2)
Functab *f1, *f2;
{
    register Address a1, a2;

    a1 = (*f1).addr;
    a2 = (*f2).addr;
    return ( (a1 < a2) ? -1 : ( (a1 == a2) ? 0 : 1 ) );
}

public ordfunctab()
{
    qsort(functab, nlhdr.nfuncs, sizeof(Functab), cmpfunc);
}

public dumpfunctab()
{
    int i;

    for (i = 0; i < nlhdr.nfuncs; i++) { 
	psym(functab[i].func);
    }
}

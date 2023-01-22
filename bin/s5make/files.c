#ifndef lint
static	char	*sccsid = "@(#)files.c	1.1	(ULTRIX)	3/20/86";
#endif lint

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

/* ------------------------------------------------------------------------
 * Modification History:
 *
 *  5 Jan 84 --jmcg
 *	Variable nopdir is used to keep track of the number of open
 *	directories.  Number is limited to MAXDIR.  This implementation
 *	follows that of 4.2BSD.
 *
 *  6 Nov 83 --jmcg
 *	Converted for BSD ASCII archives based on compile-time flag ASCARCH
 *
 *  6 Nov 83 --jmcg
 *	Converted to new BSD directory routines.
 *	Includes changing notion of "opendir" to "dirhdr".
 *
 *  5 Nov 83 --jmcg
 *	Derived from System III distribution tape.
 *	@(#)/usr/src/cmd/make/files.c	3.4
 * ------------------------------------------------------------------------
 */


#include "defs"
#include <sys/stat.h>
#include <pwd.h>
#include <ar.h>
#include <a.out.h>

/* fast macro for string compares - rr*/
#define eqstr(a,b,n) ((a)[0] == (b)[0] ? strncmp((a),(b),n) : 1)
/* UNIX DEPENDENT PROCEDURES */


char archmem[MAXNAMLEN];
char archname[BUFSIZ];		/* pathname of archive library */


TIMETYPE exists(pname)
NAMEBLOCK pname;
{
	register CHARSTAR s;
	struct stat buf;
	TIMETYPE lookarch();
	CHARSTAR filename;

	filename = pname->namep;

	if(any(filename, LPAREN))
		return(lookarch(filename));

	if(stat(filename,&buf) < 0) 
	{
		s = findfl(filename);
		if(s != (CHARSTAR )-1)
		{
			pname->alias = copys(s);
			if(stat(pname->alias, &buf) == 0)
				return(buf.st_mtime);
		}
		return(0);
	}
	else
		return(buf.st_mtime);
}


TIMETYPE prestime()
{
	TIMETYPE t;
	time(&t);
	return(t);
}



FSTATIC char nbuf[MAXNAMLEN+1];
FSTATIC CHARSTAR nbufend = &nbuf[MAXNAMLEN];



DEPBLOCK srchdir(pat, mkchain, nextdbl)
register CHARSTAR pat;		/* pattern to be matched in directory */
int mkchain;			/* nonzero if results to be remembered */
DEPBLOCK nextdbl;		/* final value for chain */
{
	DIR * dirf;
	int i, nread;
	CHARSTAR dirname, dirpref, endir, filepat, p;
	char temp[BUFSIZ];
	char fullname[BUFSIZ];
	CHARSTAR p1, p2;
	NAMEBLOCK q;
	DEPBLOCK thisdbl;
	DIRHDR od;
	PATTERN patp;
	int	cldir;

	struct direct *dptr;


	thisdbl = 0;

	if(mkchain == NO)
		for(patp=firstpat ; patp!=0 ; patp = patp->nextpattern)
			if(equal(pat,patp->patval))
				return(0);

	patp = ALLOC(pattern);
	patp->nextpattern = firstpat;
	firstpat = patp;
	patp->patval = copys(pat);

	endir = 0;

	for(p=pat; *p!=CNULL; ++p)
		if(*p==SLASH)
			endir = p;

	if(endir==0)
	{
		dirname = ".";
		dirpref = "";
		filepat = pat;
	}
	else
	{
		*endir = CNULL;
		dirpref = concat(pat, "/", temp);
		filepat = endir+1;
		dirname = temp;
	}

	dirf = NULL;
	cldir = NO;

	for(od=firstdir ; od!=0; od = od->nextdirhdr)
		if(equal(dirname, od->dirn))
		{
			dirf = od->dirfc;
			if( dirf != NULL)
				rewinddir( dirf);
			break;
		}

	if(dirf == NULL)
	{
		dirf = opendir(dirname);
	 	if(nopdir >= MAXODIR)
	 		cldir = YES;
	 	else	{
	 		++nopdir;
			od = ALLOC(dirhdr);
			od->nextdirhdr = firstdir;
			firstdir = od;
			od->dirfc = dirf;
			od->dirn = copys(dirname);
			}
	}

	if(dirf == NULL)
	{
		fprintf(stderr, "Directory %s: ", dirname);
		fatal("Cannot open");
	}

	else for (dptr = readdir(dirf); dptr != NULL; dptr = readdir(dirf))
		{
	 	p1 = dptr->d_name;
	 	p2 = nbuf;
	 	while( (p2<nbufend) && (*p2++ = *p1++)!='\0' )
	 		/* void */;
	 	if( amatch(nbuf,filepat) )
	 		{
	 		concat(dirpref,nbuf,fullname);
	 		if( (q=srchname(fullname)) ==0)
	 			q = makename(copys(fullname));
	 		if(mkchain)
				{
				thisdbl = ALLOC(depblock);
				thisdbl->nextdep = nextdbl;
				thisdbl->depname = q;
				nextdbl = thisdbl;
				}
			}
		}

	if(endir != 0)
		*endir = SLASH;

	if( cldir)
		{
		closedir(dirf);
		dirf = NULL;
		}

	return(thisdbl);
}

/* stolen from glob through find */

amatch(s, p)
register CHARSTAR s, p;
{
	register int cc, scc, k;
	register int lc, c;

	scc = *s;
	lc = 077777;
	switch (c = *p)
	{
	case LSQUAR:
		k = 0;
		while (cc = *++p)
		{
			if (cc == RSQUAR)
				if (k)
					return(amatch(++s, ++p));
				else
					return(0);
			else if (cc == MINUS)
				k |= ((lc <= scc) & (scc <= (cc=p[1])));
			if(scc==(lc=cc)) k++;
		}
		return(0);

	case QUESTN:
caseq:
		if(scc)
			return(amatch(++s, ++p));
		return(0);
	case STAR:
		p++;
		if(*p==0) return(1);
		while(*s) if(amatch(s++,p)) return(1);
		return(0);
	case 0:
		return(!scc);
	}
	if(c==scc)
		goto caseq;
	return(0);
}

/* put code inline above in amatch case STAR -- rr
umatch(s, p)
register CHARSTAR s, p;
{
	if(*p==0)
		return(1);
	while(*s)
		if(amatch(s++,p))
			return(1);
	return(0);
}
*/

#ifdef METERFILE
int meteron	= 0;	/* default: metering off */

meter(file)
CHARSTAR file;
{
	TIMETYPE tvec;
	CHARSTAR p, ctime();
	FILE * mout;
	struct passwd *pwd, *getpwuid();

	if(file==0 || meteron==0)
		return;

	pwd = getpwuid(getuid());

	time(&tvec);

	if( (mout=fopen(file,"a")) != NULL )
	{
		p = ctime(&tvec);
		p[16] = CNULL;
		fprintf(mout,"User %s, %s\n",pwd->pw_name,p+4);
		fclose(mout);
	}
}
#endif


/* look inside archives for notations a(b) and a((b))
	a(b)	is file member   b   in archive a
	a((b))	is entry point  _b  in object archive a
*/

  static long	arflen;
  static long	arfdate;
  static char	arfname[MAXNAMLEN+1];

static struct ar_hdr arhead;
FILE *arfd;
long int arpos, arlen;

static struct exec objhead;

static struct nlist objentry;


TIMETYPE lookarch(filename)
register CHARSTAR filename;
{
	register int i;
	CHARSTAR p, q, send;
	char s[MAXNAMLEN+1];
	int nc, nsym, objarch;

	for(p = filename; *p!= LPAREN ; ++p);
	q = p++;

	if(*p == LPAREN)
	{
		objarch = YES;
		nc = MAXNAMLEN;
		++p;
	}
	else
	{
		objarch = NO;
		nc = MAXNAMLEN;
		for(i = 0; i < MAXNAMLEN; i++)
		{
			if(p[i] == RPAREN)
			{
				i--;
				break;
			}
			archmem[i] = p[i];
		}
		archmem[++i] = 0;
	}
	*q = CNULL;
	copstr(archname, filename);
	i = openarch(filename);
	*q = LPAREN;
	if(i == -1)
		return(0);
	send = s + nc;

	for( q = s ; q<send && *p!=CNULL && *p!=RPAREN ; *q++ = *p++ );

	while(q < send)
		*q++ = CNULL;
	while(getarch())
	{
		if(objarch)
		{
			getobj();
			nsym = objhead.a_syms / sizeof(objentry);
			for(i = 0; i<nsym ; ++i)
			{
				fread(&objentry, sizeof(objentry),1,arfd);
				if( (objentry.n_type & N_EXT)
				   && ((objentry.n_type & ~N_EXT) || objentry.n_value)
				   && eqstr(objentry.n_un.n_name,s,nc))
				{
					clarch();
					return(arfdate);
				}
			}
		}

		else if( eqstr(arhead.ar_name, s, nc))
			{
				clarch();
				return(arfdate);
			}
	}

	clarch();
	return( 0L);
}


clarch()
{
	fclose( arfd );
}


#ifdef ASCARCH
char magic[SARMAG];
#endif ASCARCH
openarch(f)
register CHARSTAR f;
{
	int word = 0;
	struct stat buf;

	if(stat(f, &buf) == -1)
		return(-1);
	arlen = buf.st_size;

	arfd = fopen(f, "r");
	if(arfd == NULL)
		fatal1("cannot open %s", f);
	fread(&word, sizeof(word), 1, arfd);
#ifdef ASCARCH
	fseek(arfd, 0L, 0);
	fread(magic, SARMAG, 1, arfd);
	arpos = SARMAG;
	if( ! eqstr(magic, ARMAG, SARMAG) )
#else	ASCARCH
	arpos = sizeof(word);
	if(word != ARMAG)
#endif	ASCARCH
		fatal1("%s is not an archive", f);
  /*
   *	trick getarch() into jumping to the first archive member.
   */
#ifdef ASCARCH
	arpos = SARMAG;
	arflen = 0;
#else ASCARCH
	arpos = sizeof(word);
  	arhead.ar_size = -(int)sizeof(arhead);
#endif ASCARCH
	return(0);
}



getarch()
{
	long atol();

#ifdef ASCARCH
	arpos += (arflen + 1) & ~1L; /* round archived file length up to even */
#else
  	arpos += sizeof(arhead);
  	arpos += (arhead.ar_size + 1 ) & ~1L;
#endif
	if(arpos >= arlen)
		return(0);
	fseek(arfd, arpos, 0);
	fread(&arhead, sizeof(arhead), 1, arfd);
	arpos += sizeof(arhead);
#ifdef ASCARCH
	arflen = atol(arhead.ar_size);
	arfdate = atol(arhead.ar_date);
#else
	arflen = arhead.ar_size;
	arfdate = arhead.ar_date;
#endif
	strncpy(arfname, arhead.ar_name, sizeof(arhead.ar_name));
	return(1);
}


getobj()
{
	long int skip;

	fread(&objhead, sizeof(objhead), 1, arfd);
#ifdef ASCARCH
	if (N_BADMAG(objhead))
#else
	if( objhead.a_magic != A_MAGIC1 &&
	    objhead.a_magic != A_MAGIC2 &&
	    objhead.a_magic != A_MAGIC3 )
#endif
			fatal1("%s is not an object module", arhead.ar_name);
	skip = objhead.a_text + objhead.a_data;
#ifdef vax
	skip += objhead.a_trsize + objhead.a_drsize;
#else
	if(! objhead.a_flag )
		skip *= 2;
#endif
	fseek(arfd, skip, 1);
}


/* use a macro that should be faster  -- rr
eqstr(a,b,n)
register CHARSTAR a, b;
register int n;
{
	register int i;
	for(i = 0 ; i < n ; ++i)
		if(*a++ != *b++)
			return(NO);
	return(YES);
}
*/

/*
 *	Used when unlinking files. If file cannot be stat'ed or it is
 *	a directory, then do not remove it.
 */
isdir(p)
char *p;
{
	struct stat statbuf;
	if (stat(p,&statbuf) == -1)
		return(1);		/* no stat, no remove */
	if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
		return(1);		/* directory */
	return(0);
}

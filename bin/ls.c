#ifndef lint
static	char	*sccsid = "@(#)ls.c	1.6	(ULTRIX)	3/3/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
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
*
*		Modification History
*
*	David Metsky, 12/23/86
* 001	Changed the old "not found" error message into a call to perror.
*	This answers spr bar smu-819.
*
*************************************************************************/

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include <sgtty.h>

#define	kbytes(size)	(((size) + 1023) / 1024)

struct afile {
	char	ftype;		/* file type, e.g. 'd', 'c', 'f' */
	ino_t	fnum;		/* inode number of file */
	short	fflags;		/* mode&~S_IFMT, perhaps ISARG */
	short	fnl;		/* number of links */
	short	fuid;		/* owner id */
	short	fgid;		/* group id */
	long	fsize;		/* file size */
	long	fblks;		/* number of blocks used */
	time_t	fmtime;		/* time (modify or access or create) */
	char	*fname;		/* file name */
	char	*flinkto;	/* symbolic link value */
};

#define ISARG	0x8000		/* extra ``mode'' */

struct subdirs {
	char	*sd_name;
	struct	subdirs *sd_next;
} *subdirs;

int	aflg, dflg, gflg, lflg, sflg, tflg, uflg, iflg, fflg, cflg, rflg = 1;
int	qflg, Aflg, Cflg, Fflg, Lflg, Rflg;
int	usetabs;
time_t	now, sixmonthsago, onehourfromnow;
char	*dotp = ".";

long	time();
void	exit(), perror();
struct	afile *gstat();
int	fcmp();
char	*cat(), *savestr(), *strncpy();
char	*fmtentry();
char	*getname(), *getgroup();
char	*ctime(), *malloc(), *calloc(), *realloc();
char	*sprintf(), *strcpy(), *strcat();

main(argc, argv)
	int argc;
	char *argv[];
{
	register int i;
	register struct afile *fp0, *fplast;
	register struct afile *fp;
	struct sgttyb sgbuf;

	argc--, argv++;
	if (getuid() == 0)
		Aflg++;
	(void) time((long *)&now);
	sixmonthsago = now - 6L*30L*24L*60L*60L;
	onehourfromnow = now + 60L*60L;
	now += 60;
	if (isatty(1)) {
		qflg = Cflg = 1;
		(void) gtty(1, &sgbuf);
		if ((sgbuf.sg_flags & XTABS) == 0)
			usetabs = 1;
	} else
		usetabs = 1;
	while (argc > 0 && **argv == '-') {
		(*argv)++;
		while (**argv) switch (*(*argv)++) {

		case 'C':
			Cflg = 1; break;
		case 'q':
			qflg = 1; break;
		case '1':
			Cflg = 0; break;
		case 'a':
			aflg++; break;
		case 'A':
			Aflg++; break;
		case 'c':
			cflg++; break;
		case 's':
			sflg++; break;
		case 'd':
			dflg++; break;
		case 'g':
			gflg++; break;
		case 'l':
			lflg++; break;
		case 'r':
			rflg = -1; break;
		case 't':
			tflg++; break;
		case 'u':
			uflg++; break;
		case 'i':
			iflg++; break;
		case 'f':
			fflg++; break;
		case 'L':
			Lflg++; break;
		case 'F':
		        Fflg++; break;
		case 'R':
		        Rflg++; break;
		}
		argc--, argv++;
	}
	if (fflg) { 
		aflg++; lflg = 0; sflg = 0; tflg = 0;
	}
	if (lflg)
		Cflg = 0;
	if (argc == 0) {
		argc++;
		argv = &dotp;
	}
	fp = (struct afile *) calloc((unsigned)argc, sizeof (struct afile));
	if (fp == 0) {
		(void) fprintf(stderr, "ls: out of memory\n");
		exit(1);
	}
	fp0 = fp;
	for (i = 0; i < argc; i++) {
		if (gstat(fp, *argv, 1, (int *)0)) {
			fp->fname = *argv;
			fp->fflags |= ISARG;
			fp++;
		}
		argv++;
	}
	fplast = fp;
	qsort((char *)fp0, fplast - fp0, sizeof (struct afile), fcmp);
	if (dflg) {
		formatf(fp0, fplast);
		exit(0);
	}
	if (fflg)
		fp = fp0;
	else {
		for (fp = fp0; fp < fplast && fp->ftype != 'd'; fp++)
			continue;
		formatf(fp0, fp);
	}
	if (fp < fplast) {
		if (fp > fp0)
			(void) printf("\n");
		for (;;) {
			formatd(fp->fname, argc > 1);
			while (subdirs) {
				struct subdirs *t;

				t = subdirs; subdirs = t->sd_next;
				(void) printf("\n");
				formatd(t->sd_name, 1);
				cfree(t->sd_name);
				cfree((char *)t);
			}
			if (++fp == fplast)
				break;
			(void) printf("\n");
		}
	}
	exit(0);
}

formatd(name, title)
	char *name;
	int title;
{
	register struct afile *fp;
	register struct subdirs *dp;
	register int nkb;
	struct afile *dfp0, *dfplast;

	nkb = getdir(name, &dfp0, &dfplast);
	if (dfp0 == 0)
		return;
	if (fflg == 0)
		qsort((char *)dfp0, dfplast - dfp0, sizeof(struct afile), fcmp);
	if (title)
		(void) printf("%s:\n", name);
	if (lflg || sflg)
		(void) printf("total %ld\n", nkb);
	formatf(dfp0, dfplast);
	if (Rflg)
		for (fp = dfplast-1; fp >= dfp0; fp--) {
			if (fp->ftype != 'd' ||
			    !strcmp(fp->fname, ".") ||
			    !strcmp(fp->fname, ".."))
				continue;
			dp = (struct subdirs *) malloc(sizeof(struct subdirs));
			dp->sd_name = savestr(cat(name, fp->fname));
			dp->sd_next = subdirs; subdirs = dp;
		}
	for (fp = dfp0; fp < dfplast; fp++) {
		if ((fp->fflags&ISARG) == 0 && fp->fname)
			cfree(fp->fname);
		if (fp->flinkto)
			cfree(fp->flinkto);
	}
	cfree((char *)dfp0);
}

getdir(dir, pfp0, pfplast)
	char *dir;
	struct afile **pfp0, **pfplast;
{
	register struct afile *fp;
	register DIR *dirp;
	register struct direct *dp;
	register int nent = 20;
	int nb;

	dirp = opendir(dir);
	if (dirp == NULL) {
		*pfp0 = *pfplast = NULL;
		(void) printf("%s unreadable\n", dir);	/* not stderr! */
		return (0);
	}
	fp = *pfp0 = (struct afile *)
			calloc((unsigned)nent, sizeof (struct afile));
	*pfplast = *pfp0 + nent;
	nb = 0;
	while (dp = readdir(dirp)) {
		if (dp->d_ino == 0)
			continue;
		if (aflg == 0 && dp->d_name[0]=='.' &&
		    (Aflg == 0 || dp->d_name[1]==0 ||
		     dp->d_name[1]=='.' && dp->d_name[2]==0))
			continue;
		if (gstat(fp, cat(dir, dp->d_name), Fflg+Rflg, &nb) == 0)
			continue;
		fp->fnum = dp->d_ino;
		fp->fname = savestr(dp->d_name);
		fp++;
		if (fp == *pfplast) {
			*pfp0 = (struct afile *) realloc((char *)*pfp0,
			    (unsigned) (2 * nent * sizeof (struct afile)));
			if (*pfp0 == 0) {
				(void) fprintf(stderr, "ls: out of memory\n");
				exit(1);
			}
			fp = *pfp0 + nent;
			*pfplast = fp + nent;
			nent *= 2;
		}
	}
	closedir(dirp);
	*pfplast = fp;
	return (kbytes(dbtob(nb)));
}

int	stat(), lstat();

struct afile *
gstat(fp, file, statarg, pnb)
	register struct afile *fp;
	register char *file;
	int statarg;
	int *pnb;	/* (int *)0 if file is ISARG */
{
	int (*statf)() = Lflg ? stat : lstat;
	char buf[BUFSIZ];
	struct stat stb, stb1;
	register int cc;
	static struct afile azerofile;

	*fp = azerofile;
	fp->fflags = 0;
	fp->fnum = 0;
	fp->ftype = '-';
	if (statarg || sflg || lflg || tflg) {
		if ((*statf)(file, &stb) < 0) {
			if (statf == lstat || lstat(file, &stb) < 0) {
			     /*	fprintf(stderr, "%s not found\n", file); */
				(void) fprintf(stderr,"ls: \"%s",file);/* 001 */
				perror("\"");
				return (NULL);
			}
		}
		fp->fblks = stb.st_blocks;
		fp->fsize = stb.st_size;
		switch (stb.st_mode & S_IFMT) {

		case S_IFDIR:
			fp->ftype = 'd'; break;
		case S_IFPORT:
			fp->ftype = 'p'; break;
		case S_IFBLK:
			fp->ftype = 'b'; fp->fsize = stb.st_rdev; break;
		case S_IFCHR:
			fp->ftype = 'c'; fp->fsize = stb.st_rdev; break;
		case S_IFSOCK:
			fp->ftype = 's'; fp->fsize = 0; break;
		case S_IFLNK:
			fp->ftype = 'l';
			if (lflg) {
				cc = readlink(file, buf, BUFSIZ);
				if (cc >= 0) {
					buf[cc] = 0;
					fp->flinkto = savestr(buf);
				}
				break;
			}
			if (stat(file, &stb1) < 0)
				break;
			if ((stb1.st_mode & S_IFMT) == S_IFDIR) {
				stb = stb1;
				fp->ftype = 'd';
				fp->fsize = stb.st_size;
				fp->fblks = stb.st_blocks;
			}
			break;
		}
		fp->fnum = stb.st_ino;
		fp->fflags = stb.st_mode & ~S_IFMT;
		fp->fnl = stb.st_nlink;
		fp->fuid = stb.st_uid;
		fp->fgid = stb.st_gid;
		if (uflg)
			fp->fmtime = stb.st_atime;
		else if (cflg)
			fp->fmtime = stb.st_ctime;
		else
			fp->fmtime = stb.st_mtime;
		if (pnb)
			*pnb += stb.st_blocks;
	}
	return (fp);
}

formatf(fp0, fplast)
	struct afile *fp0, *fplast;
{
	register struct afile *fp;
	register int i, j, len, width = 0;
	register int lines;
	int columns;
	int nentry = fplast - fp0;
	char *cp;

	if (fp0 == fplast)
		return;
	if (lflg || Cflg == 0)
		columns = 1;
	else {
		for (fp = fp0; fp < fplast; fp++) {
			len = strlen(fmtentry(fp));
			if (len > width)
				width = len;
		}
		if (usetabs)
			width = (width + 8) &~ 7;
		else
			width += 2;
		columns = 80 / width;
		if (columns == 0)
			columns = 1;
	}
	lines = (nentry + columns - 1) / columns;
	for (i = 0; i < lines; i++) {
		for (j = 0; j < columns; j++) {
			fp = fp0 + j * lines + i;
			cp = fmtentry(fp);
			(void) printf("%s", cp);
			if (fp + lines >= fplast) {
				(void) printf("\n");
				break;
			}
			len = strlen(cp);
			while (len < width)
				if (usetabs) {
					len = (len + 8) &~ 7;
					(void) printf("\t");
				} else {
					len++;
					(void) printf(" ");
				}
		}
	}
}

fcmp(f1, f2)
	register struct afile *f1, *f2;
{
	register char *a,*b;

	if (dflg == 0 && fflg == 0) {
		if ((f1->fflags&ISARG) && f1->ftype == 'd') {
			if ((f2->fflags&ISARG) == 0 || f2->ftype != 'd')
				return (1);
		} else {
			if ((f2->fflags&ISARG) && f2->ftype == 'd')
				return (-1);
		}
	}
	if (tflg) {
		if (f2->fmtime == f1->fmtime)
			return (0);
		if (f2->fmtime > f1->fmtime)
			return (rflg);
		return (-rflg);
	}
#ifdef OLD
	return (rflg * strcmp(f1->fname, f2->fname));
#else NEW
	a = f1->fname; b = f2->fname;
#define STRCMP(a,b) ((a)[0] == (b)[0] ? strcmp((a),(b)):(a)[0] < (b)[0] ? -1:1)
	return (rflg * STRCMP(a,b));
#undef STRCMP
#endif OLD
}

char *
cat(dir, file)
	register char *dir, *file;
{
	static char dfile[BUFSIZ];
	register char *dptr = dfile;
	register len_dir = strlen(dir);

	if (len_dir+1+strlen(file)+1 > BUFSIZ) {
		(void) fprintf(stderr, "ls: filename too long\n");
		exit(1);
	}
#ifdef OLD
	if (!strcmp(dir, "") || !strcmp(dir, "."))
#else NEW
	if (*dir == NULL || (*dir == '.' && *(dir+1) == NULL))
#endif OLD
	{
		(void) strcpy(dptr, file);
		return (dptr);
	}
	(void) strcpy(dptr, dir);
	if (dir[len_dir - 1] != '/' && *file != '/')
		(void) strcat(dptr, "/");
	(void) strcat(dptr, file);
	return (dptr);
}

char *
savestr(str)
	register char *str;
{
	register char *cp = malloc((unsigned) (strlen(str) + 1));

	if (cp == NULL) {
		(void) fprintf(stderr, "ls: out of memory\n");
		exit(1);
	}
	(void) strcpy(cp, str);
	return (cp);
}

char	*fmtinum(), *fmtsize(), *fmtlstuff(), *fmtmode();

char *
fmtentry(fp)
	register struct afile *fp;
{
	static char fmtres[BUFSIZ];
	register char *cp, *dp;

	(void) sprintf(fmtres, "%s%s%s",
	    iflg ? fmtinum(fp) : "",
	    sflg ? fmtsize(fp) : "",
	    lflg ? fmtlstuff(fp) : "");
	dp = &fmtres[strlen(fmtres)];
	for (cp = fp->fname; *cp; cp++)
		if (qflg && (*cp < ' ' || *cp >= 0177))
			*dp++ = '?';
		else
			*dp++ = *cp;
	if (Fflg) {
		if (fp->ftype == 'd')
			*dp++ = '/';
		else if (fp->ftype == 'l')
			*dp++ = '@';
		else if (fp->ftype == 's')
			*dp++ = '=';
		else if (fp->fflags & 0111)
			*dp++ = '*';
	}
	if (lflg && fp->flinkto) {
		(void) strcpy(dp, " -> "); dp += 4;
		for (cp = fp->flinkto; *cp; cp++)
			if (qflg && (*cp < ' ' || *cp >= 0177))
				*dp++ = '?';
			else
				*dp++ = *cp;
	}
	*dp++ = 0;
	return (fmtres);
}

char *
fmtinum(p)
	register struct afile *p;
{
	static char inumbuf[8];

	(void) sprintf(inumbuf, "%6d ", p->fnum);
	return (inumbuf);
}

char *
fmtsize(p)
	register struct afile *p;
{
	static char sizebuf[32];

	(void) sprintf(sizebuf, "%4ld ", kbytes(dbtob(p->fblks)));
	return (sizebuf);
}

char *
fmtlstuff(p)
	register struct afile *p;
{
	static char lstuffbuf[256];
	char gname[32], uname[32], fsize[32], ftime[32];
	register char *lp = lstuffbuf;
	register char *cp;

	/* type mode uname gname fsize ftime */
/* get uname */
	{ cp = getname(p->fuid);
	  if (cp)
		(void) sprintf(uname, "%-9.9s", cp);
	  else
		(void) sprintf(uname, "%-9d", p->fuid);
	}
/* get gname */
	if (gflg) {
	  cp = getgroup(p->fgid);
	  if (cp)
		(void) sprintf(gname, "%-9.9s", cp);
	  else
		(void) sprintf(gname, "%-9d", p->fgid);
	}
/* get fsize */
	if (p->ftype == 'b' || p->ftype == 'c')
		(void) sprintf(fsize, "%3d,%4d",
		    major(p->fsize), minor(p->fsize));
	else if (p->ftype == 's')
		(void) sprintf(fsize, "%8ld", 0);
	else
		(void) sprintf(fsize, "%8ld", p->fsize);
/* get ftime */
	{ cp = ctime((long *)&p->fmtime);
	  if ((p->fmtime < sixmonthsago) || (p->fmtime > onehourfromnow))
		(void) sprintf(ftime, " %-7.7s %-4.4s ", cp+4, cp+20);
	  else
		(void) sprintf(ftime, " %-12.12s ", cp+4);
	}
/* splat */
	*lp++ = p->ftype;
	lp = fmtmode(lp, p->fflags);
	(void) sprintf(lp, "%3d %s%s%s%s",
	    p->fnl, uname, gflg ? gname : "", fsize, ftime);
	return (lstuffbuf);
}

int	m1[] = { 1, S_IREAD>>0, 'r', '-' };
int	m2[] = { 1, S_IWRITE>>0, 'w', '-' };
int	m3[] = { 2, S_ISUID, 's', S_IEXEC>>0, 'x', '-' };
int	m4[] = { 1, S_IREAD>>3, 'r', '-' };
int	m5[] = { 1, S_IWRITE>>3, 'w', '-' };
int	m6[] = { 2, S_ISGID, 's', S_IEXEC>>3, 'x', '-' };
int	m7[] = { 1, S_IREAD>>6, 'r', '-' };
int	m8[] = { 1, S_IWRITE>>6, 'w', '-' };
int	m9[] = { 2, S_ISVTX, 't', S_IEXEC>>6, 'x', '-' };

int	*m[] = { m1, m2, m3, m4, m5, m6, m7, m8, m9};

char *
fmtmode(lp, flags)
	register char *lp;
	register int flags;
{
	register int **mp;
	register int *pairp;
	register int n;

	for (mp = &m[0]; mp < &m[sizeof(m)/sizeof(m[0])]; ) {
		pairp = *mp++;
		n = *pairp++;
		while (--n >= 0 && (flags & *pairp++) == 0) pairp++;
		*lp++ = *pairp;
	}
	return (lp);
}

/* rest should be done with nameserver or database */

#include <pwd.h>
#include <grp.h>
#include <utmp.h>

struct	utmp *utmp;
#define	NMAX	(sizeof (utmp->ut_name))
#define SCPYN(a, b)	(void) strncpy(a, b, NMAX)

#undef MAXUID

#define MAXUID	5000		/* be nice this is bss */
#define MINUID  -2		/* for nfs */
#define MAXGID	300

char	namebuf[MAXUID - MINUID][NMAX+1];
char	(*names)[NMAX+1] = namebuf - MINUID;
char	outrangename[NMAX+1];
int	outrangeuid = -1;
char	groups[MAXGID][NMAX+1];
char	outrangegroup[NMAX+1];
int	outrangegid = -1;

char *
getname(uid)
{
	register struct passwd *pw;

	if (uid >= MINUID && uid < MAXUID && names[uid][0])
		return (&names[uid][0]);
	if (uid >= MINUID && uid == outrangeuid)
		return (outrangename);
	if (uid < MINUID)
		return (NULL);

	pw = getpwuid(uid);
	if (pw == NULL)
		return (NULL);

	if (uid >= MINUID && uid < MAXUID) {
		SCPYN(names[uid], pw->pw_name);
		return (&names[uid][0]);
	}

	outrangeuid = uid;
	SCPYN(outrangename, pw->pw_name);
	return (outrangename);
}

char *
getgroup(gid)
{
	register struct group *gr;

	if (gid >= 0 && gid < MAXGID && groups[gid][0])
		return (&groups[gid][0]);
	if (gid >= 0 && gid == outrangegid)
		return (outrangegroup);
	if (gid < 0)
		return (NULL);
		
	gr = getgrgid(gid);
	if (gr == NULL)
		return (NULL);
	if (gid >= 0 && gid < MAXGID) {
		SCPYN(groups[gr->gr_gid], gr->gr_name);
		return (&groups[gid][0]);
	}
		
	outrangegid = gr->gr_gid;
	SCPYN(outrangegroup, gr->gr_name);
	return (outrangegroup);
}

#ifndef lint
static char *sccsid = "@(#)getpwent.c	1.7	ULTRIX	2/13/87";
#endif lint

/****************************************************************
 *								*
 *  Licensed to Digital Equipment Corporation, Maynard, MA	*
 *		Copyright 1985 Sun Microsystems, Inc.		*
 *			All rights reserved.			*
 *								*
 ****************************************************************/

#include <stdio.h>
#include <pwd.h>
#include <ctype.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <rpcsvc/ypclnt.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypv1_prot.h>

#define MAXINT 0x7fffffff;
#define TIMEOUT 30
#define INTER_TRY 10

#define	PWSKIP(s)	while(*(s) && *(s) != ':' && *(s) != '\n') \
				++(s); \
			if (*(s)) *(s)++ = 0;
#define ATOI(p,pout) {\
		register char *p_atoi = p;	\
		register int n_atoi;		\
		register int f_atoi;		\
		n_atoi = 0;			\
		f_atoi = 0;			\
		for(;;p_atoi++) {		\
			switch(*p_atoi) {	\
			case ' ':		\
			case '\t':		\
				continue;	\
			case '-':		\
				f_atoi++;	\
			case '+':		\
				p_atoi++;	\
			}			\
			break;			\
		}				\
		while(*p_atoi >= '0' && *p_atoi <= '9')			\
			n_atoi = n_atoi*10 + *p_atoi++ - '0';		\
		(pout) = (f_atoi ? -n_atoi : n_atoi);			\
	}

extern char *strcpy();
extern char *malloc();

static char domain[256];
char *ldomain = domain;

static struct passwd NULLPW = {NULL, NULL, 0, 0, 0, NULL, NULL, NULL, NULL};
static char PASSWD[]	= "/etc/passwd"; 
static char EMPTY[] = "";
struct in_addr host_addr;
static FILE *pwf = NULL;	/* pointer into /etc/passwd */
static char *yp;		/* pointer into yellow pages */
static int yplen;
static char *oldyp = NULL;	
static int oldyplen;
static struct passwd *interpret();
static struct passwd *interpretwithsave();
static struct passwd *save();
static struct passwd *getnamefromyellow();
static struct passwd *getuidfromyellow();

static struct list {
    char *name;
    struct list *nxt
} *minuslist;			/* list of - items */


struct passwd *
getpwnam(name)
	char *name;
{
	struct passwd *pw;
	char line[BUFSIZ+1];
	register char *ptr;
	register int len = 0;

	(void) setpwent();
	if (!pwf)
		return NULL;
	while (fgets(line, BUFSIZ, pwf) != NULL) {
		ptr = line;
		len = 0;
		while(*ptr++) len++;
		pw = interpret(line, len);
		if (matchname(line, &pw, name)) {
			(void) endpwent();
			return pw;
		}
	}
	(void) endpwent();
	return NULL;
}

struct passwd *
getpwuid(uid)
	register uid;
{
	struct passwd *pw;
	char line[BUFSIZ+1];
	register char *ptr;
	register int len = 0;

	(void) setpwent();
	if (!pwf)
		return NULL;
	while (fgets(line, BUFSIZ, pwf) != NULL) {
		ptr = line;
		len = 0;
		while(*ptr++) len++;
		pw = interpret(line, len);
		if (matchuid(line, &pw, uid)) {
			(void) endpwent();
			return pw;
		}
	}
	(void) endpwent();
	return NULL;
}




setpwent()
{
	if (getdomainname(domain, sizeof(domain)) < 0) {
		(void) fprintf(stderr, 
		    "setpwent: getdomainname system call missing\n");
		exit(1);
	}
	if (pwf == NULL)
		pwf = fopen(PASSWD, "r");
	else
		rewind(pwf);
	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
}



endpwent()
{
	if (pwf != NULL) {
		(void) fclose(pwf);
		pwf = NULL;
	}
	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
	endnetgrent();
}


struct passwd *
getpwent()
{
	char line1[BUFSIZ+1];
	static struct passwd *savepw;
	struct passwd *pw;
	char *user = NULL; 
	char *mach = NULL;
	char *dom = NULL;

	if (domain[0] == 0 && getdomainname(domain, sizeof(domain)) < 0) {
		(void) fprintf(stderr, 
		    "getpwent: getdomainname system call missing\n");
		exit(1);
	}
	if (pwf == NULL && (pwf = fopen(PASSWD, "r")) == NULL) {
		return (NULL); 
	}

	for (;;) {
		if (yp) {
			pw = interpretwithsave(yp, yplen, savepw); 
			free(yp);
			getnextfromyellow();
			if (!onminuslist(pw)) {
				return(pw);
			}
		} else if (getnetgrent(&mach,&user,&dom)) {
			if (user) {
				pw = getnamefromyellow(user, savepw);
				if (pw != NULL && !onminuslist(pw)) {
					return(pw);
				}
			}
		} else {
			endnetgrent();
			if (fgets(line1, BUFSIZ, pwf) == NULL)  {
				return(NULL);
			}
			pw = interpret(line1, strlen(line1));
			switch(line1[0]) {
			case '+':
				if (strcmp(pw->pw_name, "+") == 0) {
					getfirstfromyellow();
					savepw = save(pw);
				} else if (line1[1] == '@') {
					savepw = save(pw);
					if (innetgr(pw->pw_name+2,(char *) NULL,
					"*",domain)) {
					/* include the whole yp database */
						getfirstfromyellow();
					} else {
						setnetgrent(pw->pw_name+2);
					}
				} else {
					/* 
					 * else look up this entry
					 * in yellow pages
				 	 */
					savepw = save(pw);
					pw = getnamefromyellow(pw->pw_name+1,
									savepw);
					if (pw != NULL && !onminuslist(pw)) {
						return(pw);
					}
				}
				break;
			case '-':
				if (line1[1] == '@') {
					if (innetgr(pw->pw_name+2,(char *) NULL,
					"*",domain)) {
						/* everybody was subtracted */
						return(NULL);
					}
					setnetgrent(pw->pw_name+2);
					while (getnetgrent(&mach,&user,&dom)) {
						if (user) {
							addtominuslist(user);
						}
					}
					endnetgrent();
				} else {
					addtominuslist(pw->pw_name+1);
				}
				break;
			default:
				if (!onminuslist(pw)) {
					return(pw);
				}
				break;
			}
		}
	}
}

static
matchname(line1, pwp, name)
	register char *line1;
	register struct passwd **pwp;
	register char *name;
{
	register struct passwd *savepw;
	register struct passwd *pw = *pwp;

	switch(line1[0]) {
		case '+':
			if (strcmp(pw->pw_name, "+") == 0) {
				savepw = save(pw);
				pw = getnamefromyellow(name, savepw);
				if (pw) {
					*pwp = pw;
					return 1;
				}
				else
					return 0;
			}
			if (line1[1] == '@') {
				if (innetgr(pw->pw_name+2,(char *) NULL,
				name,domain)) {
					savepw = save(pw);
					pw = getnamefromyellow(name,savepw);
					if (pw) {
						*pwp = pw;
						return 1;
					}
				}
				return 0;
			}
			if (strcmp(pw->pw_name+1, name) == 0) {
				savepw = save(pw);
				pw = getnamefromyellow(pw->pw_name+1, savepw);
				if (pw) {
					*pwp = pw;
					return 1;
				}
				else
					return 0;
			}
			break;
		case '-':
			if (line1[1] == '@') {
				if (innetgr(pw->pw_name+2,(char *) NULL,
				name,domain)) {
					*pwp = NULL;
					return 1;
				}
			}
			else if (strcmp(pw->pw_name+1, name) == 0) {
				*pwp = NULL;
				return 1;
			}
			break;
		default:
			if (pw->pw_name[0] == name[0])	/* avoid calls */
				if (strcmp(pw->pw_name, name) == 0)
					return 1;
	}
	return 0;
}

static
matchuid(line1, pwp, uid)
	register char *line1;
	register struct passwd **pwp;
	register int uid;
{
	register struct passwd *savepw;
	register struct passwd *pw = *pwp;
	char group[256];

	switch(line1[0]) {
		case '+':
			if (strcmp(pw->pw_name, "+") == 0) {
				savepw = save(pw);
				pw = getuidfromyellow(uid, savepw);
				if (pw) {
					*pwp = pw;
					return 1;
				} else {
					return 0;
				}
			}
			if (line1[1] == '@') {
				(void) strcpy(group,pw->pw_name+2);
				savepw = save(pw);
				pw = getuidfromyellow(uid,savepw);
				if (pw && innetgr(group,(char *) NULL,
				pw->pw_name,domain)) {
					*pwp = pw;
					return 1;
				} else {
					return 0;
				}
			}
			savepw = save(pw);
			pw = getnamefromyellow(pw->pw_name+1, savepw);
			if (pw && pw->pw_uid == uid) {
				*pwp = pw;
				return 1;
			} else
				return 0;
			break;
		case '-':
			if (line1[1] == '@') {
				(void) strcpy(group,pw->pw_name+2);
				pw = getuidfromyellow(uid,&NULLPW);
				if (pw && innetgr(group,(char *) NULL,
				pw->pw_name,domain)) {
					*pwp = NULL;
					return 1;
				}
			} else if (uid == uidof(pw->pw_name+1)) {
				*pwp = NULL;
				return 1;
			}
			break;
		default:
			if (pw->pw_uid == uid)
				return 1;
	}
	return 0;
}

static
uidof(name)
	char *name;
{
	struct passwd *pw;
	struct passwd nullpw;
	
	nullpw = NULLPW;
	pw = getnamefromyellow(name, &nullpw);
	if (pw)
		return pw->pw_uid;
	else
		return MAXINT;
}

static
getnextfromyellow()
{
	int reason = 0;
	char *key = NULL;
	int keylen = 0;

	reason = yp_next(domain, "passwd.byname",oldyp, oldyplen, &key,
	&keylen,&yp,&yplen);
	if (reason) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif DEBUG
		yp = NULL;
	}
	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}

static
getfirstfromyellow()
{
	int reason = 0;
	char *key = NULL;
	int keylen = 0;
	
	reason =  yp_first(domain, "passwd.byname", &key, &keylen, &yp, &yplen);
	if (reason) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif DEBUG
		yp = NULL;
	}
	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}

static struct passwd *
getnamefromyellow(name, savepw)
	char *name;
	struct passwd *savepw;
{
	struct passwd *pw;
	int reason = 0;
	char *val = NULL;
	int vallen = 0;
	
	reason = yp_match(domain, "passwd.byname", name, strlen(name)
		, &val, &vallen);
	if (reason) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif DEBUG
		return NULL;
	} else {
		pw = interpret(val, vallen);
		free(val);
		if (savepw->pw_passwd && *savepw->pw_passwd)
			pw->pw_passwd =  savepw->pw_passwd;
		if (savepw->pw_gecos && *savepw->pw_gecos)
			pw->pw_gecos = savepw->pw_gecos;
		if (savepw->pw_dir && *savepw->pw_dir)
			pw->pw_dir = savepw->pw_dir;
		if (savepw->pw_shell && *savepw->pw_shell)
			pw->pw_shell = savepw->pw_shell;
		return pw;
	}
}

static struct passwd *
getuidfromyellow(uid, savepw)
	int uid;
	struct passwd *savepw;
{
	struct passwd *pw;
	int reason = 0;
	char *val = NULL;
	int vallen = 0;
	char uidstr[20];
	
	(void) sprintf(uidstr, "%d", uid);
	reason = yp_match(domain, "passwd.byuid", uidstr, strlen(uidstr)
		, &val, &vallen);
	if (reason) {
#ifdef DEBUG
		fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif DEBUG
		return NULL;
	} else {
		pw = interpret(val, vallen);
		free(val);
		if (savepw->pw_passwd && *savepw->pw_passwd)
			pw->pw_passwd =  savepw->pw_passwd;
		if (savepw->pw_gecos && *savepw->pw_gecos)
			pw->pw_gecos = savepw->pw_gecos;
		if (savepw->pw_dir && *savepw->pw_dir)
			pw->pw_dir = savepw->pw_dir;
		if (savepw->pw_shell && *savepw->pw_shell)
			pw->pw_shell = savepw->pw_shell;
		return pw;
	}
}

static struct passwd *
interpretwithsave(val, len, savepw)
	register char *val;
	register struct passwd *savepw;
{
	register struct passwd *pw;
	
	pw = interpret(val, len);
	if (savepw->pw_passwd && *savepw->pw_passwd)
		pw->pw_passwd =  savepw->pw_passwd;
	if (savepw->pw_gecos && *savepw->pw_gecos)
		pw->pw_gecos = savepw->pw_gecos;
	if (savepw->pw_dir && *savepw->pw_dir)
		pw->pw_dir = savepw->pw_dir;
	if (savepw->pw_shell && *savepw->pw_shell)
		pw->pw_shell = savepw->pw_shell;
	return pw;
}

static struct passwd *
interpret(val, len)
	char *val;
	int len;
{
	register char *p, *v;
	register int i;
	static struct passwd passwd;
	static char line[BUFSIZ+1];
	register struct passwd *pw = &passwd;

	p = line;
	v = val;
	for (i = 0; i < len; i++) {
		if ((*p++ = *v++) == '\0') {
			while (++i < len)
				*p++ = '\0';
			break;
		}
	}
	*p++ = '\n';
	*p = 0;
	p = line;

	pw->pw_name = p;
	PWSKIP(p);
	pw->pw_passwd = p;
	PWSKIP(p);
	ATOI(p,pw->pw_uid);
	PWSKIP(p);
	ATOI(p,pw->pw_gid);

	/* The only difference in the pwd structure between ULTRIX
	 * and System V is in the definition of the following field.
	 * However, both systems ignore it because it is not present
	 * in the actual password file.
	 */
#ifndef SYSTEM_FIVE
	pw->pw_quota = 0;
#else	SYSTEM_FIVE
	pw->pw_age = NULL;
#endif	SYSTEM_FIVE

	pw->pw_comment = EMPTY;
	PWSKIP(p);
	pw->pw_gecos = p;
	PWSKIP(p);
	pw->pw_dir = p;
	PWSKIP(p);
	pw->pw_shell = p;
	while(*p && *p != '\n') p++;
	*p = '\0';
	return(pw);
}

static
freeminuslist()
{
	struct list *ls;
	
	for (ls = minuslist; ls != NULL; ls = ls->nxt) {
		free(ls->name);
		free((char *) ls);
	}
	minuslist = NULL;
}

static
addtominuslist(name)
	char *name;
{
	struct list *ls;
	char *buf;
	
	ls = (struct list *) malloc(sizeof(struct list));
	buf = malloc((unsigned) strlen(name) + 1);
	(void) strcpy(buf, name);
	ls->name = buf;
	ls->nxt = minuslist;
	minuslist = ls;
}

/* 
 * save away psswd, gecos, dir and shell fields, which are the only
 * ones which can be specified in a local + entry to override the
 * value in the yellow pages
 */
static struct passwd *
save(pw)
	struct passwd *pw;
{
	static struct passwd *sv;

	/* free up stuff from last call */
	if (sv) {
		free(sv->pw_passwd);
		free(sv->pw_gecos);
		free(sv->pw_dir);
		free(sv->pw_shell);
		free((char *) sv);
	}
	sv = (struct passwd *) malloc(sizeof(struct passwd));

	sv->pw_passwd = malloc((unsigned) strlen(pw->pw_passwd) + 1);
	(void) strcpy(sv->pw_passwd, pw->pw_passwd);

	sv->pw_gecos = malloc((unsigned) strlen(pw->pw_gecos) + 1);
	(void) strcpy(sv->pw_gecos, pw->pw_gecos);

	sv->pw_dir = malloc((unsigned) strlen(pw->pw_dir) + 1);
	(void) strcpy(sv->pw_dir, pw->pw_dir);

	sv->pw_shell = malloc((unsigned) strlen(pw->pw_shell) + 1);
	(void) strcpy(sv->pw_shell, pw->pw_shell);

	return sv;
}

static
onminuslist(pw)
	struct passwd *pw;
{
	struct list *ls;
	register char *nm;

	nm = pw->pw_name;
	for (ls = minuslist; ls != NULL; ls = ls->nxt) {
		if (strcmp(ls->name,nm) == 0) {
			return(1);
		}
	}
	return(0);
}

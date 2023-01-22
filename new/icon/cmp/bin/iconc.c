#include <stdio.h>
#include "../h/config.h"
#define BIN "/usr/new/lib/icon/iconc"
#define CC "/bin/cc"
#define AS "/bin/as"
#define LD "/bin/ld"
#define MAXARGS 20
#define PATHSIZE 100	/* max length of a fully qualified file name */
#ifndef UTRAN
#define UTRAN "%s/utran"
#endif
#ifndef ULINK
#define ULINK "%s/ulink"
#endif
#define START "%s/start.o"
#define LIB "%s/libi.a"
#define PSTART "%s/pstart.o"
#define PSTOP "%s/pstop.o"
char **rfiles;
char *ofile, *object;
int quiet, ac;
char *ccargs[10] = {"cc", "-O", "-c"};
#ifdef VAX
char *asargs[10] = {"as", "-o"};
int nas = 2;
#endif VAX
#ifdef PDP11
char *asargs[10] = {"as", "-u", "-o"};
int nas = 3;
#endif PDP11
char *ourenv[1] = {0};

main(argc,argv)
int argc; char **argv;
{
	char **tfiles;
	char **lfiles;
	char **cfiles;
	char **ofiles;
	char **execlist;
	char *tflags[MAXARGS];
	char *lflags[MAXARGS];
	int ntf, nlf, nrf, ntflags, nlflags, nof, ncf;
	int cflag, rc;
	char **arg;
	char *sflag;
	char *base, *getbase();
	char *u1, *u2, *doto;
	char *rindex(), *mkname();
	char cmd[PATHSIZE];
	
	rfiles = (char **)calloc(2*(argc+10), sizeof(char **));
	tfiles = (char **)calloc(argc+10, sizeof(char **));
	lfiles = (char **)calloc(argc+10, sizeof(char **));
	cfiles = (char **)calloc(argc+10, sizeof(char **));
	ofiles = (char **)calloc(argc+10, sizeof(char **));
	execlist = (char **)calloc(2*(argc+10), sizeof(char **));
	ac = argc;
	tflags[ntflags++] = "utran";
	lflags[nlflags++] = "ulink";
	rfiles[nrf++] = "rm"; rfiles[nrf++] = "-f";
	object = "";
	
	for (arg = &argv[1]; arg <= &argv[argc-1]; arg++) {
		if ((*arg)[0] == '-') switch ((*arg)[1]) {
			case '\0': /* "-" */
				tfiles[ntf++] = *arg;
				lfiles[nlf++] = rfiles[nrf++]
				  = "stdin.u1";
				rfiles[nrf++] = "stdin.u2";
				break;
			case 's':
				tflags[ntflags++] = "-s";
				quiet++;
				break;
			case 'o':
				object = *++arg;
				break;
			case 'c':
				cflag++;
				break;
			case 'l':
				ofiles[nof++] = *arg;
				break;
			default:
				lflags[nlflags++] = tflags[ntflags++] = *arg;
				break;
				}
		else if (suffix(*arg,".icn")) {
			tfiles[ntf++] = *arg;
			base = getbase(*arg,".icn");
			u1 = mkname(base,".u1");
			u2 = mkname(base,".u2");
			lfiles[nlf++] = rfiles[nrf++] = u1;
			rfiles[nrf++] = u2;
			}
		else if (suffix(*arg,".u1")) {
			lfiles[nlf++] = *arg;
			}
		else if (suffix(*arg,".c")) {
			cfiles[ncf++] = *arg;
			base = getbase(*arg,".c");
			doto = mkname(base,".o");
			ofiles[nof++] = doto;
			}
		else {
			ofiles[nof++] = *arg;
			}
		}
argsdone:
	if (nlf == 0)
		usage(argv[0]);
	ofile = getbase(lfiles[0],".u1");
	if (!object[0])
		object = ofile;
	
	if (ntf != 0) {
		lcat(execlist,tflags,tfiles);
		sprintf(cmd,UTRAN,BIN);
		runit(cmd,execlist,ourenv);
		}
	if (ncf != 0) {
		int i;
		for (i = 0; i < ncf; i++) {
			if (!quiet)
				fprintf(stderr,"%s:\n",cfiles[i]);
			ccargs[3] = cfiles[i];
			runit(CC,ccargs,ourenv);
			}
		}
	if (cflag) {
		exit(0);
		}
	if (!quiet)
		fprintf(stderr,"Linking:\n");
	execlist[0] = 0;
	lcat(execlist,lflags,lfiles);
	sprintf(cmd,ULINK,BIN);
	runit(cmd,execlist,ourenv);
	if (!quiet)
		fprintf(stderr,"Assembling:\n");
	asargs[nas++] = rfiles[nrf++] = mkname(ofile,".o");
	asargs[nas++] = rfiles[nrf++] = mkname(ofile,".s");
	runit(AS,asargs,ourenv);
	dold(ofiles);
	docmd("/bin/rm",rfiles,ourenv);
}
runit(c,a,e)
char *c; char **a, **e;
{
	int rc;
	if ((rc = docmd(c,a,e)) != 0) {
		docmd("/bin/rm",rfiles,e);
		exit(1);
		}
}
suffix(name,suf)
char *name,*suf;
{
	return !strcmp(suf,rindex(name,'.'));
}
char *
mkname(name,suf)
char *name,*suf;
{
	char *p, *malloc();
	
	p = malloc(16);
	strcpy(p,name);
	strcat(p,suf);
	return p;
}
char *
getbase(name,suf)
char *name,*suf;
{
	char *f,*e, *rindex(), *p, *malloc();
	
	if (!(f = rindex(name,'/')))
		f = name;
	else
		f++;
	e = rindex(f,'.');
	p = malloc(16);
	strncpy(p,f,e-f);
	return p;
}
plist(title,list)
char *title, **list;
{
	char **p;
	printf("\n%s\n",title);
	for (p = list; *p; p++)
		printf("'%s'\n",*p);
}
lcat(c,a,b)
int c[],a[],b[];
{
	int cp,p;
	
	cp = p = 0;
	while (c[cp])
		cp++;
	while (c[cp] = a[p++])
		cp++;
	p = 0;
	while (c[cp++] = b[p++]);
}
usage(p)
char *p;
{
	fprintf(stderr,"usage: %s [-c] [-m] [-t] [-u] file ... [-x args]\n",p);
	exit(1);
}
docmd(cmd,argv,envp)
char *cmd, **argv, **envp;
{
	int rc, stat;
	
#ifdef VAX
	rc = vfork();
#else
	rc = fork();
#endif
	if (rc == -1) {
		fprintf(stderr,"No more processes\n");
		return 255;
		}
	if (rc == 0) {
		execve(cmd,argv,envp);
		fprintf(stderr,"exec failed on %s\n",cmd);
		_exit(255);
		}
	while (rc != wait(&stat));
	return (stat>>8) & 0xff;
}
dold(ofiles)
char **ofiles;
{
	char Start[PATHSIZE], Lib[PATHSIZE], Pstart[PATHSIZE],
		Pstop[PATHSIZE];
	char **ldargs, *ldargs2[10];
	int nld;
	
	ldargs = (char **)calloc(ac+20, sizeof(char **));
	if (!quiet)
		fprintf(stderr,"Loading:\n");
	sprintf(Start,START,BIN);
	sprintf(Lib,LIB,BIN);
	sprintf(Pstart,PSTART,BIN);
	sprintf(Pstop,PSTOP,BIN);
	nld = 0;
	ldargs[nld++] = "ld";
	ldargs[nld++] = "-X";
#ifdef PDP11
	ldargs[nld++] = "-i";
#endif PDP11
	ldargs[nld++] = Start;
	ldargs[nld++] = mkname(ofile,".o");
	ldargs[nld++] = Pstart;
	ldargs2[0] = Lib;
	ldargs2[1] = Pstop;
	ldargs2[2] = "-lc";
	ldargs2[3] = "-o";
	ldargs2[4] = object;
#ifdef NOFP
	ldargs2[5] = "-lfpsim";
#endif NOFP
	lcat(ldargs,ofiles,ldargs2);
	runit(LD,ldargs,ourenv);
}

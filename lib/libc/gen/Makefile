#
#  @(#)Makefile	1.14	ULTRIX	12/16/86
#
# 	Modifications
#
# 007	Ray Glaser  xx-Jun-86
#	remove modules:
#	getgrgid, getgrnam, getpwnam, getpwuid 
#
#	Replace by the Sun modules getgrent.c and getpwent.c
#	which has common code for Yellow pages lookup and which
#	define the entry points in one source vs 3.
#
#	Stephen Reilly, 20-Sep-85
#	Added System V lockf 
#	David L Ballenger, 29-May-1985
# 006	Add more System V routines. 
#
#	David L Ballenger, 23-Apr-1985
# 005	Have the tools rule use OBJS variable rather than STDOBJS.
#
#	David L Ballenger, 15-Apr-1985
# 004	Add more System V routines.
#
#	David L Ballenger, 31-Mar-1985
# 003	Add System V routines.
#
#	David L Ballenger,	 8-Mar-1985
# 002	Add ftok for IPC support from system V.
#
# 001-	David L Ballenger,	24-Aug-1984
#	Make GFLOAT version of library routines for libc.
#

CFLAGS=	-O
CWD=`pwd`
DESTROOT=
ECHO=/bin/echo

V_OBJS=	a64l.o bsearch.o clock.o dial.o drand48.o ftok.o ftw.o\
	getcwd.o getopt.o getut.o hsearch.o\
	l3.o l64a.o lfind.o lockf.o lsearch.o\
	memccpy.o memchr.o memcmp.o memcpy.o memset.o \
	putenv.o putpwent.o ssignal.o strtod.o\
	strchr.o strcspn.o strpbrk.o strrchr.o strspn.o strtok.o strtol.o\
	tfind.o tolower.o toupper.o tsearch.o tell.o \
	v_regcmp.o v_regex.o

STDOBJS=alarm.o atoi.o atol.o \
	calloc.o closedir.o crypt.o ctime.o ctype_.o \
	execvp.o \
	fstab.o \
	ftok.o \
	gcvt.o getenv.o getgrent.o getlogin.o \
	getpass.o getpwent.o getwd.o \
	initgroups.o isatty.o \
	malloc.o mktemp.o \
	nlist.o \
	opendir.o \
	perror.o popen.o psignal.o \
	qsort.o \
	random.o readdir.o regex.o \
	scandir.o seekdir.o setgid.o setegid.o setrgid.o setuid.o \
	seteuid.o setruid.o siglist.o signal.o sleep.o \
	swab.o syslog.o system.o telldir.o \
	time.o timezone.o ttyname.o ttyslot.o \
	valloc.o \
	getttyent.o getttynam.o

OBJS=	${STDOBJS} ${V_OBJS} errlst.o

.c.o:
	${CC} -p ${CFLAGS} -c $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} ${CFLAGS} -Mg -c $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

genlib profiled/genlib gfloat/genlib: ${OBJS}
	@echo "building profiled genlib"
	@cd profiled; ${ECHO} ${CWD}; ar cru genlib ${OBJS}
	@echo "building gfloat genlib"
	@cd gfloat; ${ECHO} ${CWD}; ar cru genlib ${OBJS}
	@echo "buiding normal genlib"
	@ar cru genlib ${OBJS}

tools:	errlst.o
	for i in ${OBJS}; do \
		echo $$i; \
		${CC} ${CFLAGS} -c `basename $$i .o`.c; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal genlib"
	ar cru genlib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f genlib profiled/genlib gfloat/genlib \
	      ${OBJS} profiled/*.o gfloat/*.o

clobber: clean
	rm -f Makefile :errfix *.c 

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS


errlst.o: errlst.c
	cc -S errlst.c
	ed - <:errfix errlst.s
	as -o errlst.o errlst.s
	cp errlst.o profiled/errlst.o
	cp errlst.o gfloat/errlst.o
	rm errlst.s


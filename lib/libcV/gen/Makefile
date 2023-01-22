#  @(#)Makefile	1.6	ULTRIX	10/3/86
#
#
#
# 	Modifications
#
# 001	David L Ballenger, 7-Jun-1985
#	Move strtod, ssignal, strtol to standard C library.
#
# 002	David L Ballenger, 17-Oct-1985
#	Remove ctype_.[co] since the behavior is now the same on
#	ULTRIX, System V, and 4.3BSD.
#
# 003	Ray Glaser, 1-Aug-86
#	Remove  getgrid, getgrnam, getpwnam, getpwuid
#	which are replace by Yellow Pages modules getgrent
#	and getpwent.
#
CFLAGS=	-O -Y
DESTROOT=
LIBC=	../../libc
GEN=	${LIBC}/gen
COMPAT41=${LIBC}/compat-4.1

GENSRC= closedir.c crypt.c dial.c execvp.c ftw.c getgrent.c \
	getopt.c \
	getpwent.c getut.c getwd.c hsearch.c \
	initgroups.c mktemp.c nlist.c opendir.c perror.c \
	putenv.c putpwent.c \
	popen.c psignal.c readdir.c scandir.c \
	system.c tell.c ttyname.c

COMMON=	${GENSRC} getpw.c

OBJS=	abort.o assert.o \
	closedir.o crypt.o ctime.o dial.o\
	execvp.o ftw.o getgrent.o  getlogin.o \
	getopt.o getpass.o getpw.o getpwent.o \
	getut.o getwd.o hsearch.o\
	initgroups.o isatty.o mktemp.o nlist.o opendir.o \
	perror.o putenv.o putpwent.o popen.o psignal.o readdir.o  \
	scandir.o sleep.o \
	system.o tell.o ttyname.o ttyslot.o 

.c.o:
	${CC} -p ${CFLAGS} -c $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} ${CFLAGS} -Mg -c $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

genlib profiled/genlib gfloat/genlib: gfloat profiled ${OBJS}
	@for d in . profiled gfloat; \
	do ( \
		echo "Creating libcV/gen/$$d/genlib" ; \
		cd $$d; ar cru genlib ${OBJS} \
	) ; done

gfloat profiled:
	mkdir $@

clean:
	rm -rf genlib profiled gfloat *.o

clobber: clean
	rm -f Makefile :errfix *.c 

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

${GENSRC}:
	ln -s ${GEN}/$@
getpw.c:
	ln -s ${COMPAT41}/$@

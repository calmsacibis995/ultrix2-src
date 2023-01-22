# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	@(#)Makefile	4.6	6/15/83
#
ALL=	ftp
DESTROOT=
CFLAGS=	-O

all:	${ALL}

ftp:	cmds.o cmdtab.o ftp.o getpass.o glob.o main.o
	${CC} cmds.o cmdtab.o ftp.o getpass.o glob.o main.o -o ftp 

ftp.o:	/usr/include/arpa/ftp.h ftp_var.h
cmds.o:	/usr/include/arpa/ftp.h ftp_var.h
cmdtab.o: ftp_var.h
ftp.o:	ftp_var.h
main.o: ftp_var.h

install: all
	install -c -s ftp ${DESTROOT}/usr/ucb/ftp

clean:
	rm -f ${ALL} *.o *.s errs core a.out t.?

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

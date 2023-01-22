# @(#)Makefile	1.6	ULTRIX	12/4/84
#
#	@(#)Makefile	4.2	6/12/83
#
ALL=	ftpd.o ftpcmd.o glob.o vers.o
DESTROOT=
CFLAGS=	-O

ftpd:	${ALL}
	${CC} -o ftpd ${ALL}

vers.o:	ftpd.c ftpcmd.y
	sh newvers.sh
	${CC} -c vers.c

install: ftpd
	install -c -s ftpd ${DESTROOT}/etc/ftpd

clean:
	rm -f ftpd *.o *.s errs core a.out t.? yacc.* y.tab.c

clobber: clean
	rm -f *.[chy] newvers.sh Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

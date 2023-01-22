#	sccsid = "@(#)Makefile	1.7	(ULTRIX)	6/28/85";

#	Makefile	4.2	85/02/05
#
DESTROOT=
CFLAGS=	-O
OBJS=	main.o init.o subr.o gettytab.o get_date.o 

all:	getty

getty:	${OBJS} 
	${CC} -o getty ${OBJS} 

install: all
	install -c -s getty ${DESTROOT}/etc/getty

clean:
	rm -f getty ${OBJS} a.out core errs

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

main.o: gettytab.h
init.o: gettytab.h
subr.o: gettytab.h

#
#  @(#)Makefile	1.5	ULTRIX	10/2/86
#
#
#	Makefile for Opser
#
#	opser.h		Header file
#	opser.c		Main code
#	remote_opser.c	Slave Opser code
#	parse.c		Command parser
#
#
DESTROOT=
PSRCS = \
	opser.c parse.c remote_opser.c

SRCS = \
	opser.c parse.c remote_opser.c

OBJS = \
	opser.o parse.o remote_opser.o

DFLAGS=
CFLAGS= -O ${DFLAGS}

all: opser is_vaxstar

${OBJS}: opser.h

opser:  ${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o opser

is_vaxstar: is_vaxstar.c
	${CC} ${CFLAGS} -o is_vaxstar is_vaxstar.c

install: all
	-rm -f -r ${DESTROOT}/opr
	-if [ ! -d ${DESTROOT}/opr ]; then \
		mkdir ${DESTROOT}/opr; \
		/etc/chown root ${DESTROOT}/opr; \
		chmod 755 ${DESTROOT}/opr; \
	else true; \
	fi
	install -c -m 755 opser ${DESTROOT}/opr/opser
	install -c -m 755 backup ${DESTROOT}/opr/backup
	install -c -m 755 dobackup ${DESTROOT}/opr/dobackup
	install -c -m 755 is_vaxstar ${DESTROOT}/opr/is_vaxstar

clean:
	rm -f *.o opser is_vaxstar

clobber: clean
	rm -f opser opser.c parse.c remote_opser.c is_vaxstar.c backup dobackup Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

print:
	pr $(PSRCS) | lpr

$(OBJS): /usr/include/stdio.h
$(OBJS): /usr/include/ctype.h
$(OBJS): /usr/include/signal.h
$(OBJS): /usr/include/sgtty.h
$(OBJS): /usr/include/time.h
$(OBJS): /usr/include/sys/file.h
$(OBJS): /usr/include/errno.h
$(OBJS): /usr/include/sys/types.h
$(OBJS): /usr/include/sys/socket.h
$(OBJS): /usr/include/sys/dir.h
$(OBJS): /usr/include/netinet/in.h
$(OBJS): /usr/include/netdb.h
$(OBJS): /usr/include/a.out.h
$(OBJS): /usr/include/strings.h
$(OBJS): /usr/include/sys/stat.h
$(OBJS): /usr/include/sys/wait.h

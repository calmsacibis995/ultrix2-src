# @(#)Makefile	1.7	ULTRIX	1/27/86
#
#	Makefile	2.15	83/08/18
#
# Berkeley Mail
#
CFLAGS=	-O -DVMUNIX -DV7
XSTR=	/usr/ucb/xstr
DESTROOT=
CTAGS=	/usr/ucb/ctags -w
VPRINT=	/usr/ucb/print
CGRAF=	/ja/rigel/rigel/bin/cgraf
CGRAPH=	/ja/rigel/rigel/bin/cgraph
NMKINDEX=/ja/rigel/rigel/bin/nmkindex
DIRTCLOD=/ja/rigel/rigel/bin/dirtclod
LIBES=
ED=	-ed
AS=	-as
RM=	-rm
OBJS=	version.o aux.o cmd1.o cmd2.o cmd3.o cmdtab.o collect.o \
	config.o edit.o fio.o \
	getname.o head.o v7.local.o lock.o lex.o list.o main.o \
	names.o optim.o quit.o send.o strings.o temp.o tty.o \
	vars.o str.o
SRCS=	aux.c cmd1.c cmd2.c cmd3.c cmdtab.c collect.c config.c edit.c fio.c \
	getname.c head.c v7.local.c lock.c lex.c list.c main.c \
	names.c optim.c quit.c send.c strings.c temp.c tty.c vars.c \
	version.c
HDRS=	rcv.h configdefs.h def.h glob.h v7.local.h local.h
ALL=	Mail fmt
S = $(SRCS) $(HDRS)

#
# Special massaging of C files for sharing of strings
#
.c.o:
	${CC} -E ${CFLAGS} $*.c | ${XSTR} -c -
	${CC} -c ${CFLAGS} x.c 
	mv x.o $*.o
	@$(RM) -f x.c

all:	${ALL}

Mail:	$S $(OBJS)
	$(RM) -f Mail
	@echo Loading ...
	@$(CC) -n -o Mail $(OBJS) $(LIBES)

mail.g:	$(SRCS)
	$(CGRAF) $(SRCS) > mail.g

install: all
	install -c -s Mail ${DESTROOT}/usr/ucb/Mail
	cp misc/Mail.* ${DESTROOT}/usr/lib
	install -c -s fmt ${DESTROOT}/usr/ucb/fmt
	cd ${DESTROOT}/usr/ucb; rm -f mail; ln Mail mail

version.o:	version.c
	$(CC) -c -R version.c

$S:
	sccs get $@;

srcs:	$S

tags:	$(SRCS)
	${CTAGS} $(SRCS);

clean:
	$(RM) -f *.o
	rm -f Mail a.out fmt x.c xs.c tags core
	cp /dev/null strings

clobber: clean
	rm *.[ch] diffs.mark Makefile
	rm -f misc/Mail.*

sccsinfo:
	sccs info
	(cd misc; sccs info)

sccsget:
	sccs get SCCS
	(cd misc; sccs get SCCS)

lint:
	lint $(CFLAGS) $(SRCS)

fmt:	fmt.c head.c
	$(CC) fmt.c head.c -o fmt
	$(RM) fmt.o head.o

str.o: strings
	$(XSTR)
	$(CC) -R -c xs.c
	mv xs.o str.o
	$(RM) xs.c

cmdtab.o: cmdtab.c
	$(CC) -R -c $(CFLAGS) cmdtab.c

print:	$S fmt.c
	$(CGRAPH) > mail.g;
	$(NMKINDEX) > mail.i;
	$(DIRTCLOD) -x mail.i -p mail.g > mail.x
	@$(VPRINT) Makefile mail.x rcv.h def.h glob.h local.h v7.local.h \
	    $(SRCS) fmt.c;

wc:
	@wc rcv.h def.h glob.h local.h v7.local.h $(SRCS)

sc:
	@grep -c \; rcv.h def.h glob.h local.h v7.local.h $(SRCS)

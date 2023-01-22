#  @(#)Makefile	1.2	ULTRIX	10/3/86
# lcp Makefile  -- marc
CFLAGS = -O

SRCS = lcp.c direct.c lat_init.c
OBJS = lcp.o direct.o lat_init.o

all: 
	make lcp

lcp : $(OBJS) Makefile
	cc $(CFLAGS) -o lcp $(OBJS)

lcp.o : lcp.c direct.h common.h lat_init.c
	cc $(CFLAGS) -c lcp.c

direct.o : direct.c  direct.h common.h
	cc $(CFLAGS) -c direct.c

install: lcp
	install lcp ${DESTROOT}/etc/lcp
	
clean : 
	rm -f lcp $(OBJS)

clobber: clean
	rm -f  Makefile [a-z]*

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info

print :
	pr Makefile $(SRCS) direct.h common.h lat_init.c | lpr -J lcp

calls :
	cat $(SRCS) | calls -v | lpr -J callgraph


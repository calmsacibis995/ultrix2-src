#	1.3	Makefile	84/10/11
DESTROOT=
CFLAGS = -O -w

all:	Lib

Lib:		bscan.o create.o \
		escan.o field.o host.o keywd.o \
		limit.o llist.o mkrec.o 
	ar r ../bin/Lib $?
	touch Lib

bscan.o:	../h/rt.h
coact.o:	../h/rt.h
cofail.o:	../h/rt.h
coret.o:	../h/rt.h
create.o:	../h/rt.h
efail.o:	../h/rt.h
escan.o:	../h/rt.h
esusp.o:	../h/rt.h
field.o:	../h/rt.h ../h/record.h
host.o:		../h/rt.h ../h/config.h
invoke.o:	../h/rt.h
keywd.o:	../h/rt.h ../tran/keyword.h ../h/config.h
limit.o:	../h/rt.h
llist.o:	../h/rt.h
lsusp.o:	../h/rt.h
mkrec.o:	../h/rt.h ../h/record.h
pfail.o:	../h/rt.h
pret.o:		../h/rt.h
psusp.o:	../h/rt.h

Listall:
	@pr *.[cs]
	@date >List

#List:		bscan.c coact.s cofail.s coret.s create.c \
		efail.s escan.c esusp.s field.c invoke.s keywd.c \
		limit.c llist.c lsusp.s mkrec.c pfail.s pret.s psusp.s
#	@pr $?
#	@date >List

#.s.o:
#	cat ../h/defs.s $< >t.c
#	cc -E t.c | grep -v "^#" >t.s
#	as -o $@ t.s
#	rm -f t.c t.s
clean:
	@rm -f Lib *.o
clobber: clean
	@rm -f *.c Makefile
sccsinfo:
	@sccs info
sccsget:
	@sccs get -s SCCS

dist-clean: clean
	rm -f `gcomp Makefile *.c *.s`

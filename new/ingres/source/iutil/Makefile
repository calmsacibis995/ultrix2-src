#		@(#)Makefile	1.4	(ULTRIX)	2/13/85
#
#  Makefile for INGRES iutil library
#	(includes old access library)
#
#	David L Ballenger, 16-Jan-1984
# 001	Integrate into ULTRIX build procedures.
#

INGRES=	../..
H=	../h
LIB=	${INGRES}/lib

OBJS=	acc_addbuf.o add_prim.o batch.o batchxtra.o cleanrel.o \
	closecat.o closer.o clr_tuple.o delete.o do_u_flag.o \
	errfilen.o formatpg.o getnuser.o gettree.o getuser.o \
	initucode.o dblocks.o inpcloser.o insert.o markopen.o \
	noclose.o null_fn.o opencat.o cat_desc.o openr.o \
	get_reltup.o getequal.o get.o printdesc.o \
	replace.o del_tuple.o findbest.o add_ovflo.o cslocks.o \
	find.o last_page.o ndxsearch.o compare.o newlino.o \
	paramd.o printup.o out_arg.o put_tuple.o rhash.o \
	rllocks.o rub.o scan_dups.o get_tuple.o setkey.o \
	trim_relna.o tup_len.o writebatch.o accbuf.o page.o \
	pglocks.o globals.o readadmin.o checkadmin.o \
	ingresname.o utility.o acc_init.o lock.o getservent.o

SRCS=	acc_addbuf.c add_prim.c batch.c batchxtra.c cleanrel.c \
	closecat.c closer.c clr_tuple.c delete.c do_u_flag.c \
	errfilen.c formatpg.c getnuser.c gettree.c getuser.c \
	initucode.c dblocks.c inpcloser.c insert.c markopen.c \
	noclose.c null_fn.c opencat.c cat_desc.c openr.c \
	acc_init.c get_reltup.c getequal.c get.c printdesc.c \
	replace.c del_tuple.c findbest.c add_ovflo.c cslocks.c \
	find.c last_page.c ndxsearch.c compare.c newlino.c \
	paramd.c printup.c out_arg.c put_tuple.c rhash.c \
	rllocks.c rub.c scan_dups.c get_tuple.c setkey.c \
	trim_relna.c tup_len.c writebatch.c accbuf.c page.c \
	pglocks.c globals.c readadmin.c checkadmin.c \
	ingresname.c utility.c lock.c getservent.c

OLDOBJS=setjmp.o
OLDSRCS=setjmp.s

CFLAGS=	-I${H} -O 
AR=	-ar
ARFLAGS=urv
INSTALL=install
RANLIB=	ranlib
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Iutil Library
LIBMODE=664

all: iutil.a

iutil.a: ${OBJS}
	${AR} ${ARFLAGS} iutil.a $?
	${RANLIB} iutil.a

install: all

new:
	rm -f iutil.a

clean:
	rm -f *.o
	rm -f iutil.a a.out core

print: sources
	${PR} Makefile TraceFlags *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	${VGRIND} -h "${HEADER} (Release 7)" -n Makefile TraceFlags
	${VGRIND} -h "${HEADER} (Release 7)" *.[csy]
	${VGRIND} -h "${HEADER} (Release 7)" -x index

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

sources: ${SRCS}

#${SRCS}:
#	sccs get $@

save_version:
	@sccs get -s SCCS
	@sccs what ${FILES} | awk '$$2 != "" {print $$1, $$2}' > SCCS/version_${VERSION}

restore_version:
	@echo ${PATH}
	@for i in ${FILES}; do \
		echo "Restore" `grep $$i SCCS/version_${VERSION}`; \
		sccs get -s -r`grep $$i SCCS/version_${VERSION} | awk '{print $$2, $$1}'`; \
	done

show_current:
	@echo ${PATH}
	@sccs what ${FILES} | awk '$$2 != "" {printf "%s  %s\t", $$1, $$2}'
	@echo

depend:
	grep '^#[ 	]*include' ${SRCS} | sed -n -f ../h/depend.sed > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

acc_addbuf.o:$H/ingres.h
acc_addbuf.o:$H/access.h
add_prim.o:$H/ingres.h
add_prim.o:$H/access.h
batch.o:$H/ingres.h
batch.o:$H/aux.h
batch.o:$H/catalog.h
batch.o:$H/symbol.h
batch.o:$H/access.h
batchxtra.o:$H/ingres.h
batchxtra.o:$H/aux.h
batchxtra.o:$H/symbol.h
batchxtra.o:$H/access.h
cleanrel.o:$H/ingres.h
cleanrel.o:$H/aux.h
cleanrel.o:$H/access.h
closecat.o:$H/ingres.h
closecat.o:$H/aux.h
closecat.o:$H/access.h
closer.o:$H/ingres.h
closer.o:$H/access.h
clr_tuple.o:$H/ingres.h
clr_tuple.o:$H/symbol.h
clr_tuple.o:$H/access.h
delete.o:$H/ingres.h
delete.o:$H/access.h
do_u_flag.o:$H/ingres.h
do_u_flag.o:$H/aux.h
errfilen.o:$H/ingres.h
errfilen.o:$H/aux.h
errfilen.o:$H/version.h
formatpg.o:$H/ingres.h
formatpg.o:$H/access.h
getnuser.o:$H/ingres.h
getnuser.o:$H/aux.h
gettree.o:$H/ingres.h
gettree.o:$H/catalog.h
gettree.o:$H/tree.h
gettree.o:$H/symbol.h
getuser.o:$H/ingres.h
getuser.o:$H/aux.h
initucode.o:$H/ingres.h
initucode.o:$H/aux.h
initucode.o:$H/version.h
initucode.o:$H/opsys.h
initucode.o:$H/access.h
initucode.o:$H/lock.h
dblocks.o:$H/ingres.h
dblocks.o:$H/lock.h
inpcloser.o:$H/ingres.h
inpcloser.o:$H/access.h
insert.o:$H/ingres.h
insert.o:$H/access.h
markopen.o:$H/useful.h
markopen.o:$H/opsys.h
noclose.o:$H/ingres.h
noclose.o:$H/aux.h
noclose.o:$H/access.h
opencat.o:$H/ingres.h
opencat.o:$H/aux.h
opencat.o:$H/opsys.h
opencat.o:$H/access.h
cat_desc.o:$H/ingres.h
cat_desc.o:$H/aux.h
cat_desc.o:$H/access.h
openr.o:$H/ingres.h
openr.o:$H/aux.h
openr.o:$H/access.h
openr.o:$H/symbol.h
get_reltup.o:$H/ingres.h
get_reltup.o:$H/access.h
get_reltup.o:$H/aux.h
getequal.o:$H/ingres.h
getequal.o:$H/access.h
get.o:$H/ingres.h
get.o:$H/aux.h
get.o:$H/access.h
printdesc.o:$H/ingres.h
replace.o:$H/ingres.h
replace.o:$H/access.h
del_tuple.o:$H/ingres.h
del_tuple.o:$H/access.h
findbest.o:$H/ingres.h
findbest.o:$H/aux.h
findbest.o:$H/access.h
add_ovflo.o:$H/ingres.h
add_ovflo.o:$H/access.h
add_ovflo.o:$H/aux.h
add_ovflo.o:$H/lock.h
cslocks.o:$H/ingres.h
cslocks.o:$H/access.h
cslocks.o:$H/lock.h
find.o:$H/ingres.h
find.o:$H/aux.h
find.o:$H/symbol.h
find.o:$H/access.h
find.o:$H/lock.h
last_page.o:$H/ingres.h
last_page.o:$H/access.h
last_page.o:$H/opsys.h
ndxsearch.o:$H/ingres.h
ndxsearch.o:$H/aux.h
ndxsearch.o:$H/symbol.h
ndxsearch.o:$H/access.h
ndxsearch.o:$H/lock.h
compare.o:$H/ingres.h
compare.o:$H/access.h
compare.o:$H/symbol.h
compare.o:$H/aux.h
newlino.o:$H/ingres.h
newlino.o:$H/access.h
paramd.o:$H/ingres.h
paramd.o:$H/aux.h
paramd.o:$H/catalog.h
paramd.o:$H/access.h
printup.o:$H/ingres.h
printup.o:$H/aux.h
printup.o:$H/access.h
printup.o:$H/symbol.h
out_arg.o:$H/ingres.h
out_arg.o:$H/aux.h
put_tuple.o:$H/ingres.h
put_tuple.o:$H/symbol.h
put_tuple.o:$H/access.h
rhash.o:$H/ingres.h
rhash.o:$H/aux.h
rhash.o:$H/symbol.h
rhash.o:$H/access.h
rhash.o:$H/lock.h
rllocks.o:$H/ingres.h
rllocks.o:$H/lock.h
rub.o:$H/useful.h
scan_dups.o:$H/ingres.h
scan_dups.o:$H/symbol.h
scan_dups.o:$H/access.h
scan_dups.o:$H/lock.h
get_tuple.o:$H/ingres.h
get_tuple.o:$H/access.h
get_tuple.o:$H/symbol.h
setkey.o:$H/ingres.h
setkey.o:$H/symbol.h
trim_relna.o:$H/ingres.h
tup_len.o:$H/ingres.h
tup_len.o:$H/access.h
writebatch.o:$H/ingres.h
writebatch.o:$H/aux.h
writebatch.o:$H/symbol.h
writebatch.o:$H/access.h
accbuf.o:$H/ingres.h
accbuf.o:$H/access.h
accbuf.o:$H/aux.h
accbuf.o:$H/lock.h
accbuf.o:$H/opsys.h
page.o:$H/ingres.h
page.o:$H/access.h
page.o:$H/aux.h
page.o:$H/lock.h
pglocks.o:$H/ingres.h
pglocks.o:$H/access.h
pglocks.o:$H/lock.h
readadmin.o:$H/ingres.h
readadmin.o:$H/access.h
readadmin.o:$H/aux.h
readadmin.o:$H/lock.h
checkadmin.o:$H/ingres.h
checkadmin.o:$H/version.h
checkadmin.o:$H/access.h
ingresname.o:$H/ingres.h
utility.o:$H/ingres.h
utility.o:$H/access.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above

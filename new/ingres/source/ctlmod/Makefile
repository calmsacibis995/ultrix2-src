#		@(#)Makefile	1.4	(ULTRIX)	2/13/85
#
#  Makefile for the INGRES control module
#
#	David L Ballenger, 16-Jan-1984
# 001	Integrate into ULTRIX build procedures.
#

INGRES=	../..
H=	../h
LIB=	${INGRES}/lib

OBJS=	main.o error.o proc_err.o initp.o setp.o resetp.o \
	init_qt.o call.o sysdump.o mapvars.o \
	do_seq.o readinput.o do_st.o send_off.o call_fn.o \
	readmon.o readqry.o writeqry.o range.o treepr.o \
	pb_prime.o pb_get.o pb_tput.o pb_put.o pb_flush.o \
	pb_read.o pb_write.o pb_rphys.o pb_wphys.o pb_dump.o \
	cm_reset.o cm_cleanup.o cm_close.o \
	prvect.o getp.o monitor.o av_files.o

SRCS=	pipes.h proc.h state.h ctlmod.h \
	main.c error.c proc_err.c initp.c setp.c resetp.c \
	init_qt.c call.c sysdump.c mapvars.c \
	do_seq.c readinput.c do_st.c send_off.c call_fn.c \
	readmon.c readqry.c writeqry.c range.c treepr.c\
	pb_prime.c pb_get.c pb_tput.c pb_put.c pb_flush.c \
	pb_read.c pb_write.c pb_rphys.c pb_wphys.c pb_dump.c \
	cm_reset.c cm_cleanup.c cm_close.c \
	prvect.c getp.c monitor.c av_files.c

CFLAGS=	-O -I${H}
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Control Module
CTAGS=	/bin/csh /usr/bin/ctags
CXREF=	/bin/csh /mnt/eric/bin/cxref
AR=	-ar
ARFLAGS=rvu
RANLIB=	ranlib
LIBMODE=664
INSTALL=install

all: ctlmod.a

ctlmod.a: ${OBJS}
	${AR} ${ARFLAGS} ctlmod.a $?
	${RANLIB} ctlmod.a

install: all

new:
	rm -f ctlmod.a

clean:  new
	rm -f *.o
	rm -f a.out core

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
ctags:
	${CTAGS} *.c

print: sources
	${PR} Makefile TraceFlags *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	${VGRIND} -h "${HEADER} (Release 7)" -n Makefile TraceFlags
	${VGRIND} -h "${HEADER} (Release 7)" *.[hm] *.[csy]
	${VGRIND} -h "${HEADER} (Release 7)" -x index

depend: sources
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

ctlmod.h:state.h
ctlmod.h:proc.h
ctlmod.h:$H/pv.h
ctlmod.h:pipes.h
ctlmod.h:$H/func.h
ctlmod.h:$H/pmon.h
ctlmod.h:$H/useful.h
main.o:ctlmod.h
main.o:$H/resp.h
main.o:$H/ingres.h
main.o:$H/aux.h
error.o:ctlmod.h
error.o:$H/pv.h
proc_err.o:ctlmod.h
initp.o:ctlmod.h
initp.o:$H/ingres.h
initp.o:$H/aux.h
initp.o:$H/tree.h
setp.o:ctlmod.h
setp.o:$H/ingres.h
setp.o:$H/tree.h
setp.o:$H/aux.h
resetp.o:ctlmod.h
resetp.o:$H/ingres.h
resetp.o:$H/tree.h
init_qt.o:ctlmod.h
init_qt.o:$H/ingres.h
init_qt.o:$H/aux.h
init_qt.o:$H/tree.h
call.o:ctlmod.h
call.o:$H/resp.h
call.o:$H/ingres.h
call.o:$H/aux.h
call.o:$H/tree.h
sysdump.o:ctlmod.h
sysdump.o:$H/tree.h
mapvars.o:$H/ingres.h
mapvars.o:$H/symbol.h
mapvars.o:$H/tree.h
do_seq.o:ctlmod.h
do_seq.o:$H/resp.h
readinput.o:ctlmod.h
readinput.o:$H/resp.h
readinput.o:$H/ingres.h
readinput.o:$H/aux.h
readinput.o:$H/tree.h
do_st.o:ctlmod.h
send_off.o:ctlmod.h
send_off.o:$H/symbol.h
send_off.o:$H/tree.h
send_off.o:$H/resp.h
call_fn.o:ctlmod.h
call_fn.o:$H/resp.h
readqry.o:ctlmod.h
readqry.o:$H/ingres.h
readqry.o:$H/aux.h
readqry.o:$H/tree.h
readqry.o:$H/symbol.h
writeqry.o:$H/ingres.h
writeqry.o:$H/aux.h
writeqry.o:$H/tree.h
writeqry.o:$H/symbol.h
range.o:$H/ingres.h
range.o:$H/symbol.h
range.o:$H/range.h
range.o:$H/tree.h
treepr.o:$H/ingres.h
treepr.o:$H/symbol.h
treepr.o:$H/tree.h
treepr.o:$H/aux.h
pb_prime.o:ctlmod.h
pb_get.o:ctlmod.h
pb_tput.o:ctlmod.h
pb_put.o:ctlmod.h
pb_flush.o:ctlmod.h
pb_read.o:ctlmod.h
pb_write.o:ctlmod.h
pb_wphys.o:$H/useful.h
cm_reset.o:ctlmod.h
cm_cleanup.o:ctlmod.h
cm_close.o:ctlmod.h
prvect.o:$H/tree.h
prvect.o:$H/pv.h
getp.o:ctlmod.h
getp.o:$H/pv.h
monitor.o:$H/useful.h
monitor.o:$H/opsys.h
monitor.o:$H/pmon.h
av_files.o:$H/ingres.h
av_files.o:$H/aux.h
av_files.o:$H/opsys.h
av_files.o:ctlmod.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above

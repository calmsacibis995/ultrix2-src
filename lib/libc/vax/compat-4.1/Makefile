# @(#)Makefile	1.4	ULTRIX	11/20/84
#
#	Makefile	1.4	84/11/20
#
# 	Modifications
#
# 001-	David L Ballenger,	29-Aug-1984
#	Make GFLOAT version of library routines for libc.
#
OBJS=	reset.o
CFLAGS=
DESTROOT=

.c.o:
	/lib/cpp -E -DPROF $*.c | ${AS} -o $*.o
	-ld -X -r -o profiled/$*.o $*.o
	/lib/cpp -E $*.c | ${AS} -o $*.o
	-ld -x -r $*.o
	mv a.out $*.o

compat-4.1lib profiled/compat-4.1lib gfloat/compat-4.1lib: ${OBJS}
	@echo "building profiled compat-4.1"
	@cd profiled; ar cru compat-4.1lib ${OBJS}
	@echo "building normal compat-4.1"
	@ar cru compat-4.1lib ${OBJS}
	@echo "building gfloat compat-4.1"
	@cp compat-4.1lib gfloat/compat-4.1lib

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		/lib/cpp -E `basename $$i .o`.c | ${AS} -o $$i; \
		ld -x -r $$i; \
		mv a.out $$i; \
	done
	@echo "building normal compat-4.1"
	ar cru compat-4.1lib ${OBJS}
	rm -f ${OBJS}

clean:
	rm -f ${OBJS} profiled/*.o errs a.out core \
	       compat-4.1lib profiled/compat-4.1lib gfloat/compat-4.1lib

clobber: clean
	rm -f *.c Makefile 

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

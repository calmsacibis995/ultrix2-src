# @(#)Makefile	1.2	ULTRIX	9/23/85
#
#	Makefile	4.3	83/07/05
#
# 	Modifications
#
# Machine dependent routines for the VAX are located here
#
DESTROOT=

ALL=	sys

vaxlib profiled/vaxlib gfloat/vaxlib: ${ALL}
	for d in tmp gfloat profiled; do ( rm -rf $$d; mkdir $$d ); done
	@(cd tmp; \
	  for d in . profiled gfloat; \
	  do ( \
		echo "Creating libcV/vax/$$d/vaxlib" ; \
		rm -f * ; \
		for i in ${ALL}; do ar x ../$$i/$$d/$${i}lib ; done ; \
		ar cr ../$$d/vaxlib `ls *.o | sort -t/ +1` ; \
	  ); done )


${ALL}:	FRC
	cd $@; make ${MFLAGS}

FRC:

clean:
	for i in ${ALL}; do (cd $$i; make ${MFLAGS} clean); done
	rm -rf tmp vaxlib gfloat profiled

clobber: clean
	for i in ${ALL}; do (cd $$i; make ${MFLAGS} clobber); done
	rm -f Makefile 

sccsinfo:
	sccs info
	for i in ${ALL}; do (cd $$i; echo $$i; make ${MFLAGS} sccsinfo); done

sccsget:
	sccs get SCCS
	for i in ${ALL}; do (cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); done

.DEFAULT:
	sccs get $<

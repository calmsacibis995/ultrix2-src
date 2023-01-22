# @(#)Makefile	1.8	ULTRIX	11/20/84
#
#	Makefile	4.3	83/07/05
#
# 	Modifications
#
# 001-	David L Ballenger,	24-Aug-1984
#	Make GFLOAT version of library routines for libc.
#
# Machine dependent routines for the VAX are located here
#
DESTROOT=

COMPAT=	compat-4.1
ALL=	gen net stdio sys ${COMPAT}

vaxlib profiled/vaxlib gfloat/vaxlib: ${ALL}
	-if [ ! -d tmp ]; \
	then \
		mkdir tmp; \
	else \
		rm -f tmp/*; \
	fi
	-if [ ! -d profiled ]; \
	then \
		mkdir profiled; \
	else \
		true; \
	fi
	-if [ ! -d gfloat ]; \
	then \
		mkdir gfloat; \
	else \
		true; \
	fi
	for i in ${ALL}; do (cd tmp; ar x ../$$i/$${i}lib); done
	ar cr vaxlib `ls tmp/*.o | sort -t/ +1`
	rm -f tmp/*
	for i in ${ALL}; do (cd tmp; ar x ../$$i/gfloat/$${i}lib); done
	ar cr gfloat/vaxlib `ls tmp/*.o | sort -t/ +1`
	rm -f tmp/*
	for i in ${ALL}; do (cd tmp; ar x ../$$i/profiled/$${i}lib); done
	ar cr profiled/vaxlib `ls tmp/*.o | sort -t/ +1`
	rm -rf tmp

tools:
	for i in ${ALL}; do (cd $$i; make ${MFLAGS} tools); done
	-if [ ! -d tmp ]; \
	then \
		mkdir tmp; \
	else \
		rm -f tmp/*; \
	fi
	for i in ${ALL}; do (cd tmp; ar x ../$$i/$${i}lib); done
	ar cr vaxlib `ls tmp/*.o | sort -t/ +1`
	rm -rf tmp

${ALL}:	FRC
	cd $@; make ${MFLAGS}

FRC:

clean:
	for i in ${ALL}; do (cd $$i; make ${MFLAGS} clean); done
	rm -rf tmp;

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

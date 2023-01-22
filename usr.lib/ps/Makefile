#  @(#)Makefile	1.5	ULTRIX	10/3/86
#
#	Makefile for PostScript(TM) Print filters
#

OBJS = ps_img.o ps_plot.o ps_text.o ps_ascii.o

INSTFLAGS=
CFLAGS= -g
LINT=
LINTFLAGS=
STD = ps_img ps_plot ps_text ps_ascii
PROLOG = img_pro.h plot_pro.h ascii_pro.h
DESTROOT =
PSDIR = /usr/lib/ps
all: 	${STD}
ps_img: ps_img.o 
	ld -X -o ps_img /lib/crt0.o ps_img.o -lc -lg
ps_plot: ps_plot.o
	ld -X -o ps_plot /lib/crt0.o ps_plot.o -lm -lc -lg 
ps_text: ps_text.o
	ld -X -o ps_text /lib/crt0.o ps_text.o -lc -lg
ps_ascii: ps_ascii.o
	ld -X -o ps_ascii /lib/crt0.o ps_ascii.o -lc -lg
install: all
	-if [ ! -d ${DESTROOT}${PSDIR} ]; \
	 then \
		mkdir ${DESTROOT}${PSDIR}; \
		chmod 755 ${DESTROOT}${PSDIR}; \
		/etc/chown root ${DESTROOT}${PSDIR}; \
		chgrp system ${DESTROOT}${PSDIR}; \
	else true; \
	fi
	install ${INSTFLAGS} -c -m 755 ps_img ${DESTROOT}${PSDIR}/ps_img
	install ${INSTFLAGS} -c -m 755 ps_plot ${DESTROOT}${PSDIR}/ps_plot
	install ${INSTFLAGS} -c -m 755 ps_text ${DESTROOT}${PSDIR}/ps_text
	install ${INSTFLAGS} -c -m 755 ps_ascii ${DESTROOT}${PSDIR}/ps_ascii
clean:
	rm -f a.out core ${OBJS} ${STD} ${PROLOG}
clobber:	
	rm -f tags
	${SCCS} clean
sccsget:
	${SCCS} get ${DATE} SCCS
sccsinfo:
	@${SCCS} info
tags:
	ctags *.[ch]
lint:
	${LINT} ${LINTFLAGS} *.c

ps_img.o: ps_img.c img_pro.h 
ps_plot.o: ps_plot.c plot_pro.h 
ps_text.o: ps_text.c 
ps_ascii.o: ps_ascii.c ascii_pro.h

#
#	PostScript prolog filter setup
#
img_pro.h: img_pro.ps
	sed -e 's/.*/"&",/' img_pro.ps > img_pro.h
plot_pro.h: plot_pro.ps
	sed -e 's/.*/"&",/' plot_pro.ps > plot_pro.h
ascii_pro.h: ascii_pro.ps
	sed -e 's/.*/"&",/' ascii_pro.ps > ascii_pro.h


#
# @(#)Makefile	1.7 	ULTRIX	1/11/85
#
#
FILES = Readmefirst syscript cmx dskx fsx fsxr lpx memx memxr mtx netx \
	shmx shmxb showsnap shoadrs 8600README

DIAGO = cmx.o dskx.o fsx.o fsxr.o lpx.o memx.o memxr.o mtx.o netx.o \
	shmx.o shmxb.o diaglib.o

DIAG = cmx dskx fsx fsxr lpx memx memxr mtx netx shmx shmxb

SHOWSNAP = csl.c emm.c esc.c ipr.c isp.c mstr.c pamm.c sbi.c uvsr.c

all: diags showsnap shoadrs

diags: $(DIAG)

$(DIAG): $(DIAGO)
	${CC} -O $@.o -lerrlog diaglib.o -o $@

$(DIAGO): diag.h

showsnap: ${SHOWSNAP}
	cc -O -o showsnap ${SHOWSNAP}

shoadrs: shoadrs.c
	cc -O -o shoadrs shoadrs.c

sccsget:
	sccs get SCCS

sccsinfo:
	sccs info
clean:

clobber: clean
	rm -f Makefile makefile.rdt Readmefirst \
	cmx* diag* dskx* fsx* lpx* memx* mtx* netx* shmx* showsnap shoadrs

install: all
	-if [ ! -d ${DESTROOT}/usr/field ]; then \
		mkdir ${DESTROOT}/usr/field; \
		chmod 755 ${DESTROOT}/usr/field; \
	fi; \
	for i in ${FILES}; do (install -c -m 744 $$i ${DESTROOT}/usr/field/$$i); done
	cp ../root/.cshrc ${DESTROOT}/usr/field
	cp ../root/.login ${DESTROOT}/usr/field
	install -c -m 440 makefile.rdt ${DESTROOT}/usr/field/Makefile

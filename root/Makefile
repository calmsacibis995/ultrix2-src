#	@(#)Makefile	1.13	Ultrix	11/30/84
#
# This Makefiles installs the . files that live in the root of the 
# distribution. 
#  

# DESTROOT is the directory below which the distribution is built.

DESTROOT=/Ultrix

FILES= .login .cshrc .profile .rhosts 

VERSION= 1.0

all: ${FILES}

clean: 
	@echo done

clobber:
	rm -f ${FILES} Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

install: ${FILES}
	cp ${FILES} ${DESTROOT}/.
	( cd ${DESTROOT}/.; chmod 744 ${FILES} )
save_version:
	@sccs get -s SCCS
	@sccs what ${FILES} Makefile | awk '$$2 != "" {print $$1, $$2}' > SCCS/version_${VERSION}

restore_version:
	@echo ../library/root:
	@for i in ${FILES} Makefile; do \
		echo "Restore" `grep $$i SCCS/version_${VERSION}`; \
		sccs get -s -r`grep $$i SCCS/version_${VERSION} | awk '{print $$2, $$1}'`; \
	done
show_current:
	@echo ../library/root:
	@sccs what ${FILES} Makefile | awk '$$2 != "" {printf "%s  %s\t", $$1, $$2}'
	@echo 
.DEFAULT:
	sccs get $<

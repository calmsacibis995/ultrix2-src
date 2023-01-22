# @(#)Makefile	1.2	ULTRIX	1/15/85
#

FILESX = .cshrc .login 

FILES = .mailrc .profile .project

all:



install:
	for i in ${FILES}; do \
		(install -c -m 744 $$i ${DESTROOT}/usr/skel/$$i); done

	for i in ${FILESX}; do \
		(install -c -m 755 $$i ${DESTROOT}/usr/skel/$$i); done
clean:


sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

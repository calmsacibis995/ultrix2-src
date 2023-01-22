# @(#)Makefile	1.5	ULTRIX	12/4/84
#
# @(#)Makefile	1.10	(Berkeley) 7/1/83
#
# reorder gives an editor command for most common terminals
# (in reverse order from n'th to 1'st most commonly used)
# to move them to the front of termcap
#
DESTROOT =

termcap: reorder termcap.src
	ex - termcap.src < reorder

install: termcap tabset
	install -c -m 664 termcap ${DESTROOT}/etc/termcap

tabset:
	rm -fr ${DESTROOT}/usr/lib/tabset
	mkdir ${DESTROOT}/usr/lib/tabset
	cp tabset/* ${DESTROOT}/usr/lib/tabset

clean:
	rm -f termcap

clobber: clean
	rm -f termcap.src reorder
	(cd tabset; rm -f [a-z]*)

sccsinfo:
	sccs info
	(cd tabset; sccs info)

sccsget:
	sccs get SCCS
	(cd tabset; sccs get SCCS)

.DEFAULT:
	sccs get $<

installall:
	cp termcap /etc/termcap
	@for i in arpa ernie kim dali calder ucbvax miro;\
		do echo $$i;\
		rcp termcap $$i':'/etc/termcap;\
		done

tabsetall: tabset
	@for i in arpa ernie kim dali calder ucbvax miro;\
		do echo $$i;\
		rsh $$i /bin/rm -fr /usr/lib/tabset;\
		rcp -r tabset $$i':'/usr/lib;\
		done

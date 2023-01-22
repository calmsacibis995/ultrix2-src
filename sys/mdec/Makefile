#	@(#)Makefile	1.13	Ultrix	6/28/85
#
DESTROOT=
ALL=	hkboot hpboot htboot mtboot noboot raboot rlboot csboot\
	upboot utboot tmboot tsboot tuboot installboot
ALL1=	httoggle mttoggle tmtoggle tstoggle uttoggle installboot
MALL=   rqboot

all: ${ALL} ${MALL} ${ALL1}

hkboot: hkboot.s
	as hkboot.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=b.out
	dd if=b.out of=hkboot conv=sync

hpboot: hpboot.s
	as hpboot.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=b.out
	dd if=b.out of=hpboot conv=sync

htboot: htboot.s
	as htboot.s
	nm -u a.out
	strip a.out
	dd if=a.out of=b.out bs=32 skip=1
	dd if=b.out of=htboot conv=sync

mtboot: mtboot.s
	as mtboot.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=b.out
	dd if=b.out of=mtboot conv=sync

raboot: raboot.s
	as raboot.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=b.out
	dd if=b.out of=raboot conv=sync
csboot: csboot.s
	as csboot.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=b.out
	dd if=b.out of=csboot conv=sync

rqboot: rqboot.s
	as rqboot.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=b.out
	dd if=b.out of=rqboot conv=sync

rlboot: rlboot.s
	as rlboot.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=b.out
	dd if=b.out of=rlboot conv=sync

tmboot: tmboot.s
	as tmboot.s
	nm -u a.out
	strip a.out
	dd if=a.out of=b.out bs=32 skip=1
	dd if=b.out of=tmboot conv=sync

tsboot: tsboot.s
	as tsboot.s
	nm -u a.out
	strip a.out
	dd if=a.out of=b.out bs=32 skip=1
	dd if=b.out of=tsboot conv=sync

upboot: upboot.s
	as upboot.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=b.out
	dd if=b.out of=upboot conv=sync

tuboot: tuboot.s
	as tuboot.s
	nm -u a.out
	strip a.out
	dd if=a.out of=b.out bs=32 skip=1
	dd if=b.out of=tuboot conv=sync

utboot: utboot.s
	as utboot.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=b.out
	dd if=b.out of=utboot conv=sync

httoggle: httoggle.s
	as httoggle.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=httoggle

mttoggle: mttoggle.s
	as mttoggle.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=mttoggle

tmtoggle: tmtoggle.s
	as tmtoggle.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=tmtoggle

tstoggle: tstoggle.s
	as tstoggle.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=tstoggle

uttoggle: uttoggle.s
	as uttoggle.s
	nm -u a.out
	strip a.out
	dd if=a.out bs=32 skip=1 of=uttoggle

noboot:
	echo | dd of=noboot conv=sync
installboot:
	cc -O installboot.c -o installboot

clean:
	rm -f a.out b.out ${ALL} ${MALL} ${ALL1}

install32m: ${MALL}
	-if [ ! -d ${DESTROOT}/usr/mdec ]; \
	then \
		mkdir ${DESTROOT}/usr/mdec; \
		/etc/chown root ${DESTROOT}/usr/mdec; \
	else \
		true; \
	fi
	install -c rqboot ${DESTROOT}/usr/mdec/raboot
	

install: ${ALL} ${ALL1}
	-if [ ! -d ${DESTROOT}/usr/mdec ]; \
	then \
		mkdir ${DESTROOT}/usr/mdec; \
		/etc/chown root ${DESTROOT}/usr/mdec; \
	else \
		true; \
	fi
	for i in ${ALL}; do \
	install -c $$i ${DESTROOT}/usr/mdec/$$i; done
	rm -f ${DESTROOT}/usr/mdec/mboot
	ln ${DESTROOT}/usr/mdec/htboot ${DESTROOT}/usr/mdec/mboot
	rm -f ${DESTROOT}/usr/mdec/uboot
	ln ${DESTROOT}/usr/mdec/hpboot ${DESTROOT}/usr/mdec/uboot
	install -c Makefile ${DESTROOT}/usr/mdec/Makefile

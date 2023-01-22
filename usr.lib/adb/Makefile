#
#  @(#)Makefile	1.2	ULTRIX	10/3/86
#

FILES = buf callout callout.next clist clist.nxt dino dir \
	dir.nxt dirblk file file.nxt filsys findproc findproc.nxt \
	host.nxt hosts hosts.nxt hosttable ifnet ifuba imp inode \
	inpcb iovec ipreass ipreass.nxt mact mact.nxt 

FILES2 =	mbstat mbuf mbuf.nxt mbufs mbufs.nxt mount nl nu oimp \
	pcb proc protosw rawcb rtentry rusage setproc setproc.done \
	setproc.nxt socket stacktrace stat tcpcb tcpip tcpreass \
	tcpreass.nxt text traceall traceall.nxt tty u u.all \
	ubahd ucred uio un vtimes

all:

clean:
	
clobber: clean
	sccs clean

sccsget:
	sccs get SCCS

install: all 
	for i in ${FILES} ${FILES2}; do (install -c $$i ${DESTROOT}/usr/lib/adb/$$i); done

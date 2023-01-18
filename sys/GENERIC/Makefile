# 2.10BSD kernel Makefile

# OV should be -V for overlaid systems, empty otherwise
OV=-V

H=	../h
I=	/usr/include
M=	../machine
N1=	../net
N2=	../netimp
N3=	../netinet
N4=	../netns
N5=	../netpup
N6=	../vaxif
S=	../sys
U=	../pdpuba
MB=	../pdpmba

DEFS=	-DKERNEL -Dpdp11 -I. -I${H}
CFLAGS=	-O ${DEFS}
CPP=	/lib/cpp -P ${DEFS}
AS=	/bin/as ${OV}
ED=	/bin/ed
SUB=	sys pdp pdpuba pdpmba
NSUB=	net ${SUB}
OFILE=	unix

# Network, separate I/D kernel overlay structure.
NSBASE=	af.o cons.o dh.o dhu.o dr.o dz.o hk.o ht.o init_sysent.o	\
	kern_clock.o kern_descrip.o kern_mman.o kern_proc.o kern_subr.o	\
	kern_synch.o lp.o machdep.o mbuf11.o ram.o ra.o rk.o rl.o si.o	\
	subr_rmap.o sys_inode.o tty.o tty_conf.o tty_subr.o tty_tb.o	\
	ufs_alloc.o ufs_bio.o ufs_dsort.o ufs_fio.o ufs_inode.o		\
	ufs_namei.o uipc_mbuf.o vm_proc.o vm_sched.o vm_swap.o vm_swp.o	\
	vm_text.o rx.o xp.o
NSOV1=	kern_prot.o sys_generic.o sys_inode2.o ufs_syscalls.o
NSOV2=	kern_acct.o kern_exec.o kern_exit.o kern_fork.o kern_resrce.o
NSOV3=	clock.o init_main.o kern_pdp.o kern_rtp.o kern_time.o		\
	kern_xxx.o machdep2.o quota_sys.o subr_prf.o sys_process.o	\
	syscalls.o ufs_mount.o ufs_subr.o
NSOV4=	dkbad.o kern_sig.o mem.o subr_xxx.o tm.o trap.o ts.o tty_pty.o	\
	tty_tty.o
NSOV5=	in_proto.o ip_icmp.o sys_pipe.o ufs_bmap.o uipc_domain.o	\
	uipc_sys.o
NSOV6=	uipc_socket.o uipc_socket2.o
NSOV7=	if_ether.o if_il.o if_qe.o if_sri.o if_uba.o if_vv.o
NSOV8=	if.o in.o in_pcb.o route.o sys_socket.o
NSOV9=	ip_input.o ip_output.o
NSOV10=	tcp_debug.o tcp_input.o tcp_usrreq.o
NSOV11=	raw_cb.o raw_ip.o raw_usrreq.o tcp_output.o tcp_subr.o tcp_timer.o
NSOV12=	udp_usrreq.o uipc_usrreq.o
NSOV13=	if_loop.o if_sl.o

# Non-network, but separate I/D kernel overlay structure.
SBASE=	dh.o dhu.o dkbad.o dr.o dz.o hk.o init_sysent.o kern_clock.o	\
	kern_descrip.o kern_mman.o kern_proc.o kern_sig.o kern_subr.o	\
	kern_synch.o lp.o machdep.o ra.o ram.o rk.o rl.o rx.o si.o	\
	subr_rmap.o subr_xxx.o sys_inode.o sys_pipe.o trap.o tty.o	\
	tty_conf.o tty_subr.o tty_tb.o tty_tty.o ufs_alloc.o		\
	ufs_bio.o ufs_bmap.o ufs_dsort.o ufs_fio.o ufs_inode.o		\
	ufs_namei.o ufs_subr.o vm_proc.o vm_sched.o vm_swap.o vm_swp.o	\
	vm_text.o xp.o mem.o
SOV1=	kern_prot.o sys_generic.o sys_inode2.o ufs_syscalls.o
SOV2=	kern_acct.o kern_exec.o kern_exit.o kern_fork.o kern_resrce.o
SOV3=	clock.o cons.o init_main.o kern_pdp.o kern_rtp.o kern_time.o	\
	kern_xxx.o machdep2.o quota_sys.o subr_prf.o sys_process.o	\
	syscalls.o ufs_mount.o
SOV4=	tty_pty.o
SOV5=	ht.o tm.o ts.o
SOV6=	
SOV7=	
SOV8=	
SOV9=	
SOV10=	
SOV11=	
SOV12=	
SOV13=	
SOV14=	
SOV15=	

# Non-network, non-separate I/D kernel overlay structure.
BASE=	
OV1=	
OV2=	
OV3=	
OV4=	
OV5=	
OV6=	
OV7=	
OV8=	
OV9=	
OV10=	
OV11=	
OV12=	
OV13=	
OV14=	
OV15=	

# scb.o MUST be first
# boot.o must be loaded in the base well above 0 data
CONFOBJ=scb.o libc_bcmp.o libc_bcopy.o libc_bzero.o libc_csv.o		\
	libc_ffs.o libc_htonl.o libc_htons.o libc_insque.o libc_ldiv.o	\
	libc_lmul.o libc_lrem.o libc_remque.o libc_strlen.o		\
	mch_backup.o mch_click.o mch_copy.o mch_dump.o mch_dzpdma.o	\
	mch_profile.o mch_start.o mch_trap.o mch_vars.o mch_xxx.o	\
	conf.o ioconf.o boot.o

NCONFOBJ=${CONFOBJ} in_cksum.o

# locore.o MUST be loaded first, param.o MUST be loaded last.
# enable34.o must be in the base segment for overlaid kernels.
# Don't forget the "-i" flag for split I/D, use "-n" otherwise.

all:
	@echo "Make:"
	@echo "  nsunix - networking, separate I/D kernel"
	@echo "  sunix  - non-networking, separate I/D kernel"
	@echo "  unix   - non-networking, non-separate I/D kernel"

# Networking, separate I/D kernel
nsunix: ${NSUB} ${CONFOBJ} param.o newvers checksys
	ld -X -i -o ${OFILE} ${NCONFOBJ} \
		-Z ${NSOV1}  -Z ${NSOV2}  -Z ${NSOV3}  -Z ${NSOV4} \
		-Z ${NSOV5}  -Z ${NSOV6}  -Z ${NSOV7}  -Z ${NSOV8} \
		-Z ${NSOV9}  -Z ${NSOV10} -Z ${NSOV11} -Z ${NSOV12} \
		-Z ${NSOV13} -Z ${NSOV14} -Z ${NSOV15} \
		-Y ${NSBASE} enable34.o vers.o param.o
	size ${OFILE}
	./checksys ${OFILE}

# Non-networking, but separate I/D kernel
sunix: ${SUB} ${CONFOBJ} param.o newvers checksys
	ld -X -i -o ${OFILE} ${CONFOBJ} \
		-Z ${SOV1}  -Z ${SOV2}  -Z ${SOV3}  -Z ${SOV4} \
		-Z ${SOV5}  -Z ${SOV6}  -Z ${SOV7}  -Z ${SOV8} \
		-Z ${SOV9}  -Z ${SOV10} -Z ${SOV11} -Z ${SOV12} \
		-Z ${SOV13} -Z ${SOV14} -Z ${SOV15} \
		-Y ${SBASE} enable34.o vers.o param.o
	size ${OFILE}
	./checksys ${OFILE}

# Non-networking, non-separate I/D kernel
unix: ${SUB} ${CONFOBJ} param.o newvers checksys
	ld -X -n -o ${OFILE} ${CONFOBJ} \
		-Z ${OV1}  -Z ${OV2}  -Z ${OV3}  -Z ${OV4} \
		-Z ${OV5}  -Z ${OV6}  -Z ${OV7}  -Z ${OV8} \
		-Z ${OV9}  -Z ${OV10} -Z ${OV11} -Z ${OV12} \
		-Z ${OV13} -Z ${OV14} -Z ${OV15} \
		-Y ${BASE} enable34.o vers.o param.o
	size ${OFILE}
	./checksys ${OFILE}

newvers:
	sh ../conf/newvers.sh
	${CC} -c vers.c

boot.o:	boot.s
	${CPP} boot.s > boot.i
	${AS} -o boot.o boot.i
	rm -f boot.i

param.o:
	${CC} -S ${CFLAGS} param.c
	ed - param.s < ../conf/:comm-to-bss
	${AS} -o param.o param.s
	rm -f param.s

ioconf.o: ioconf.c
	${CC} -c ${CFLAGS} ioconf.c

checksys: checksys.o
	${CC} checksys.o -o $@

checksys.o: ../conf/checksys.c
	${CC} -c ${CFLAGS} ../conf/checksys.c

depend:
	for i in ${NSUB}; do \
		make -f Make.$$i CFLAGS="${CFLAGS}" depend; done
	mkdep ${CFLAGS} param.c ioconf.c ../conf/checksys.c

TAGSFILE= ${S}/tags
TLIST=	${H} ${I} ${N1} ${N2} ${N3} ${N4} ${N5} ${N6} ${M} ${S} ${U} ${MB}

tags: FRC
	rm -f ${TAGSFILE}
	for i in ${TLIST}; do \
		(cd ${S}; /usr/ucb/ctags -a -t $$i/*.[ch]); done
	/usr/bin/egrep "^ASENTRY|^ENTRY" ${M}/*.s | \
	    /bin/sed 's;\([^:]*\):\([^(]*\)(\([^)]*\));\3	\1	/^\2(\3)$$/;' \
	    >> ${TAGSFILE}
	/usr/bin/sort -o ${TAGSFILE} ${TAGSFILE}

# rule to build all system objects
sys pdp pdpuba pdpmba net:
	make -f Make.$@ I=${I} H=${H} M=${M} AS="${AS}" CPP="${CPP}" \
	   CFLAGS="${CFLAGS}" ED="${ED}"

clean:
	rm -f ${OFILE} core mklog *.o *.i assym.h genassym checksys

install:
	install -o root -g staff -m 744 ${OFILE} ${DESTDIR}/${OFILE}

FRC:

# DO NOT DELETE THIS LINE


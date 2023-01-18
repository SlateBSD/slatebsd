# VAX kernel makefile

CC=	../conf/VAX.compile/CC
H=	../h
I=	../../../include
FLAGS=	-DBSD2_10 -DKERNEL -Dpdp11 -I. -I${H} -I${I}
CFLAGS=	-O ${FLAGS}
VPATH=	../pdpuba:../machine:../sys:../net:../netimp:../netinet:../netpup:../vaxif:../pdpmba:../netns

D=	../pdpuba
DFILES=	${D}/dh.c ${D}/dhu.c ${D}/dkbad.c ${D}/dr.c ${D}/dz.c ${D}/hk.c	\
	${D}/ht.c ${D}/lp.c ${D}/ra.c ${D}/rk.c ${D}/rl.c ${D}/rx.c	\
	${D}/si.c ${D}/tm.c ${D}/ts.c ${D}/xp.c
DOBJS=	dh.o dhu.o dkbad.o dr.o dz.o hk.o ht.o lp.o ra.o rk.o rl.o rx.o	\
	si.o tm.o ts.o xp.o

MB=	../pdpmba
MBFILES=
MBOBJS=

M=	../machine
MFILES=	${M}/clock.c ${M}/conf.c ${M}/cons.c ${M}/kern_pdp.c		\
	${M}/machdep.c ${M}/machdep2.c ${M}/mem.c ${M}/trap.c
MOBJS=	clock.o conf.o cons.o kern_pdp.o machdep.o machdep2.o mem.o	\
	trap.o

S=	../sys
SFILES=	${S}/init_main.c ${S}/init_sysent.c ${S}/kern_acct.c		\
	${S}/kern_clock.c ${S}/kern_descrip.c ${S}/kern_exec.c		\
	${S}/kern_exit.c ${S}/kern_fork.c ${S}/kern_mman.c		\
	${S}/kern_proc.c ${S}/kern_prot.c ${S}/kern_resrce.c		\
	${S}/kern_rtp.c ${S}/kern_sig.c ${S}/kern_subr.c		\
	${S}/kern_synch.c ${S}/kern_time.c ${S}/kern_xxx.c		\
	${S}/mbuf11.c ${S}/quota_sys.c ${S}/sys_socket.c		\
	${S}/uipc_socket2.c ${S}/subr_prf.c ${S}/subr_rmap.c		\
	${S}/subr_xxx.c ${S}/sys_generic.c ${S}/sys_inode.c		\
	${S}/sys_inode2.c ${S}/sys_pipe.c ${S}/sys_process.c		\
	${S}/syscalls.c ${S}/tty.c ${S}/tty_conf.c ${S}/tty_pty.c	\
	${S}/tty_subr.c ${S}/tty_tb.c ${S}/tty_tty.c ${S}/ufs_alloc.c	\
	${S}/ufs_bio.c ${S}/ufs_bmap.c ${S}/ufs_dsort.c ${S}/ufs_fio.c	\
	${S}/ufs_inode.c ${S}/ufs_mount.c ${S}/ufs_namei.c		\
	${S}/ufs_subr.c ${S}/ufs_syscalls.c ${S}/uipc_domain.c		\
	${S}/uipc_mbuf.c ${S}/uipc_socket.c ${S}/uipc_sys.c		\
	${S}/uipc_usrreq.c ${S}/vm_proc.c ${S}/vm_sched.c		\
	${S}/vm_swap.c ${S}/vm_swp.c ${S}/vm_text.c
SOBJS=	init_main.o init_sysent.o kern_acct.o kern_clock.o		\
	kern_descrip.o kern_exec.o kern_exit.o kern_fork.o kern_mman.o	\
	kern_proc.o kern_prot.o kern_resrce.o kern_rtp.o kern_sig.o	\
	kern_subr.o kern_synch.o kern_time.o kern_xxx.o mbuf11.o	\
	quota_sys.o sys_socket.o uipc_socket2.o subr_prf.o subr_rmap.o	\
	subr_xxx.o sys_generic.o sys_inode.o sys_inode2.o sys_pipe.o	\
	sys_process.o syscalls.o tty.o tty_conf.o tty_pty.o tty_subr.o	\
	tty_tb.o tty_tty.o ufs_alloc.o ufs_bio.o ufs_bmap.o ufs_dsort.o	\
	ufs_fio.o ufs_inode.o ufs_mount.o ufs_namei.o ufs_subr.o	\
	ufs_syscalls.o uipc_domain.o uipc_mbuf.o uipc_socket.o		\
	uipc_sys.o uipc_usrreq.o vm_proc.o vm_sched.o vm_swap.o		\
	vm_swp.o vm_text.o

N1=	../net
N1C=	${N1}/af.c ${N1}/if.c ${N1}/if_loop.c ${N1}/if_sl.c		\
	${N1}/raw_cb.c ${N1}/raw_usrreq.c ${N1}/route.c
N1O=	af.o if.o if_loop.o if_sl.o raw_cb.o raw_usrreq.o route.o

N2=	../netimp
N2C=	${N2}/if_imp.c ${N2}/if_imphost.c ${N2}/raw_imp.c
N2O=	if_imp.o if_imphost.o raw_imp.o

N3=	../netinet
N3C=	${N3}/if_ether.c ${N3}/in.c ${N3}/in_pcb.c ${N3}/in_proto.c	\
	${N3}/ip_icmp.c ${N3}/ip_input.c ${N3}/ip_output.c		\
	${N3}/raw_ip.c ${N3}/tcp_debug.c ${N3}/tcp_input.c		\
	${N3}/tcp_output.c ${N3}/tcp_subr.c ${N3}/tcp_timer.c		\
	${N3}/tcp_usrreq.c ${N3}/udp_usrreq.c
N3O=	if_ether.o in.o in_pcb.o in_proto.o ip_icmp.o ip_input.o	\
	ip_output.o raw_ip.o tcp_debug.o tcp_input.o tcp_output.o	\
	tcp_subr.o tcp_timer.o tcp_usrreq.o udp_usrreq.o

N4=	../netpup
N4C=	${N4}/pup.c ${N4}/raw_pup.c
N4O=	pup.o raw_pup.o

N5=	../netns
N5C=	${N5}/idp_usrreq.c ${N5}/ns.c ${N5}/ns_pcb.c ${N5}/ns_proto.c	\
	${N5}/ns_error.c ${N5}/ns_ip.c ${N5}/ns_input.c			\
	${N5}/ns_output.c ${N5}/spp_debug.c ${N5}/spp_usrreq.c
N5O=	ns.o ns_pcb.o ns_proto.o ns_error.o ns_ip.o ns_input.o		\
	ns_output.o spp_debug.o spp_usrreq.o idp_usrreq.o

N6=	../vaxif
N6C=	${N6}/if_acc.c ${N6}/if_css.c ${N6}/if_de.c ${N6}/if_dmc.c	\
	${N6}/if_ec.c ${N6}/if_en.c ${N6}/if_il.c ${N6}/if_qe.c		\
	${N6}/if_sri.c ${N6}/if_uba.c ${N6}/if_vv.c
N6O=	if_acc.o if_css.o if_de.o if_dmc.o if_ec.o if_en.o if_il.o	\
	if_qe.o if_sri.o if_uba.o if_vv.o

CONFOBJ=ioconf.o VAX.o

CFILES=	${DFILES} ${MBFILES} ${SFILES} ${MFILES} ${N1C} ${N3C} ${N6C}

OBJS=	${CONFOBJ} ${SOBJS} ${DOBJS} ${MOBJS} ${MBOBJS} \
	${N1O} ${N3O} ${N6O}

unix: genassym checksys ${OBJS} param.o newvers
	${CC} -o $@ ${OBJS} vers.o param.o
	size $@

lint:
	lint -chapbx ${FLAGS} ${CFILES} | \
		sed -e '/warning: illegal pointer combination/d' \
		-e '/warning: possible pointer alignment problem/d' \
		-e '1d'

newvers:
	sh ../conf/newvers.sh
	${CC} -c vers.c

param.h: ${H}/signal.h ${H}/psw.h ${H}/types.h
	touch ${H}/param.h

in_cksum.o: ${N1}/in_cksum.c ${N1}/cksum-fix ${H}/param.h ${H}/types.h \
	${H}/mbuf.h ${N3}/in.h ${N3}/in_systm.h
	${CC} -c ${CFLAGS} ${N1}/in_cksum.c

checksys: checksys.o
	${CC} checksys.o -o $@

checksys.o: ../conf/checksys.c
	${CC} -O -c -I${H} -I${I} ../conf/checksys.c

genassym: genassym.o
	${CC} genassym.o -o $@

genassym.o: ${M}/genassym.c
	${CC} -c ${CFLAGS} ${M}/genassym.c

depend: 
	../conf/VAX.compile/mkdep ${CFLAGS} ${CFILES}

INC1=	${N1}/*.h ${N2}/*.h ${N3}/*.h ${N4}/*.h ${N5}/*.h ${N6}/*.h
INC2=	${D}/*.h ${MB}/*.h ${M}/*.h ${H}/*.h ${I}/*.h

tags: FRC
	-(cd ${S};/usr/ucb/ctags ${CFILES})
	-(cd ${S};/usr/ucb/ctags -a ${INC1})
	-(cd ${S};/usr/ucb/ctags -a ${INC2})
	-/usr/bin/egrep "^ASENTRY|^ENTRY" ${M}/*.s | \
	    /bin/sed 's;\([^:]*\):\([^(]*\)(\([^)]*\));\3	\1	/^\2(\3)$$/;' \
	    >> ${S}/tags
	/usr/bin/sort -o ${S}/tags ${S}/tags

clean:
	rm -f core mklog *.o *.i assym.h genassym checksys

FRC:

# DO NOT DELETE THIS LINE


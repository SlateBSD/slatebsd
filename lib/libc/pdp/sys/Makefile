#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.6 (Berkeley) 1/25/87
#
SRCS=	_exit.s accept.s access.s acct.s adjtime.s bind.s brk.s chdir.s \
	chmod.s chown.s chroot.s close.s connect.s creat.s dup.s dup2.s \
	execl.s execle.s execv.s execve.s fchmod.s fchown.s fcntl.s flock.s \
	fork.s fstat.s fsync.s ftruncate.s getdopt.s getdtablesiz.s \
	getegid.s geteuid.s getgid.s getgroups.s gethostid.s gethostname.s \
	getitimer.s getpagesize.s getpeername.s getpgrp.s getpid.s getppid.s \
	getpriority.s getrlimit.s getrusage.s getsockname.s getsockopt.s \
	gettimeofday.s getuid.s ioctl.s kill.s killpg.s link.s listen.s \
	lseek.s lstat.s mkdir.s mknod.s mount.s open.s pipe.s profil.s \
	ptrace.s quota.s read.s readlink.s reboot.s recv.s recvfrom.s \
	recvmsg.s rename.s rmdir.s sbrk.s select.s send.s sendmsg.s sendto.s \
	setdopt.s setgroups.s sethostid.s sethostname.s setitimer.s \
	setpgrp.s setpriority.s setquota.s setregid.s setreuid.s setrlimit.s \
	setsockopt.s settimeofday.s shutdown.s sigblock.s sigpause.s \
	sigreturn.s sigsetmask.s sigstack.s sigvec.s socket.s socketpair.s \
	stat.s symlink.s sync.s truncate.s umask.s umount.s unlink.s \
	utimes.s vfork.s vhangup.s wait.s wait3.s write.s
OBJS=	_exit.o accept.o access.o acct.o adjtime.o bind.o brk.o chdir.o \
	chmod.o chown.o chroot.o close.o connect.o creat.o dup.o dup2.o \
	execl.o execle.o execv.o execve.o fchmod.o fchown.o fcntl.o flock.o \
	fork.o fstat.o fsync.o ftruncate.o getdopt.o getdtablesiz.o \
	getegid.o geteuid.o getgid.o getgroups.o gethostid.o gethostname.o \
	getitimer.o getpagesize.o getpeername.o getpgrp.o getpid.o getppid.o \
	getpriority.o getrlimit.o getrusage.o getsockname.o getsockopt.o \
	gettimeofday.o getuid.o ioctl.o kill.o killpg.o link.o listen.o \
	lseek.o lstat.o mkdir.o mknod.o mount.o open.o pipe.o profil.o \
	ptrace.o quota.o read.o readlink.o reboot.o recv.o recvfrom.o \
	recvmsg.o rename.o rmdir.o sbrk.o select.o send.o sendmsg.o sendto.o \
	setdopt.o setgroups.o sethostid.o sethostname.o setitimer.o \
	setpgrp.o setpriority.o setquota.o setregid.o setreuid.o setrlimit.o \
	setsockopt.o settimeofday.o shutdown.o sigblock.o sigpause.o \
	sigreturn.o sigsetmask.o sigstack.o sigvec.o socket.o socketpair.o \
	stat.o symlink.o sync.o truncate.o umask.o umount.o unlink.o \
	utimes.o vfork.o vhangup.o wait.o wait3.o write.o
CFLAGS=	-O ${DEFS}
TAGSFILE=tags

.s.o:
	/lib/cpp -E -DPROF ${DEFS} $*.s | sed -e 's;^#;/;' > x.s
	as -V -o $*.o x.s
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	/lib/cpp -E ${DEFS} $*.s | sed -e 's;^#;/;' > x.s
	as -V -o $*.o x.s
	-ld -x -r $*.o
	mv a.out $*.o
	rm -f x.s

.c.o:
	cc -p -c ${CFLAGS} $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	cc -c ${CFLAGS} $*.c
	-ld -x -r $*.o
	mv a.out $*.o

syslib syslib_p: ${OBJS}
	@echo "building profiled syslib"
	@cd profiled; ar cru ../syslib_p ${OBJS}
	@echo "building normal syslib"
	@ar cru syslib ${OBJS}

tags:

clean:
	rm -f *.o profiled/*.o errs a.out core syslib syslib_p \
		tags Makefile.bak

depend:
	for i in ${SRCS}; do \
	    cc -M ${CFLAGS} $$i | awk ' { if ($$1 != prev) \
		{ if (rec != "") print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		else rec = rec " " $$2 } } \
		END { print rec } ' >> makedep; done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

_exit.o: _exit.s ./SYS.h /usr/include/syscall.h
accept.o: accept.s ./SYS.h /usr/include/syscall.h
access.o: access.s ./SYS.h /usr/include/syscall.h
acct.o: acct.s ./SYS.h /usr/include/syscall.h
adjtime.o: adjtime.s ./SYS.h /usr/include/syscall.h
bind.o: bind.s ./SYS.h /usr/include/syscall.h
brk.o: brk.s ./SYS.h /usr/include/syscall.h
chdir.o: chdir.s ./SYS.h /usr/include/syscall.h
chmod.o: chmod.s ./SYS.h /usr/include/syscall.h
chown.o: chown.s ./SYS.h /usr/include/syscall.h
chroot.o: chroot.s ./SYS.h /usr/include/syscall.h
close.o: close.s ./SYS.h /usr/include/syscall.h
connect.o: connect.s ./SYS.h /usr/include/syscall.h
creat.o: creat.s ./SYS.h /usr/include/syscall.h
dup.o: dup.s ./SYS.h /usr/include/syscall.h
dup2.o: dup2.s ./SYS.h /usr/include/syscall.h
execl.o: execl.s ./SYS.h /usr/include/syscall.h
execle.o: execle.s ./SYS.h /usr/include/syscall.h
execv.o: execv.s ./SYS.h /usr/include/syscall.h
execve.o: execve.s ./SYS.h /usr/include/syscall.h
fchmod.o: fchmod.s ./SYS.h /usr/include/syscall.h
fchown.o: fchown.s ./SYS.h /usr/include/syscall.h
fcntl.o: fcntl.s ./SYS.h /usr/include/syscall.h
flock.o: flock.s ./SYS.h /usr/include/syscall.h
fork.o: fork.s ./SYS.h /usr/include/syscall.h
fstat.o: fstat.s ./SYS.h /usr/include/syscall.h
fsync.o: fsync.s ./SYS.h /usr/include/syscall.h
ftruncate.o: ftruncate.s ./SYS.h /usr/include/syscall.h
getdopt.o: getdopt.s ./SYS.h /usr/include/syscall.h
getdtablesiz.o: getdtablesiz.s ./SYS.h /usr/include/syscall.h
getegid.o: getegid.s ./SYS.h /usr/include/syscall.h
geteuid.o: geteuid.s ./SYS.h /usr/include/syscall.h
getgid.o: getgid.s ./SYS.h /usr/include/syscall.h
getgroups.o: getgroups.s ./SYS.h /usr/include/syscall.h
gethostid.o: gethostid.s ./SYS.h /usr/include/syscall.h
gethostname.o: gethostname.s ./SYS.h /usr/include/syscall.h
getitimer.o: getitimer.s ./SYS.h /usr/include/syscall.h
getpagesize.o: getpagesize.s ./SYS.h /usr/include/syscall.h
getpeername.o: getpeername.s ./SYS.h /usr/include/syscall.h
getpgrp.o: getpgrp.s ./SYS.h /usr/include/syscall.h
getpid.o: getpid.s ./SYS.h /usr/include/syscall.h
getppid.o: getppid.s ./SYS.h /usr/include/syscall.h
getpriority.o: getpriority.s ./SYS.h /usr/include/syscall.h
getrlimit.o: getrlimit.s ./SYS.h /usr/include/syscall.h
getrusage.o: getrusage.s ./SYS.h /usr/include/syscall.h
getsockname.o: getsockname.s ./SYS.h /usr/include/syscall.h
getsockopt.o: getsockopt.s ./SYS.h /usr/include/syscall.h
gettimeofday.o: gettimeofday.s ./SYS.h /usr/include/syscall.h
getuid.o: getuid.s ./SYS.h /usr/include/syscall.h
ioctl.o: ioctl.s ./SYS.h /usr/include/syscall.h
kill.o: kill.s ./SYS.h /usr/include/syscall.h
killpg.o: killpg.s ./SYS.h /usr/include/syscall.h
link.o: link.s ./SYS.h /usr/include/syscall.h
listen.o: listen.s ./SYS.h /usr/include/syscall.h
lseek.o: lseek.s ./SYS.h /usr/include/syscall.h
lstat.o: lstat.s ./SYS.h /usr/include/syscall.h
mkdir.o: mkdir.s ./SYS.h /usr/include/syscall.h
mknod.o: mknod.s ./SYS.h /usr/include/syscall.h
mount.o: mount.s ./SYS.h /usr/include/syscall.h
open.o: open.s ./SYS.h /usr/include/syscall.h
pipe.o: pipe.s ./SYS.h /usr/include/syscall.h
profil.o: profil.s ./SYS.h /usr/include/syscall.h
ptrace.o: ptrace.s ./SYS.h /usr/include/syscall.h
quota.o: quota.s ./SYS.h /usr/include/syscall.h
read.o: read.s ./SYS.h /usr/include/syscall.h
readlink.o: readlink.s ./SYS.h /usr/include/syscall.h
reboot.o: reboot.s ./SYS.h /usr/include/syscall.h
recv.o: recv.s ./SYS.h /usr/include/syscall.h
recvfrom.o: recvfrom.s ./SYS.h /usr/include/syscall.h
recvmsg.o: recvmsg.s ./SYS.h /usr/include/syscall.h
rename.o: rename.s ./SYS.h /usr/include/syscall.h
rmdir.o: rmdir.s ./SYS.h /usr/include/syscall.h
sbrk.o: sbrk.s ./SYS.h /usr/include/syscall.h
select.o: select.s ./SYS.h /usr/include/syscall.h
send.o: send.s ./SYS.h /usr/include/syscall.h
sendmsg.o: sendmsg.s ./SYS.h /usr/include/syscall.h
sendto.o: sendto.s ./SYS.h /usr/include/syscall.h
setdopt.o: setdopt.s ./SYS.h /usr/include/syscall.h
setgroups.o: setgroups.s ./SYS.h /usr/include/syscall.h
sethostid.o: sethostid.s ./SYS.h /usr/include/syscall.h
sethostname.o: sethostname.s ./SYS.h /usr/include/syscall.h
setitimer.o: setitimer.s ./SYS.h /usr/include/syscall.h
setpgrp.o: setpgrp.s ./SYS.h /usr/include/syscall.h
setpriority.o: setpriority.s ./SYS.h /usr/include/syscall.h
setquota.o: setquota.s ./SYS.h /usr/include/syscall.h
setregid.o: setregid.s ./SYS.h /usr/include/syscall.h
setreuid.o: setreuid.s ./SYS.h /usr/include/syscall.h
setrlimit.o: setrlimit.s ./SYS.h /usr/include/syscall.h
setsockopt.o: setsockopt.s ./SYS.h /usr/include/syscall.h
settimeofday.o: settimeofday.s ./SYS.h /usr/include/syscall.h
shutdown.o: shutdown.s ./SYS.h /usr/include/syscall.h
sigblock.o: sigblock.s ./SYS.h /usr/include/syscall.h
sigpause.o: sigpause.s ./SYS.h /usr/include/syscall.h
sigreturn.o: sigreturn.s ./SYS.h /usr/include/syscall.h
sigsetmask.o: sigsetmask.s ./SYS.h /usr/include/syscall.h
sigstack.o: sigstack.s ./SYS.h /usr/include/syscall.h
sigvec.o: sigvec.s ./SYS.h /usr/include/syscall.h
socket.o: socket.s ./SYS.h /usr/include/syscall.h
socketpair.o: socketpair.s ./SYS.h /usr/include/syscall.h
stat.o: stat.s ./SYS.h /usr/include/syscall.h
symlink.o: symlink.s ./SYS.h /usr/include/syscall.h
sync.o: sync.s ./SYS.h /usr/include/syscall.h
truncate.o: truncate.s ./SYS.h /usr/include/syscall.h
umask.o: umask.s ./SYS.h /usr/include/syscall.h
umount.o: umount.s ./SYS.h /usr/include/syscall.h
unlink.o: unlink.s ./SYS.h /usr/include/syscall.h
utimes.o: utimes.s ./SYS.h /usr/include/syscall.h
vfork.o: vfork.s ./SYS.h /usr/include/syscall.h
vhangup.o: vhangup.s ./SYS.h /usr/include/syscall.h
wait.o: wait.s ./SYS.h /usr/include/syscall.h
wait3.o: wait3.s ./SYS.h /usr/include/syscall.h
write.o: write.s ./SYS.h /usr/include/syscall.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above

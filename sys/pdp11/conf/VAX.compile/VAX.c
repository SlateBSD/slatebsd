/*
 * This file should allow 2.10BSD to compile on a VAX.
 */
#include "../h/param.h"
#include "../machine/seg.h"

#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/file.h"

int	_ovno;
int	icode[1];
int	szicode;
short	cputype;
segm	seg5;
struct buf	*hasmap;
struct user	u;
struct proc	proc[1];
struct file	file[1];
struct text	text[1];

_spl0(){}
_spl1(){}
_spl4(){}
_spl5(){}
_spl6(){}
_spl7(){}
spl0(){}
spl1(){}
spl4(){}
spl5(){}
spl6(){}
spl7(){}

_splnet(){}
_splsoftclock(){}
_spltty(){}
_splbio(){}
_splimp(){}
_splclock(){}
_splhigh(){}
splnet(){}
splsoftclock(){}
spltty(){}
splbio(){}
splimp(){}
splclock(){}
splhigh(){}

caddr_t mapin(){}

_insque(){}
_remque(){}
addupc(){}
backup(){}
bcopy(){}
clear(){}
clrbuf(){}
copy(){}
copyiin(){}
copyin(){}
copyinstr(){}
copyiout(){}
copyout(){}
copyoutstr(){}
copystr(){}
copyu(){}
copyv(){}
doboot(){}
fioword() {}
fmove(){}
fubyte(){}
fuibyte(){}
fuiword(){}
fuword(){}
halt(){}
idle(){}
in_cksum(){}
longjmp(){}
mapout(){}
munmapfd(){}
noop(){}
restfp(){}
restormap(){}
save(){}
savemap(){}
saveregs(){}
savfp(){}
sep_id(){}
setjmp(){}
stst(){}
subyte(){}
suibyte(){}
suiword(){}
suword(){}
vcopyin(){}
vcopyout(){}
waitloc(){}

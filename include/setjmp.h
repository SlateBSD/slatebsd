/*	setjmp.h	4.1	83/05/03	*/

typedef int jmp_buf[10];

#ifdef BSD2_10
#define	longjmperror	_ljerr
#endif BSD2_10

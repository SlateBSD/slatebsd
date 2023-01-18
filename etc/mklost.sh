#!/bin/sh -
#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#	@(#)mklost+found.sh	5.1 (Berkeley) 5/28/85

# The VAX version doesn't work for 2.10 'cause we don't have unlimited
# file names.

#	mkdir lost+found
#	cd lost+found
#	echo creating slots...
#	for i in 1 2 3 4 5 6 7 8 9 0 a b c d e f
#	do
#		cat </dev/null >${i}XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#	done
#	echo removing dummy files...
#	for i in 1 2 3 4 5 6 7 8 9 0 a b c d e f
#	do
#		rm ${i}X*
#	done
#	cd ..
#	echo done
#	ls -ld `pwd`/lost+found

mkdir lost+found
cd lost+found
echo creating slots...
for i in 1 2 3 4 5 6 7 8 9 0 a b c d e
do
	tee ${i}1 ${i}2 ${i}3 ${i}4 ${i}5 ${i}6 ${i}7 ${i}8 < /dev/null
	tee ${i}9 ${i}a ${i}b ${i}c ${i}d ${i}e ${i}f ${i}0 < /dev/null
done

# this is here so that the directory doesn't get truncated
# by the kernel.  It should never be removed.
tee .dont_remove < /dev/null
chmod 400 .dont_remove

echo removing dummy files...
for i in 1 2 3 4 5 6 7 8 9 0 a b c d e
do
	rm ${i}1 ${i}2 ${i}3 ${i}4 ${i}5 ${i}6 ${i}7 ${i}8
	rm ${i}9 ${i}a ${i}b ${i}c ${i}d ${i}e ${i}f ${i}0
done
cd ..
echo done
ls -ld `pwd`/lost+found

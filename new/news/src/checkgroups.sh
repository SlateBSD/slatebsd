: check active file for missing or extra newsgroups
: '@(#)checkgroups	1.17	10/29/86'

if  test  ! -s /usr/lib/news/newsgroups
then
	cp /dev/null /usr/lib/news/newsgroups
fi
# Read first line of stdin.  If of the form "-n group", then only check
# for the specified group.  Otherwise, assume doing standard groups
sed -e '/^[a-zA-Z-]*: /d' -e '/^$/d' -e '/^[#:]/d' | (
read line
case "${line}" in
-n*)
	# Doing specific group.  extract group name and preserve
	# all of current newsgroups file except for that group.
	# Then append entries for this group.
	group=`echo "${line}" | sed -e 's/-n /^/' -e 's/$/\\\\./'`
	egrep -v "${group}" /usr/lib/news/newsgroups > /tmp/$$a
	cat /tmp/$$a - > /usr/lib/news/newsgroups
	;;
*)
	group="^net\\.|^mod\\.|^comp\\.|^sci\\.|^rec\\.|^news\\.|^soc\\.|^misc\\.|^talk\\."
	egrep -v "${group}" /usr/lib/news/newsgroups > /tmp/$$a
	cat /tmp/$$a > /usr/lib/news/newsgroups
	cat >> /usr/lib/news/newsgroups
	;;
esac

egrep "${group}" /usr/lib/news/active | sed 's/ .*//' | sort >/tmp/$$active
egrep "${group}" /usr/lib/news/newsgroups | sed 's/	.*//' | sort >/tmp/$$newsgroups

comm -13 /tmp/$$active /tmp/$$newsgroups >/tmp/$$missing
comm -23 /tmp/$$active /tmp/$$newsgroups >/tmp/$$remove

egrep "${group}" /usr/lib/news/active | sed -n "/m\$/s/ .*//p" |
	sort > /tmp/$$active.mod.all
egrep "${group}" /usr/lib/news/newsgroups |
sed -n "/Moderated/s/[ 	][ 	]*.*//p" | sort > /tmp/$$newsg.mod

comm -12 /tmp/$$missing /tmp/$$newsg.mod >/tmp/$$add.mod
comm -23 /tmp/$$missing /tmp/$$newsg.mod >/tmp/$$add.unmod
cat /tmp/$$add.mod /tmp/$$add.unmod >>/tmp/$$add

comm -23 /tmp/$$active.mod.all /tmp/$$remove >/tmp/$$active.mod
comm -13 /tmp/$$newsg.mod /tmp/$$active.mod >/tmp/$$ismod
comm -23 /tmp/$$newsg.mod /tmp/$$active.mod >/tmp/$$notmod.all
comm -23 /tmp/$$notmod.all /tmp/$$add >/tmp/$$notmod

if test -s /tmp/$$remove
then
	(
	echo "The following newsgroups are not valid and should be removed."
	sed "s/^/	/" /tmp/$$remove
	echo ""
	echo "You can do this by executing the command:"
	echo \	/usr/lib/news/rmgroup `cat /tmp/$$remove`
	echo ""
	) 2>&1 >/tmp/$$out
fi

if test -s /tmp/$$add
then
	(
	echo "The following newsgroups were missing and should be added."
	sed "s/^/	/" /tmp/$$add
	echo ""
	echo "You can do this by executing the command(s):"
	for i in `cat /tmp/$$add.unmod`
	do
		echo '/usr/lib/news/inews -C '$i' </dev/null'
	done
	for i in `cat /tmp/$$add.mod`
	do
		echo '/usr/lib/news/inews -C '$i' moderated </dev/null'
	done
	echo ""
	) 2>&1 >>/tmp/$$out
fi

if test -s /tmp/$$ismod
then
	(
	echo "The following newsgroups are not moderated and are marked moderated."
	sed "s/^/	/" /tmp/$$ismod
	echo ""
	echo "You can correct this by executing the command(s):"
	for i in `cat /tmp/$$ismod`
	do
		echo '/usr/lib/news/inews -C '$i' </dev/null'
	done
	echo ""
	) 2>&1 >>/tmp/$$out
fi

if test -s /tmp/$$notmod
then
	(
	echo "The following newsgroups are moderated and not marked so."
	sed "s/^/	/" /tmp/$$notmod
	echo ""
	echo "You can correct this by executing the command(s):"
	for i in `cat /tmp/$$notmod`
	do
		echo '/usr/lib/news/inews -C '$i' moderated </dev/null'
	done
	echo ""
	) 2>&1 >>/tmp/$$out
fi

if test -s /tmp/$$out
then
	(echo	"Subject: Problems with your active file"
	echo ""
	cat /tmp/$$out
	) | if test $# -gt 0
		then
			mail $1
		else
			cat
		fi	
fi
)

rm -f /tmp/$$*

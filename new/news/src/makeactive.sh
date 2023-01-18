: "Create active file and newsgroup hierarchy for new machine"
: "Usage: sh makeactive.sh LIBDIR SPOOLDIR NEWSUSR NEWSGRP"
: '@(#)makeactive	1.22	10/29/86'
LIBDIR=$1
SPOOLDIR=$2
NEWSUSR=$3
NEWSGRP=$4
cat <<"E_O_F" > /tmp/$$groups
general	Articles that should be read by everyone on your local system
net.ai			Artificial intelligence discussions.
net.announce.arpa-internet	Announcements from the Arpa world
net.arch		Computer architecture.
net.audio		High fidelity audio.
net.auto		Automobiles, automotive products and laws.
net.auto.tech		Technical aspects of automobiles, et. al.
net.aviation		Aviation rules, means, and methods.
net.bicycle		Bicycles, related products and laws.
net.books		Books of all genres, shapes, and sizes.
net.bugs		General bug reports and fixes.
net.bugs.2bsd		Reports of UNIX* version 2BSD related bugs.
net.bugs.4bsd		Reports of UNIX version 4BSD related bugs.
net.bugs.usg		Reports of USG (System III, V, etc.) bugs.
net.bugs.uucp		Reports of UUCP related bugs.
net.bugs.v7		Reports of UNIX V7 related bugs.
net.cog-eng		Cognitive engineering.
net.comics		The funnies, old and new.
net.cooks		Food, cooking, cookbooks, and recipes.
net.cse			Computer science education.
net.cycle		Motorcycles and related products and laws.
net.database		Database and data management issues and theory.
net.dcom		Data communications hardware and software.
net.decus		DEC* Users' Society newsgroup.
net.emacs		EMACS editors of different flavors.
net.eunice		The SRI Eunice system.
net.games		Games and computer games.
net.games.board		Discussion and hints on board games.
net.games.chess		Chess & computer chess.
net.games.emp		Discussion and hints about Empire.
net.games.frp		Discussion about Fantasy Role Playing games.
net.games.go		Discussion about Go.
net.games.hack		Discussion, hints, etc. about the Hack game.
net.games.pbm		Discussion about Play by Mail games.
net.games.rogue		Discussion and hints about Rogue.
net.games.trivia	Discussion about trivia.
net.games.video		Discussion about video games.
net.garden		Gardening, methods and results.
net.graphics		Computer graphics, art, animation, image processing,
net.ham-radio		Amateur Radio practices, contests, events, rules, etc.
net.ham-radio.packet	Discussion about packet radio setups.
net.info-terms		All sorts of terminals.
net.internat		Discussion about international standards
net.jokes		Jokes and the like.  May be somewhat offensive.
net.jokes.d		Discussions on the content of net.jokes articles
net.lan			Local area network hardware and software.
net.lang		Different computer languages.
net.lang.ada		Discussion about Ada*.
net.lang.apl		Discussion about APL.
net.lang.c		Discussion about C.
net.lang.c++		The object-oriented C++ language.
net.lang.f77		Discussion about FORTRAN.
net.lang.forth		Discussion about Forth.
net.lang.lisp		Discussion about LISP.
net.lang.mod2		Discussion about Modula-2.
net.lang.pascal		Discussion about Pascal.
net.lang.prolog		Discussion about PROLOG.
net.lang.st80		Discussion about Smalltalk 80.
net.lsi			Large scale integrated circuits.
net.mag			Magazine summaries, tables of contents, etc.
net.mail		Proposed new mail/network standards.
net.mail.headers	Gatewayed from the ARPA header-people list.
net.micro		Micro computers of all kinds.
net.micro.6809		Discussion about 6809's.
net.micro.68k		Discussion about 68k's.
net.micro.apple		Discussion about Apple micros.
net.micro.amiga		Talk about the new Amiga micro.
net.micro.atari8	Discussion about 8 bit Atari micros.
net.micro.atari16	Discussion about 16 bit Atari micros.
net.micro.att		Discussions about AT&T microcomputers 
net.micro.cbm		Discussion about Commodore micros.
net.micro.cpm		Discussion about the CP/M operating system.
net.micro.hp		Discussion about Hewlett/Packard's.
net.micro.mac		Material about the Apple Macintosh & Lisa
net.micro.ns32k		National Semiconductor 32000 series chips
net.micro.pc		Discussion about IBM personal computers.
net.micro.ti		Discussion about Texas Instruments.
net.micro.trs-80	Discussion about TRS-80's.
net.movies		Reviews and discussions of movies.
net.music		Music lovers' group.
net.music.classical	Discussion about classical music.
net.music.folk		Folks discussing folk music of various sorts
net.music.gdead		A group for (Grateful) Dead-heads
net.music.makers	For performers and their discussions.
net.music.synth		Synthesizers and computer music
net.news		Discussions of USENET itself.
net.news.adm		Comments directed to news administrators.
net.news.b		Discussion about B news software.
net.news.config		Postings of system down times and interruptions.
net.news.group		Discussions and lists of newsgroups
net.news.newsite	Postings of new site announcements.
net.news.notes		Notesfile software from the Univ. of Illinois.
net.news.sa		Comments directed to system administrators.
net.news.stargate	Discussion about satellite transmission of news.
net.periphs		Peripheral devices.
net.pets		Pets, pet care, and household animals in general.
net.poems		For the posting of poems.
net.puzzle		Puzzles, problems, and quizzes.
net.railroad		Real and model train fans' newsgroup.
net.rec			Recreational/participant sports.
net.rec.birds		Hobbyists interested in bird watching.
net.rec.boat		Hobbyists interested in boating.
net.rec.bridge		Hobbyists interested in bridge.
net.rec.nude		Hobbyists interested in naturist/nudist activities.
net.rec.photo		Hobbyists interested in photography.
net.rec.scuba		Hobbyists interested in SCUBA diving.
net.rec.ski		Hobbyists interested in skiing.
net.rec.skydive		Hobbyists interested in skydiving.
net.rec.wood		Hobbyists interested in woodworking.
net.sf-lovers		Science fiction lovers' newsgroup.
net.sources		For the posting of software packages & documentation.
net.sources.bugs	For bug fixes and features discussion
net.sources.d		For any discussion on net.sources postings.
net.sources.games	Postings of recreational software
net.sources.mac		Software for the Apple Macintosh
net.sport		Spectator sports.
net.sport.baseball	Discussion about baseball.
net.sport.football	Discussion about football.
net.sport.hockey	Discussion about hockey.
net.sport.hoops		Discussion about basketball.
net.startrek		Star Trek, the TV show and the movies.
net.text		Text processing.
net.travel		Traveling all over the world.
net.tv			The boob tube, its history, and past and current shows.
net.tv.drwho		Discussion about Dr. Who.
net.tv.soaps		Postings about soap operas.
net.unix		UNIX neophytes group.
net.unix-wizards	Discussions, bug reports, and fixes on and for UNIX.
net.usenix		USENIX Association events and announcements.
net.veg			Vegetarians.
net.video		Video and video components.
net.wines		Wines and spirits.
net.wobegon		"A Prairie Home Companion" radio show discussion.
mod.announce		General announcements of interest to all. (Moderated)
mod.announce.newusers	Explanatory postings for new users. (Moderated)
mod.ai			Discussions about Artificial Intelligence (Moderated)
mod.amiga		Commodore Amiga micros -- info, uses, but no programs. (Moderated)
mod.amiga.binaries	Encoded public domain programs in binary form. (Moderated)
mod.amiga.sources	Public domain software in source code format. (Moderated)
mod.compilers		Discussion about compiler construction, theory, etc. (Moderated)
mod.computers		Discussion about various computers and related. (Moderated)
mod.computers.68k		68000-based systems. (Moderated)
mod.computers.apollo		Apollo computer systems. (Moderated)
mod.computers.masscomp		The Masscomp line of computers. (Moderated)
mod.computers.ibm-pc		The IBM PC, PC-XT, and PC-AT. (Moderated)
mod.computers.laser-printers	Laser printers, hardware and software. (Moderated)
mod.computers.pyramid		Pyramid 90x computers. (Moderated)
mod.computers.ridge		Ridge 32 computers and ROS. (Moderated)
mod.computers.sequent		Sequent systems, (esp. Balance 8000). (Moderated)
mod.computers.sun		Sun "workstation" computers (Moderated)
mod.computers.vax		DEC's VAX* line of computers & VMS. (Moderated)
mod.computers.workstations	Various workstation-type computers. (Moderated)
mod.conferences		Calls for papers and conference announcements. (Moderated)
mod.comp-soc		Discussion on the impact of technology on society. (Moderated)
mod.graphics		Graphics software, hardware, theory, etc. (Moderated)
mod.human-nets		Computer aided communications digest. (Moderated)
mod.legal		Discussions of computers and the law. (Moderated)
mod.mac			Apple Macintosh micros -- info, uses, but no programs. (Moderated)
mod.mac.binaries	Encoded public domain programs in binary form. (Moderated)
mod.mac.sources		Public domain software in source code format. (Moderated)
mod.mag			Discussions on electronicly published "magazines" (Moderated)
mod.mag.otherrealms	Edited science fiction and fantasy "magazine". (Moderated)
mod.map			Various maps, including UUCP maps (Moderated)
mod.movies		Reviews and discussion of movies (Moderated)
mod.music		Reviews and discussion of things musical (Moderated)
mod.music.gaffa		Progressive music discussions (e.g., Kate Bush). (Moderated)
mod.newprod		Announcements of new products of interest to readers (Moderated)
mod.newslists		Postings of news-related statistics and lists (Moderated)
mod.os			Disussions about operating systems and related areas. (Moderated)
mod.os.os9		Discussions about the os9 operating system. (Moderated)
mod.os.unix		Discussion of UNIX* features and bugs. (Moderated)
mod.philosophy		Discussion of philosphical issues and concepts. (Moderated)
mod.philosophy.tech	Technical philosophy: math, science, logic, etc (Moderated)
mod.politics		Discussions on political problems, systems, solutions. (Moderated)
mod.politics.arms-d		Arms discussion digest. (Moderated)
mod.protocols		Various forms and types of FTP protocol discussions. (Moderated)
mod.protocols.appletalk		Applebus hardware & software discussion. (Moderated)
mod.protocols.kermit		Information about the Kermit package. (Moderated)
mod.protocols.tcp-ip		TCP and IP network protocols. (Moderated)
mod.psi			Discussion of paranormal abilities and experiences. (Moderated)
mod.rec			Discussions on pastimes (not currently active) (Moderated)
mod.rec.guns		Discussions about firearms (Moderated)
mod.recipes		A "distributed cookbook" of screened recipes. (Moderated)
mod.religion		Top-level group with no moderator (as of yet). (Moderated)
mod.religion.christian	Discussions on Christianity and related topics. (Moderated)
mod.risks		Risks to the public from computers & users. (Moderated)
mod.sources		postings of public-domain sources. (Moderated)
mod.sources.doc		Archived public-domain documentation. (Moderated)
mod.sources.games	Postings of public-domain game sources (Moderated)
mod.std			Discussion about various standards (Moderated)
mod.std.c		Discussion about C language standards (Moderated)
mod.std.mumps		Discussion for the X11.1 committee on Mumps (Moderated)
mod.std.unix		Discussion for the P1003 committee on UNIX (Moderated)
mod.techreports		Announcements and lists of technical reports. (Moderated)
mod.telecom		Telecommunications digest. (Moderated)
mod.test		Testing of moderated newsgroups -- no moderator (Moderated)
mod.vlsi		Very large scale integrated circuits. (Moderated)
misc.consumers		Consumer interests, product reviews, etc.
misc.consumers.house	Discussion about owning and maintaining a house.
misc.forsale		Items for sale.
misc.headlines		Current interest: drug testing, terrorism, etc.
misc.invest		Investments and the handling of money.
misc.jobs		Job announcements, requests, etc.
misc.kids		Children, their behavior and activities.
misc.legal		Legalities and the ethics of law.
misc.misc		Various discussions not fitting in any other group.
misc.taxes		Tax laws and advice.
misc.test		For testing of network software.  Very boring.
misc.wanted		Requests for things that are needed (NOT software).
sci.astro		Astronomy discussions and information.
sci.bio			Biology and related sciences.
sci.crypt		Different methods of data en/decryption.
sci.electronics		Circuits, theory, electrons and discussions.
sci.lang		Natural languages, communication, etc.
sci.math		Mathematical discussions and pursuits
sci.math.stat		Statistics discussion.
sci.math.symbolic	Symbolic algebra discussion.
sci.med			Medicine and its related products and regulations.
sci.misc		Short-lived discussions on subjects in the sciences.
sci.physics		Physical laws, properties, etc.
sci.research		Research methods, funding, ethics, and whatever.
sci.space		Space, space programs, space related research, etc.
sci.space.shuttle	The space shuttle and the STS program.
soc.college		College, college activities, campus life, etc.
soc.culture.african	Discussions about Africa & things African
soc.culture.celtic	Group about Celtics (*not* basketball!)
soc.culture.greek	Group about Greeks.
soc.culture.indian	Group for discussion about India & things Indian
soc.culture.jewish	Group for discussion about Jewish culture
soc.culture.misc	Group for discussion about other cultures
soc.misc		Socially-oriented topics not in other groups.
soc.motss		Issues pertaining to homosexuality.
soc.roots		Genealogical matters.
soc.singles		Newsgroup for single people, their activities, etc.
soc.net-people		Announcements, requests, etc. about people on the net.
soc.women		Women's rights, discrimination, etc.
talk.abortion		All sorts of discussions and arguments on abortion.
talk.bizarre		The unusual, bizarre, curious, and often stupid.
talk.origins		Evolution versus creationism (sometimes hot!).
talk.philosophy.misc	Philosophical musings on all topics.
talk.politics.misc	Political discussions and ravings of all kinds.
talk.politics.theory	Theory of politics and political systems.
talk.religion.misc	Religious, ethical, & moral implications.
talk.rumors		For the posting of rumors.
E_O_F
: if active file is empty, create it
if test ! -s $LIBDIR/active
then
	sed 's/[ 	].*/ 00000 00001/' /tmp/$$groups > $LIBDIR/active
	cat <<'E_O_F' >>$LIBDIR/active
control 00000 00001
junk 00000 00001
E_O_F
	set - group 0 1
else
: make sure it is in the new format
	set - `sed 1q $LIBDIR/active`
	case $# in
	4)	ed - $LIBDIR/active << 'EOF'
g/^mod\./s/y$/m/
w
q
EOF
		;;
	3)	;;
	2)	ed - $LIBDIR/active << 'EOF'
1,$s/$/ 00001/
w
q
EOF
		echo
		echo Active file updated to new format.
		echo You must run expire immediately after this install
		echo is done to properly update the tables.;;
	*) echo Active file is in unrecognized format. Not upgraded.;;
	esac
fi
if test $# -eq 3 -o $# -eq 2
then
	(sed '/^!net/!d
s/^!//
s!^!/!
s!$! /s/$/ n/!
' $LIBDIR/ngfile
	echo '/ n$/!s/$/ y/') >/tmp/$$sed
	mv $LIBDIR/active $LIBDIR/oactive
	sed -f /tmp/$$sed $LIBDIR/oactive >$LIBDIR/active
	chown $NEWSUSR $LIBDIR/active
	chgrp $NEWSGRP $LIBDIR/active
	chmod 644 $LIBDIR/active
fi
sort /tmp/$$groups | $LIBDIR/checkgroups | tee /tmp/checkgroups.out
echo the output of checkgroups has been copied into /tmp/checkgroups.out
rm -f /tmp/$$*

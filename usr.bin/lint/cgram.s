.comm	_yylval,2
.comm	_yyval,2
.data
_fake:
0
.globl
.bss
_fakenam:
.=.+12
.globl	_mkty
.text
_mkty:
~~mkty:
jsr	r5,csv
~t=4
~d=6
~s=10
jbr	L1
L2:mov	10(r5),(sp)
mov	6(r5),-(sp)
mov	4(r5),-(sp)
mov	_NIL,-(sp)
mov	_NIL,-(sp)
mov	$41,-(sp)
jsr	pc,*$_block
add	$12,sp
jbr	L3
L3:jmp	cret
L1:jbr	L2
.globl	_bdty
.text
_bdty:
~~bdty:
jsr	r5,csv
~op=4
~p=6
~v=10
jbr	L4
L5:~q=r4
mov	$4,(sp)
clr	-(sp)
mov	$4,-(sp)
mov	_NIL,-(sp)
mov	6(r5),-(sp)
mov	4(r5),-(sp)
jsr	pc,*$_block
add	$12,sp
mov	r0,r4
mov	4(r5),r0
jbr	L8
L9:L10:jbr	L7
L11:mov	10(r5),(sp)
jsr	pc,*$_bcon
mov	r0,12(r4)
jbr	L7
L12:mov	10(r5),14(r4)
jbr	L7
L13:mov	$L14,(sp)
jsr	pc,*$_cerror
jbr	L7
L8:cmp	r0,$2
jeq	L12
cmp	r0,$15
jeq	L9
cmp	r0,$66
jeq	L11
cmp	r0,$110
jeq	L10
jbr	L13
L7:mov	r4,r0
jbr	L6
L6:jmp	cret
L4:jbr	L5
.globl	_dstash
.text
_dstash:
~~dstash:
jsr	r5,csv
~n=4
jbr	L15
L16:cmp	$1355,_curdim
jgt	L18
mov	$L19,(sp)
jsr	pc,*$_cerror
L18:mov	_curdim,r0
asl	r0
inc	_curdim
mov	4(r5),_dimtab(r0)
L17:jmp	cret
L15:jbr	L16
.globl	_savebc
.text
_savebc:
~~savebc:
jsr	r5,csv
jbr	L20
L21:cmp	$300+_asavbc,_psavbc
jhis	L23
mov	$L24,(sp)
jsr	pc,*$_cerror
L23:mov	_brklab,*_psavbc
add	$2,_psavbc
mov	_contlab,*_psavbc
add	$2,_psavbc
mov	_flostat,*_psavbc
add	$2,_psavbc
mov	_swx,*_psavbc
add	$2,_psavbc
clr	_flostat
L22:jmp	cret
L20:jbr	L21
.globl	_resetbc
.text
_resetbc:
~~resetbc:
jsr	r5,csv
~mask=4
jbr	L25
L26:sub	$2,_psavbc
mov	*_psavbc,_swx
sub	$2,_psavbc
mov	_flostat,r0
mov	4(r5),r1
com	r1
bic	r1,r0
bis	*_psavbc,r0
mov	r0,_flostat
sub	$2,_psavbc
mov	*_psavbc,_contlab
sub	$2,_psavbc
mov	*_psavbc,_brklab
L27:jmp	cret
L25:jbr	L26
.globl	_addcase
.text
_addcase:
~~addcase:
jsr	r5,csv
~p=4
jbr	L28
L29:mov	4(r5),(sp)
jsr	pc,*$_optim
mov	r0,4(r5)
cmp	$4,*4(r5)
jeq	L31
mov	$L32,(sp)
jsr	pc,*$_uerror
jbr	L30
L31:cmp	$_swtab,_swp
jne	L33
mov	$L34,(sp)
jsr	pc,*$_uerror
jbr	L30
L33:cmp	$2734+_swtab,_swp
jhi	L35
mov	$L36,(sp)
jsr	pc,*$_cerror
L35:mov	4(r5),r0
mov	10+2(r0),r1
mov	10(r0),r0
mov	_swp,r2
mov	r1,+2(r2)
mov	r0,(r2)
mov	_swp,r0
mov	$12,4(r0)
mov	4(r0),r0
mov	r0,(sp)
jsr	pc,*$_deflab
add	$6,_swp
mov	4(r5),(sp)
jsr	pc,*$_tfree
L30:jmp	cret
L28:jbr	L29
.globl	_adddef
.text
_adddef:
~~adddef:
jsr	r5,csv
jbr	L37
L38:mov	_swx,r1
mul	$6,r1
tst	4+_swtab(r1)
jlt	L40
mov	$L41,(sp)
jsr	pc,*$_uerror
jbr	L39
L40:cmp	$_swtab,_swp
jne	L42
mov	$L43,(sp)
jsr	pc,*$_uerror
jbr	L39
L42:mov	_swx,r1
mul	$6,r1
mov	$12,4+_swtab(r1)
mov	4+_swtab(r1),r1
mov	r1,(sp)
jsr	pc,*$_deflab
L39:jmp	cret
L37:jbr	L38
.globl	_swstart
.text
_swstart:
~~swstart:
jsr	r5,csv
jbr	L44
L45:cmp	$2734+_swtab,_swp
jhi	L47
mov	$L48,(sp)
jsr	pc,*$_cerror
L47:mov	_swp,r1
sub	$_swtab,r1
bic	r0,r0
sbc	r0
div	$6,r0
mov	r0,_swx
mov	_swp,r0
mov	$-1,4(r0)
add	$6,_swp
L46:jmp	cret
L44:jbr	L45
.globl	_swend
.text
_swend:
~~swend:
jsr	r5,csv
jbr	L49
L50:~swbeg=r4
~p=r3
~q=r2
~r=177766
~r1=177764
~temp=177760
~tempi=177756
mov	_swx,r1
mul	$6,r1
mov	r1,r4
add	$6+_swtab,r4
mov	r4,-14(r5)
mov	_swp,r0
add	$-6,r0
mov	r0,-12(r5)
L52:cmp	-12(r5),r4
jlos	L53
mov	r4,r2
L54:cmp	-12(r5),r2
jlos	L55
cmp	(r2),6(r2)
jlt	L57
jgt	L10001
cmp	2(r2),10(r2)
jlos	L57
L10001:mov	r2,r0
add	$6,r0
mov	r0,-14(r5)
mov	(r2),-20(r5)
mov	2(r2),-16(r5)
mov	-14(r5),r0
mov	(r0),(r2)
mov	+2(r0),2(r2)
mov	-14(r5),r0
mov	-16(r5),+2(r0)
mov	-20(r5),(r0)
mov	4(r2),-22(r5)
mov	-14(r5),r0
mov	4(r0),4(r2)
mov	-14(r5),r0
mov	-22(r5),4(r0)
L57:L56:add	$6,r2
jbr	L54
L55:mov	-14(r5),-12(r5)
mov	r4,-14(r5)
jbr	L52
L53:mov	r4,r3
add	$6,r3
L58:cmp	_swp,r3
jlos	L59
cmp	-6(r3),(r3)
jne	L61
cmp	-4(r3),2(r3)
jne	L61
mov	2(r3),r0
mov	r0,-22(r5)
mov	r0,(sp)
mov	$L62,-(sp)
jsr	pc,*$_uerror
tst	(sp)+
jbr	L51
L61:L60:add	$6,r3
jbr	L58
L59:mov	r4,r0
add	$-6,r0
mov	r0,_swp
L51:jmp	cret
L49:sub	$12,sp
jbr	L50
.globl	_yyexca
_yyexca:
177777
1
0
177777
2
25
13
25
62
25
71
25
177776
0
177777
23
70
116
71
116
177776
7
177777
30
70
115
71
115
177776
113
177777
40
64
53
177776
51
177777
42
64
43
177776
41
.globl	_yyact
_yyact:
345
30
407
371
127
125
126
32
32
116
343
200
114
32
113
115
25
25
12
11
16
140
25
50
52
177
32
136
136
142
133
120
117
22
20
25
132
357
360
364
100
370
355
356
365
366
367
373
372
121
111
122
302
152
344
26
26
135
361
217
32
26
240
12
.globl
11
16
154
71
341
25
346
66
420
24
26
201
417
10
62
20
134
21
152
220
224
34
23
44
401
110
460
210
211
212
213
214
215
6
230
313
44
43
51
53
457
452
135
437
26
67
413
412
242
243
244
246
250
252
254
256
257
261
263
265
266
267
270
271
.globl
204
241
206
201
145
143
275
232
131
306
273
235
144
237
305
220
230
104
141
431
207
272
105
376
205
46
103
400
274
47
333
150
221
227
156
153
107
234
314
67
315
46
316
57
317
47
320
60
231
321
77
322
333
323
375
371
127
125
126
325
201
116
233
226
.globl
114
41
310
115
57
102
337
326
60
135
277
56
57
32
445
332
60
37
327
120
117
374
25
435
304
357
360
364
221
370
355
356
365
366
367
373
372
121
340
122
332
152
304
342
410
426
361
331
415
4
403
404
405
406
300
411
335
336
106
427
55
26
421
303
.globl
311
362
76
446
377
137
137
432
425
225
424
33
12
11
16
423
422
303
334
440
441
410
443
442
33
225
144
63
20
434
151
433
11
162
64
163
36
430
165
454
447
167
416
410
170
455
171
12
174
16
172
173
175
157
166
161
164
162
33
163
276
7
165
20
.globl
165
101
72
155
42
162
35
163
40
363
165
147
202
167
456
161
170
161
171
160
174
176
172
173
175
157
166
161
164
61
31
73
54
307
130
65
70
236
312
146
75
162
74
163
45
3
165
2
223
167
453
123
170
13
171
160
174
176
172
173
175
157
166
161
.globl
164
14
5
27
15
17
354
352
353
351
347
350
1
162
0
163
0
0
165
0
0
167
451
0
170
0
171
160
174
176
172
173
175
157
166
161
164
0
0
0
0
0
0
0
0
0
162
0
163
0
0
165
0
362
0
0
0
0
450
162
0
163
0
160
.globl
165
176
157
167
161
164
170
0
171
0
174
444
172
173
175
157
166
161
164
162
0
163
0
0
165
0
0
167
0
0
170
0
171
0
174
0
172
173
175
157
166
161
164
0
162
160
163
176
0
165
0
162
167
163
0
170
165
171
0
174
0
172
173
175
.globl
157
166
161
164
0
160
436
176
0
161
164
0
0
0
0
0
0
0
0
0
0
0
0
0
0
0
0
0
0
0
160
402
176
162
0
163
0
0
165
0
0
167
0
0
170
0
171
0
174
0
172
173
175
157
166
161
164
162
0
163
0
0
165
0
.globl
0
167
0
0
170
0
171
0
174
324
172
173
175
157
166
161
164
0
330
160
0
176
0
0
0
0
0
0
0
162
0
163
0
162
165
163
0
167
165
0
170
167
171
160
174
176
172
173
175
157
166
161
164
157
166
161
164
0
0
0
0
0
0
0
.globl
0
162
0
163
0
0
165
0
0
167
301
0
170
0
171
160
174
176
172
173
175
157
166
161
164
162
0
163
0
0
165
0
0
167
0
0
170
0
171
0
174
0
172
173
175
157
166
161
164
0
0
160
0
176
124
127
125
126
0
0
116
0
0
114
.globl
0
0
115
124
127
125
126
0
0
116
0
0
114
176
0
115
0
0
120
117
0
0
0
0
0
0
0
0
0
0
0
120
117
0
0
0
121
0
122
0
0
124
127
125
126
0
264
116
0
121
114
122
0
115
124
127
125
126
0
262
116
0
0
114
.globl
0
0
115
0
0
120
117
0
0
0
0
0
0
0
0
0
0
0
120
117
0
0
0
121
0
122
0
0
124
127
125
126
0
260
116
0
121
114
122
0
115
124
127
125
126
0
255
116
0
0
114
0
0
115
0
0
120
117
0
0
0
0
0
0
.globl
0
0
0
0
0
120
117
0
0
0
121
0
122
0
0
124
127
125
126
0
253
116
0
121
114
122
0
115
124
127
125
126
0
251
116
0
0
114
0
0
115
0
0
120
117
0
0
0
0
0
0
0
0
0
0
0
120
117
0
0
0
121
0
122
.globl
0
0
124
127
125
126
0
247
116
0
121
114
122
0
115
124
127
125
126
0
245
116
0
0
114
0
0
115
0
0
120
117
162
0
163
0
0
165
0
0
167
0
0
120
117
171
0
0
121
0
122
0
157
166
161
164
0
414
0
0
0
121
0
122
.globl
0
203
124
127
125
126
0
0
116
0
0
114
0
0
115
124
127
125
126
0
0
116
0
0
114
0
0
115
0
0
120
117
0
12
0
16
0
0
0
0
0
0
0
120
117
0
0
0
121
20
122
0
0
0
0
124
127
125
126
0
0
121
0
122
.globl
0
112
124
127
125
126
0
0
116
0
0
114
0
0
115
124
127
125
126
120
117
116
0
0
114
0
0
115
0
0
120
117
0
0
0
0
0
121
0
122
0
0
0
120
117
0
0
162
121
163
122
222
165
124
127
125
126
0
0
116
0
121
114
122
.globl
0
115
0
157
166
161
164
0
162
0
163
0
0
165
0
0
167
120
117
170
0
171
0
174
0
172
173
0
157
166
161
164
0
0
162
121
163
216
0
165
0
0
167
0
0
170
0
171
0
0
0
172
0
0
157
166
161
164
162
0
163
0
0
165
.globl
0
0
167
0
0
170
0
171
0
0
0
0
0
0
157
166
161
164
.globl	_yypact
_yypact:
176030
177761
176030
176030
176030
30
176030
416
376
176030
403
176030
176030
235
506
215
502
176030
54
167
176030
313
313
311
171
24
351
176030
176030
176030
401
416
176030
500
176030
176030
176030
176030
317
173
171
167
222
147
136
176030
176030
307
155
1715
176030
176030
176030
117
176030
6
132
176030
177743
72
176030
36
176030
176030
.globl
154
2015
176030
176030
176030
501
176030
176030
153
1223
1615
142
2015
2015
2015
2015
2015
2063
1700
2000
347
176030
176030
176030
212
416
130
176030
167
232
176030
176030
211
500
176030
176030
167
176030
176030
176030
5
110
176030
176030
1223
176030
176030
2015
2015
1532
1515
1447
1432
1364
2015
1347
1301
1264
2015
2015
2015
2015
2015
122
.globl
176030
1223
1615
176030
176030
2015
472
176030
142
142
142
142
142
142
1700
303
1127
315
176030
126
1223
176030
176030
176030
176030
176030
176030
176030
176030
176030
353
52
176030
176030
755
1223
176030
2015
465
2015
465
2015
463
2015
176030
2015
650
2051
2015
1632
2015
1133
2015
2164
2134
1063
1223
1223
206
1615
122
1033
176030
274
.globl
2015
176030
176
337
315
2015
176030
6
176030
13
1
176030
1223
1223
1223
1223
1223
1223
1223
1223
2015
176030
176030
201
176030
1765
142
140
176030
150
176
1223
176030
176030
37
176030
176030
746
176030
265
265
265
265
2015
265
66
65
1600
450
176030
23
265
336
335
176030
330
326
327
2015
417
2102
176030
176030
134
.globl
325
176030
176030
370
176030
176030
362
244
1173
176030
176030
176030
176030
715
62
176030
176030
176030
2015
2015
2015
2015
176030
665
176030
176030
233
176030
321
265
176030
176030
607
543
60
477
176030
176030
2015
176030
176030
176030
2015
176030
433
57
41
176030
176030
.globl	_yypgo
_yypgo:
0
614
50
613
612
611
610
607
606
605
604
603
0
2
16
141
602
115
601
565
73
64
563
36
120
1
560
557
555
41
554
552
550
106
547
546
12
545
44
475
544
25
103
543
542
107
541
540
111
537
536
535
31
13
514
513
511
501
.globl	_yyr1
_yyr1:
0
1
1
33
33
34
34
36
34
37
40
40
43
43
45
45
45
42
42
42
20
20
17
17
17
17
47
21
21
21
21
21
22
22
11
11
50
50
52
52
23
23
12
12
53
53
55
55
46
56
46
27
27
27
27
27
31
31
31
31
31
31
30
30
.globl
30
30
30
30
30
13
57
57
35
61
35
62
62
60
60
60
60
64
64
65
65
51
51
54
54
63
63
66
41
67
44
44
44
44
44
44
44
44
44
44
44
44
44
44
44
44
44
70
70
70
7
4
3
5
6
10
71
2
15
15
32
32
14
14
.globl
14
14
14
14
14
14
14
14
14
14
14
14
14
14
14
14
14
14
14
14
14
14
14
16
16
16
16
16
16
16
16
16
16
16
16
16
16
16
16
16
16
24
25
25
25
25
25
25
25
26
26
.globl	_yyr2
_yyr2:
0
2
0
1
1
2
3
0
4
2
2
0
2
0
3
4
0
3
2
2
1
0
2
2
1
1
1
1
2
3
1
1
5
2
1
2
1
3
1
3
5
2
1
2
1
3
2
1
1
0
4
1
1
3
2
1
2
3
3
4
1
3
2
3
.globl
3
4
3
3
2
2
1
3
1
0
4
1
1
1
1
3
6
1
3
1
4
0
1
0
1
0
1
1
4
1
2
1
2
2
2
7
4
2
2
2
2
3
3
1
2
2
2
2
3
2
1
4
3
4
6
4
0
2
1
0
1
3
3
3
.globl
3
3
3
3
3
3
3
3
3
3
3
4
4
4
4
4
4
4
4
5
3
3
1
2
2
2
2
2
2
2
4
4
4
2
3
3
1
1
1
1
3
2
0
2
5
2
3
4
3
2
2
.globl	_yychk
_yychk:
176030
177777
177745
177744
400
177760
177761
177731
177757
42
41
177755
177756
177766
43
177767
61
71
177743
177750
177720
13
62
177765
177747
177716
2
400
177757
177731
41
64
2
64
2
71
70
177742
62
66
177747
177750
177747
177750
177721
63
2
62
66
177715
72
62
41
177725
177723
177757
177730
177726
2
177717
177741
177740
63
67
.globl
177776
177707
63
63
63
70
63
67
177776
177764
64
177762
13
16
10
37
36
60
62
177752
2
4
5
3
177724
71
177732
177751
177750
177747
26
400
177727
70
72
177720
177750
177737
177736
177711
177761
400
64
67
177764
2
67
32
70
34
6
10
35
13
33
16
21
23
27
30
25
31
72
177714
.globl
177713
177764
177712
64
36
66
40
62
177762
177762
177762
177762
177762
177762
62
177754
177764
177757
63
177746
177764
62
65
177723
70
26
177776
65
177726
177776
177733
177732
71
71
177764
177764
177764
72
177764
72
177764
72
177764
72
177764
72
177764
177764
72
177764
72
177764
72
177764
177764
177764
177764
177764
177727
70
177714
177764
2
177754
.globl
63
63
177753
62
13
70
63
177722
177776
177761
177735
71
177764
177764
177764
177764
177764
177764
177764
177764
26
65
177713
177727
67
63
177762
66
63
177753
177753
177764
177751
71
177743
177734
65
177764
177737
177774
177775
177773
177771
177772
177770
51
52
44
45
71
400
177710
46
53
54
55
50
2
57
56
177764
65
67
177776
.globl
63
71
71
177734
177734
177734
177734
177763
177764
177734
71
71
71
177764
2
71
65
177734
62
62
62
62
26
177764
26
67
62
47
53
63
71
71
177764
177764
177763
177764
26
63
62
177734
63
63
71
63
177764
177763
63
71
71
.globl	_yydef
.data
_yydef:
2
177776
1
3
4
0
24
30
31
32
33
36
37
0
52
0
42
5
0
177776
110
0
0
0
177776
131
74
114
26
27
34
0
177776
0
177776
6
111
13
0
170
70
76
0
0
0
104
106
0
170
0
132
105
35
127
54
57
125
44
46
0
10
0
77
100
.globl
0
0
75
102
103
0
71
72
0
117
0
226
0
0
0
0
0
0
0
0
244
245
246
247
0
130
56
60
63
64
170
67
0
126
170
112
116
11
12
20
0
0
135
101
171
107
73
0
0
0
0
0
0
0
0
0
0
0
0
0
0
0
0
125
.globl
121
123
0
133
227
0
0
262
230
231
232
233
234
235
0
0
0
252
241
0
174
261
50
55
61
170
66
40
45
47
15
0
22
23
176
177
200
0
201
0
202
0
203
0
204
0
205
206
0
207
0
210
0
211
212
0
224
225
0
126
125
0
243
0
.globl
0
250
251
252
252
0
242
0
65
0
0
21
214
215
216
217
213
220
221
222
0
120
122
0
240
237
236
170
253
0
255
175
62
16
0
14
134
0
137
0
0
0
0
173
0
0
0
0
0
153
0
0
0
0
162
0
0
244
0
0
223
124
256
0
.globl
260
17
136
140
141
142
0
0
172
145
146
147
150
0
0
154
155
156
0
0
173
0
157
0
161
257
0
164
0
0
151
152
0
0
0
0
160
254
0
144
163
165
173
167
0
0
0
166
143
.globl	_yydebug
.data
_yydebug:
0
.comm	_yyv,454
.globl	_yychar
.data
_yychar:
177777
.globl	_yynerrs
.data
_yynerrs:
0
.globl	_yyerrfl
.data
_yyerrfl:
0
.globl	_yyparse
.text
_yyparse:
~~yyparse:
jsr	r5,csv
jbr	L63
L64:~yys=177314
~yyj=177312
~yym=177310
~yypvt=r4
~yystate=r3
~yyps=r2
~yyn=177306
~yypv=177304
~yyxi=177302
clr	r3
mov	$-1,_yychar
clr	_yynerrs
clr	_yyerrfl
mov	r5,r2
add	$-466,r2
mov	$-2+_yyv,-474(r5)
L66:tst	_yydebug
jeq	L67
mov	_yychar,(sp)
mov	r3,-(sp)
mov	$L68,-(sp)
jsr	pc,*$_printf
cmp	(sp)+,(sp)+
L67:add	$2,r2
mov	r5,r0
add	$-10,r0
cmp	r2,r0
jlos	L69
mov	$L70,(sp)
jsr	pc,*$_yyerror
mov	$1,r0
jbr	L65
L69:mov	r3,(r2)
add	$2,-474(r5)
mov	_yyval,*-474(r5)
L71:mov	r3,r0
asl	r0
mov	_yypact(r0),-472(r5)
cmp	$-1750,-472(r5)
jge	L72
tst	_yychar
jge	L73
jsr	pc,_yylex
mov	r0,_yychar
jge	L74
clr	_yychar
L74:L73:add	_yychar,-472(r5)
jlt	L72
cmp	$2222,-472(r5)
jle	L72
mov	-472(r5),r0
asl	r0
mov	_yyact(r0),r0
mov	r0,-472(r5)
asl	r0
cmp	_yychar,_yychk(r0)
jne	L75
mov	$-1,_yychar
mov	_yylval,_yyval
mov	-472(r5),r3
tst	_yyerrfl
jle	L76
dec	_yyerrfl
L76:jbr	L66
L75:L72:mov	r3,r0
asl	r0
mov	_yydef(r0),r0
mov	r0,-472(r5)
cmp	$-2,r0
jne	L77
tst	_yychar
jge	L78
jsr	pc,_yylex
mov	r0,_yychar
jge	L79
clr	_yychar
L79:L78:mov	$_yyexca,-476(r5)
L80:cmp	$-1,*-476(r5)
jne	L10002
mov	-476(r5),r0
cmp	r3,2(r0)
jeq	L81
L10002:L82:add	$4,-476(r5)
jbr	L80
L81:L83:add	$4,-476(r5)
tst	*-476(r5)
jlt	L84
cmp	_yychar,*-476(r5)
jeq	L84
jbr	L83
L84:mov	-476(r5),r0
mov	2(r0),-472(r5)
jge	L85
clr	r0
jbr	L65
L85:L77:tst	-472(r5)
jne	L86
mov	_yyerrfl,r0
jbr	L88
L89:mov	$L90,(sp)
jsr	pc,*$_yyerror
L91:inc	_yynerrs
L92:L93:mov	$3,_yyerrfl
L94:mov	r5,r0
add	$-464,r0
cmp	r2,r0
jlo	L95
mov	(r2),r0
asl	r0
mov	_yypact(r0),r0
add	$400,r0
mov	r0,-472(r5)
tst	-472(r5)
jlt	L96
cmp	$2222,-472(r5)
jle	L96
mov	-472(r5),r0
asl	r0
mov	_yyact(r0),r0
asl	r0
cmp	$400,_yychk(r0)
jne	L96
mov	-472(r5),r0
asl	r0
mov	_yyact(r0),r3
jbr	L66
L96:mov	(r2),r0
asl	r0
mov	_yypact(r0),-472(r5)
tst	_yydebug
jeq	L97
mov	-2(r2),(sp)
mov	(r2),-(sp)
mov	$L98,-(sp)
jsr	pc,*$_printf
cmp	(sp)+,(sp)+
L97:sub	$2,r2
sub	$2,-474(r5)
jbr	L94
L95:L99:mov	$1,r0
jbr	L65
L100:tst	_yydebug
jeq	L101
mov	_yychar,(sp)
mov	$L102,-(sp)
jsr	pc,*$_printf
tst	(sp)+
L101:tst	_yychar
jeq	L99
mov	$-1,_yychar
jbr	L71
jbr	L87
L88:cmp	r0,$3
jhi	L87
asl	r0
jmp	*L10004(r0)
	.data
L10004:L89
L92
L93
L100
.text
L87:L86:tst	_yydebug
jeq	L103
mov	-472(r5),(sp)
mov	$L104,-(sp)
jsr	pc,*$_printf
tst	(sp)+
L103:mov	-472(r5),r0
asl	r0
mov	_yyr2(r0),r0
asl	r0
sub	r0,r2
mov	-474(r5),r4
mov	-472(r5),r0
asl	r0
mov	_yyr2(r0),r0
asl	r0
sub	r0,-474(r5)
mov	-474(r5),r0
mov	2(r0),_yyval
mov	-472(r5),-470(r5)
mov	-472(r5),r0
asl	r0
mov	_yyr1(r0),-472(r5)
mov	-472(r5),r0
asl	r0
mov	_yypgo(r0),r0
add	(r2),r0
inc	r0
mov	r0,-466(r5)
cmp	$2222,-466(r5)
jle	L10005
mov	-466(r5),r0
asl	r0
mov	_yyact(r0),r3
mov	r3,r0
asl	r0
mov	-472(r5),r1
neg	r1
cmp	r1,_yychk(r0)
jeq	L105
L10005:mov	-472(r5),r0
asl	r0
mov	_yypgo(r0),r0
asl	r0
mov	_yyact(r0),r3
L105:mov	-470(r5),r0
jbr	L107
L108:jsr	pc,_ftnend
jbr	L106
L109:clr	_curclas
clr	_blevel
jbr	L106
L110:clr	_curclas
clr	_blevel
jbr	L106
L111:mov	$141,*-2(r4)
jbr	L106
L112:mov	$141,*-4(r4)
jbr	L106
L113:cmp	$3,_curclas
jne	L10006
mov	$3,(sp)
jbr	L10007
L10006:mov	$5,(sp)
L10007:mov	(r4),-(sp)
mov	-2(r4),-(sp)
jsr	pc,_tymerge
cmp	(sp)+,(sp)+
mov	r0,-(sp)
jsr	pc,*$_defid
tst	(sp)+
jbr	L106
L114:tst	_blevel
jeq	L115
mov	$L116,(sp)
jsr	pc,*$_cerror
L115:tst	_reached
jeq	L117
bis	$2,_retstat
L117:mov	$141,*-6(r4)
jsr	pc,_ftnend
jbr	L106
L118:mov	$1,_blevel
jbr	L106
L119:clr	r0
jbr	L106
L120:mov	$141,*-2(r4)
jbr	L106
L121:mov	$141,*-4(r4)
jbr	L106
L122:clr	_curclas
mov	$141,*-4(r4)
jbr	L106
L123:clr	_curclas
mov	$141,*-2(r4)
jbr	L106
L124:clr	_curclas
jbr	L106
L125:mov	$4,(sp)
clr	-(sp)
mov	$4,-(sp)
jsr	pc,*$_mkty
cmp	(sp)+,(sp)+
mov	r0,_yyval
clr	_curclas
jbr	L106
L126:mov	(r4),_yyval
jbr	L106
L127:mov	$4,(sp)
clr	-(sp)
mov	$4,-(sp)
jsr	pc,*$_mkty
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L128:clr	_curclas
jbr	L106
L129:mov	(r4),_curclas
jbr	L106
L130:clr	(sp)
mov	(r4),r0
mov	2(r0),-(sp)
mov	-2(r4),r0
mov	2(r0),-(sp)
jsr	pc,*$_types
cmp	(sp)+,(sp)+
mov	-2(r4),r1
mov	r0,2(r1)
mov	$141,*(r4)
jbr	L106
L131:mov	(r4),r0
mov	2(r0),(sp)
mov	-2(r4),r0
mov	2(r0),-(sp)
mov	-4(r4),r0
mov	2(r0),-(sp)
jsr	pc,*$_types
cmp	(sp)+,(sp)+
mov	-4(r4),r1
mov	r0,2(r1)
mov	$141,r0
mov	r0,*(r4)
mov	r0,*-2(r4)
jbr	L106
L132:mov	-10(r4),(sp)
jsr	pc,*$_dclstru
mov	r0,_yyval
jbr	L106
L133:clr	(sp)
mov	(r4),-(sp)
jsr	pc,*$_rstruct
tst	(sp)+
mov	r0,_yyval
mov	_instruc,_stwart
jbr	L106
L134:clr	(sp)
mov	$-1,-(sp)
jsr	pc,*$_bstruct
tst	(sp)+
mov	r0,_yyval
jbr	L106
L135:clr	(sp)
mov	(r4),-(sp)
jsr	pc,*$_bstruct
tst	(sp)+
mov	r0,_yyval
jbr	L106
L136:mov	(r4),(sp)
jsr	pc,*$_moedef
jbr	L106
L137:mov	(r4),_strucof
mov	-4(r4),(sp)
jsr	pc,*$_moedef
jbr	L106
L138:mov	-10(r4),(sp)
jsr	pc,*$_dclstru
mov	r0,_yyval
jbr	L106
L139:mov	-2(r4),(sp)
mov	(r4),-(sp)
jsr	pc,*$_rstruct
tst	(sp)+
mov	r0,_yyval
jbr	L106
L140:mov	(r4),(sp)
mov	$-1,-(sp)
jsr	pc,*$_bstruct
tst	(sp)+
mov	r0,_yyval
clr	_stwart
jbr	L106
L141:mov	-2(r4),(sp)
mov	(r4),-(sp)
jsr	pc,*$_bstruct
tst	(sp)+
mov	r0,_yyval
clr	_stwart
jbr	L106
L142:clr	_curclas
clr	_stwart
mov	$141,*-2(r4)
jbr	L106
L143:cmp	$13,_curclas
jeq	L144
clr	_curclas
jbr	L145
L144:mov	_fake,(sp)
inc	_fake
mov	$L146,-(sp)
mov	$_fakenam,-(sp)
jsr	pc,*$_sprintf
cmp	(sp)+,(sp)+
mov	_curclas,(sp)
mov	$1,-(sp)
mov	$_fakenam,-(sp)
jsr	pc,_lookup
cmp	(sp)+,(sp)+
mov	r0,-(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,_bdty
add	$6,sp
mov	r0,-(sp)
mov	(r4),-(sp)
jsr	pc,_tymerge
cmp	(sp)+,(sp)+
mov	r0,-(sp)
jsr	pc,*$_defid
tst	(sp)+
L145:clr	_stwart
mov	$141,*(r4)
jbr	L106
L147:mov	_curclas,(sp)
mov	(r4),-(sp)
mov	-2(r4),-(sp)
jsr	pc,_tymerge
cmp	(sp)+,(sp)+
mov	r0,-(sp)
jsr	pc,*$_defid
tst	(sp)+
mov	_instruc,_stwart
jbr	L106
L148:mov	-4(r4),_yyval
jbr	L106
L149:mov	_curclas,(sp)
mov	(r4),-(sp)
mov	-10(r4),-(sp)
jsr	pc,_tymerge
cmp	(sp)+,(sp)+
mov	r0,-(sp)
jsr	pc,*$_defid
tst	(sp)+
mov	_instruc,_stwart
jbr	L106
L150:bit	$2,_instruc
jne	L151
mov	$L152,(sp)
jsr	pc,*$_uerror
L151:tst	(r4)
jlt	L10008
cmp	$100,(r4)
jgt	L153
L10008:mov	$L154,(sp)
jsr	pc,*$_uerror
mov	$1,(r4)
L153:mov	(r4),(sp)
bis	$100,(sp)
mov	-4(r4),-(sp)
mov	-6(r4),-(sp)
jsr	pc,_tymerge
cmp	(sp)+,(sp)+
mov	r0,-(sp)
jsr	pc,*$_defid
tst	(sp)+
mov	_NIL,_yyval
jbr	L106
L155:bit	$2,_instruc
jne	L156
mov	$L157,(sp)
jsr	pc,*$_uerror
L156:mov	-4(r4),(sp)
mov	$-1,-(sp)
mov	(r4),-(sp)
mov	$_stab,-(sp)
jsr	pc,*$_falloc
add	$6,sp
mov	_NIL,_yyval
jbr	L106
L158:mov	_NIL,_yyval
jbr	L106
L159:L160:clr	(sp)
mov	(r4),-(sp)
mov	$15,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L161:L162:clr	(sp)
mov	-4(r4),-(sp)
mov	$110,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L163:L164:clr	(sp)
mov	-4(r4),-(sp)
mov	$66,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L165:L166:tst	-2(r4)
jgt	L167
mov	$L168,(sp)
jsr	pc,*$_werror
L167:mov	-2(r4),(sp)
mov	-6(r4),-(sp)
mov	$66,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L169:mov	(r4),(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L170:mov	-2(r4),_yyval
jbr	L106
L171:jbr	L160
jbr	L106
L172:jbr	L162
jbr	L106
L173:jbr	L164
jbr	L106
L174:jbr	L166
jbr	L106
L175:mov	-2(r4),_yyval
jbr	L106
L176:tst	_blevel
jeq	L177
mov	$L178,(sp)
jsr	pc,*$_uerror
L177:clr	(sp)
mov	-4(r4),-(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,_bdty
add	$6,sp
mov	r0,-(sp)
mov	$110,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
clr	_stwart
jbr	L106
L179:clr	(sp)
mov	-2(r4),-(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,_bdty
add	$6,sp
mov	r0,-(sp)
mov	$110,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
clr	_stwart
jbr	L106
L180:mov	$1,_stwart
jbr	L106
L181:mov	(r4),(sp)
jsr	pc,*$_ftnarg
mov	$1,_stwart
jbr	L106
L182:mov	(r4),(sp)
jsr	pc,*$_ftnarg
mov	$1,_stwart
jbr	L106
L183:mov	-4(r4),_yyval
jbr	L106
L184:mov	_curclas,(sp)
mov	(r4),-(sp)
mov	-2(r4),-(sp)
jsr	pc,_tymerge
cmp	(sp)+,(sp)+
mov	r0,(r4)
mov	r0,-(sp)
jsr	pc,*$_defid
tst	(sp)+
mov	(r4),r0
mov	14(r0),(sp)
jsr	pc,*$_beginit
jbr	L106
L185:mov	(r4),(sp)
mov	-2(r4),-(sp)
jsr	pc,*$_tymerge
tst	(sp)+
mov	r0,(sp)
jsr	pc,*$_nidcl
jbr	L106
L186:mov	_curclas,(sp)
jsr	pc,*$_uclass
mov	r0,(sp)
mov	(r4),-(sp)
mov	-2(r4),-(sp)
jsr	pc,_tymerge
cmp	(sp)+,(sp)+
mov	r0,-(sp)
jsr	pc,*$_defid
tst	(sp)+
jbr	L106
L187:mov	(r4),(sp)
jsr	pc,*$_doinit
jsr	pc,_endinit
jbr	L106
L188:jsr	pc,_endinit
jbr	L106
L189:mov	(r4),(sp)
jsr	pc,*$_doinit
jbr	L106
L190:jsr	pc,_irbrace
jbr	L106
L191:mov	$L192,(sp)
jsr	pc,*$_werror
jbr	L106
L193:jsr	pc,_ilbrace
jbr	L106
L194:dec	_blevel
cmp	$1,_blevel
jne	L195
clr	_blevel
L195:mov	_blevel,(sp)
jsr	pc,*$_clearst
sub	$2,_psavbc
mov	*_psavbc,_autooff
sub	$2,_psavbc
mov	*_psavbc,_regvar
jbr	L106
L196:cmp	$1,_blevel
jne	L197
jsr	pc,_dclargs
L197:inc	_blevel
cmp	$304+_asavbc,_psavbc
jhis	L198
mov	$L199,(sp)
jsr	pc,*$_cerror
L198:mov	_regvar,*_psavbc
add	$2,_psavbc
mov	_autooff,*_psavbc
add	$2,_psavbc
jbr	L106
L200:mov	-2(r4),(sp)
jsr	pc,*$_ecomp
jbr	L106
L201:mov	-2(r4),(sp)
jsr	pc,*$_deflab
mov	$1,_reached
jbr	L106
L202:cmp	$-1,-2(r4)
jeq	L203
mov	-2(r4),(sp)
jsr	pc,*$_deflab
mov	$1,_reached
L203:jbr	L106
L204:mov	_contlab,(sp)
jsr	pc,*$_branch
mov	_brklab,(sp)
jsr	pc,*$_deflab
bit	$2,_flostat
jne	L10009
bit	$20,_flostat
jne	L205
L10009:mov	$1,_reached
jbr	L206
L205:clr	_reached
L206:clr	(sp)
jsr	pc,*$_resetbc
jbr	L106
L207:mov	_contlab,(sp)
jsr	pc,*$_deflab
bit	$4,_flostat
jeq	L208
mov	$1,_reached
L208:mov	-14(r4),(sp)
jsr	pc,*$_bcon
mov	r0,(sp)
mov	_NIL,-(sp)
mov	-4(r4),-(sp)
mov	$114,-(sp)
jsr	pc,_buildtr
add	$6,sp
mov	r0,-(sp)
mov	$155,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,(sp)
jsr	pc,*$_ecomp
mov	_brklab,(sp)
jsr	pc,*$_deflab
mov	$1,_reached
clr	(sp)
jsr	pc,*$_resetbc
jbr	L106
L209:mov	_contlab,(sp)
jsr	pc,*$_deflab
bit	$4,_flostat
jeq	L210
mov	$1,_reached
L210:tst	-4(r4)
jeq	L211
mov	-4(r4),(sp)
jsr	pc,*$_ecomp
L211:mov	-6(r4),(sp)
jsr	pc,*$_branch
mov	_brklab,(sp)
jsr	pc,*$_deflab
bit	$2,_flostat
jne	L10010
bit	$20,_flostat
jne	L212
L10010:mov	$1,_reached
jbr	L213
L212:clr	_reached
L213:clr	(sp)
jsr	pc,*$_resetbc
jbr	L106
L214:tst	_reached
jeq	L215
mov	_brklab,(sp)
jsr	pc,*$_branch
L215:mov	-2(r4),(sp)
jsr	pc,*$_deflab
jsr	pc,_swend
mov	_brklab,(sp)
jsr	pc,*$_deflab
bit	$2,_flostat
jne	L10011
bit	$10,_flostat
jne	L216
L10011:mov	$1,_reached
L216:mov	$4,(sp)
jsr	pc,*$_resetbc
jbr	L106
L217:cmp	$-1,_brklab
jne	L218
mov	$L219,(sp)
jsr	pc,*$_uerror
jbr	L220
L218:tst	_reached
jeq	L221
mov	_brklab,(sp)
jsr	pc,*$_branch
L221:L220:bis	$2,_flostat
tst	_brkflag
jne	L222
clr	_reached
jbr	L106
L223:cmp	$-1,_contlab
jne	L224
mov	$L225,(sp)
jsr	pc,*$_uerror
jbr	L226
L224:mov	_contlab,(sp)
jsr	pc,*$_branch
L226:bis	$4,_flostat
jbr	L222
jbr	L106
L227:bis	$2,_retstat
mov	_retlab,(sp)
jsr	pc,*$_branch
L222:tst	_reached
jne	L228
mov	$L229,(sp)
jsr	pc,*$_werror
L228:clr	_reached
jbr	L106
L230:~temp=177300
mov	_curftn,_idname
mov	_NIL,(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,-500(r5)
mov	-500(r5),r1
mov	2(r1),r1
clr	r0
ashc	$-2,r0
bic	$17,r1
mov	r1,r0
mov	-500(r5),r1
mov	2(r1),r1
bic	$-20,r1
bis	r1,r0
mov	-500(r5),r1
mov	r0,2(r1)
mov	-2(r4),(sp)
mov	-500(r5),-(sp)
mov	$44,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,-500(r5)
mov	-500(r5),r0
mov	$141,*10(r0)
mov	$141,*-500(r5)
mov	_NIL,(sp)
mov	-500(r5),r0
mov	12(r0),-(sp)
mov	$154,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,(sp)
jsr	pc,*$_ecomp
bis	$1,_retstat
mov	_retlab,(sp)
jsr	pc,*$_branch
clr	_reached
jbr	L106
L231:~q=177300
mov	$4,(sp)
clr	-(sp)
mov	$64,-(sp)
mov	_NIL,-(sp)
mov	_NIL,-(sp)
mov	$141,-(sp)
jsr	pc,*$_block
add	$12,sp
mov	r0,-500(r5)
mov	-500(r5),r0
mov	-2(r4),r1
mov	r1,_idname
mov	r1,14(r0)
mov	$7,(sp)
mov	-500(r5),-(sp)
jsr	pc,*$_defid
tst	(sp)+
mov	_idname,r1
mul	$26,r1
mov	r1,r0
mov	_lineno,r1
neg	r1
mov	r1,24+_stab(r0)
mov	_idname,r1
mul	$26,r1
mov	16+_stab(r1),(sp)
jsr	pc,*$_branch
jbr	L222
jbr	L106
L232:~q=177300
mov	$6,(sp)
clr	-(sp)
mov	$64,-(sp)
mov	_NIL,-(sp)
mov	_NIL,-(sp)
mov	$141,-(sp)
jsr	pc,*$_block
add	$12,sp
mov	r0,-500(r5)
mov	-500(r5),r0
mov	-2(r4),14(r0)
mov	$6,(sp)
mov	-500(r5),-(sp)
jsr	pc,*$_defid
tst	(sp)+
mov	$1,_reached
jbr	L106
L233:mov	-2(r4),(sp)
jsr	pc,*$_addcase
mov	$1,_reached
jbr	L106
L234:mov	$1,_reached
jsr	pc,_adddef
bis	$10,_flostat
jbr	L106
L235:jsr	pc,_savebc
tst	_reached
jne	L236
mov	$L237,(sp)
jsr	pc,*$_werror
L236:mov	$12,_brklab
mov	$12,_contlab
mov	$12,r0
mov	r0,_yyval
mov	r0,(sp)
jsr	pc,*$_deflab
mov	$1,_reached
jbr	L106
L238:mov	$12,r0
mov	r0,_yyval
mov	r0,(sp)
jsr	pc,*$_bcon
mov	r0,(sp)
mov	-2(r4),-(sp)
mov	$155,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,(sp)
jsr	pc,*$_ecomp
mov	$1,_reached
jbr	L106
L239:tst	_reached
jeq	L240
mov	$12,r0
mov	r0,_yyval
mov	r0,(sp)
jsr	pc,*$_branch
jbr	L241
L240:mov	$-1,_yyval
L241:mov	-4(r4),(sp)
jsr	pc,*$_deflab
mov	$1,_reached
jbr	L106
L242:jsr	pc,_savebc
tst	_reached
jne	L243
mov	$L244,(sp)
jsr	pc,*$_werror
L243:cmp	$4,*-2(r4)
jne	L245
mov	-2(r4),r0
tst	10(r0)
jne	L10012
tst	10+2(r0)
jeq	L245
L10012:mov	$20,_flostat
L245:mov	$12,r0
mov	r0,_contlab
mov	r0,(sp)
jsr	pc,*$_deflab
mov	$1,_reached
mov	$12,_brklab
cmp	$20,_flostat
jne	L246
mov	-2(r4),(sp)
jsr	pc,*$_tfree
jbr	L247
L246:mov	_brklab,(sp)
jsr	pc,*$_bcon
mov	r0,(sp)
mov	-2(r4),-(sp)
mov	$155,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,(sp)
jsr	pc,*$_ecomp
L247:jbr	L106
L248:tst	-6(r4)
jeq	L249
mov	-6(r4),(sp)
jsr	pc,*$_ecomp
jbr	L250
L249:tst	_reached
jne	L251
mov	$L252,(sp)
jsr	pc,*$_werror
L251:L250:jsr	pc,_savebc
mov	$12,_contlab
mov	$12,_brklab
mov	$12,r0
mov	r0,_yyval
mov	r0,(sp)
jsr	pc,*$_deflab
mov	$1,_reached
tst	-2(r4)
jeq	L253
mov	_brklab,(sp)
jsr	pc,*$_bcon
mov	r0,(sp)
mov	-2(r4),-(sp)
mov	$155,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,(sp)
jsr	pc,*$_ecomp
jbr	L254
L253:bis	$20,_flostat
L254:jbr	L106
L255:jsr	pc,_savebc
mov	$12,_brklab
mov	_NIL,(sp)
mov	-2(r4),-(sp)
mov	$154,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,(sp)
jsr	pc,*$_ecomp
mov	$12,r0
mov	r0,_yyval
mov	r0,(sp)
jsr	pc,*$_branch
jsr	pc,_swstart
clr	_reached
jbr	L106
L256:mov	_instruc,_yyval
clr	r0
mov	r0,_instruc
mov	r0,_stwart
jbr	L106
L257:mov	(r4),(sp)
jsr	pc,*$_icons
mov	r0,_yyval
mov	-2(r4),_instruc
jbr	L106
L258:clr	_yyval
jbr	L106
L259:jbr	L260
jbr	L106
L261:L262:cmp	$32,_yychar
jeq	L10013
cmp	$33,_yychar
jeq	L10013
cmp	$16,_yychar
jeq	L10013
cmp	$21,_yychar
jeq	L10013
cmp	$23,_yychar
jne	L263
L10013:L264:tst	_hflag
jeq	L265
mov	$L266,(sp)
jsr	pc,*$_werror
L265:L263:L260:mov	(r4),(sp)
mov	-4(r4),-(sp)
mov	-2(r4),-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L267:mov	$73,-2(r4)
jbr	L260
jbr	L106
L268:jbr	L260
jbr	L106
L269:cmp	$35,_yychar
jeq	L264
jbr	L260
jbr	L106
L270:cmp	$35,_yychar
jeq	L264
jbr	L260
jbr	L106
L271:cmp	$6,_yychar
jeq	L264
cmp	$10,_yychar
jeq	L264
jbr	L260
jbr	L106
L272:jbr	L260
jbr	L106
L273:jbr	L262
jbr	L106
L274:cmp	$32,_yychar
jeq	L262
cmp	$33,_yychar
jeq	L262
jbr	L260
jbr	L106
L275:cmp	$32,_yychar
jeq	L262
cmp	$33,_yychar
jeq	L262
jbr	L260
jbr	L106
L276:cmp	$32,_yychar
jeq	L262
cmp	$33,_yychar
jeq	L262
jbr	L260
jbr	L106
L277:jbr	L260
jbr	L106
L278:jbr	L260
jbr	L106
L279:L280:mov	(r4),(sp)
mov	-6(r4),-(sp)
mov	-4(r4),-(sp)
inc	(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L281:jbr	L280
jbr	L106
L282:jbr	L280
jbr	L106
L283:jbr	L280
jbr	L106
L284:jbr	L280
jbr	L106
L285:jbr	L280
jbr	L106
L286:jbr	L280
jbr	L106
L287:jbr	L280
jbr	L106
L288:mov	(r4),(sp)
mov	-4(r4),-(sp)
mov	$26,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,(sp)
mov	-10(r4),-(sp)
mov	$25,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L289:mov	$L290,(sp)
jsr	pc,*$_werror
jbr	L260
jbr	L106
L291:jbr	L260
jbr	L106
L292:mov	$1,(sp)
jsr	pc,*$_bcon
mov	r0,(sp)
mov	-2(r4),-(sp)
mov	(r4),-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L293:L294:mov	_NIL,(sp)
mov	(r4),-(sp)
mov	-2(r4),-(sp)
add	$2,(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L295:mov	(r4),r0
mov	2(r0),r0
bic	$-61,r0
cmp	$40,r0
jeq	L10014
mov	(r4),r0
mov	2(r0),r0
bic	$-61,r0
cmp	$60,r0
jne	L296
L10014:mov	$L297,(sp)
jsr	pc,*$_werror
mov	(r4),_yyval
jbr	L298
L296:jbr	L294
L298:jbr	L106
L299:jbr	L294
jbr	L106
L300:mov	_NIL,(sp)
mov	(r4),-(sp)
mov	-2(r4),-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L301:mov	$1,(sp)
jsr	pc,*$_bcon
mov	r0,(sp)
mov	(r4),-(sp)
cmp	$116,-2(r4)
jne	L10015
mov	$7,-(sp)
jbr	L10016
L10015:mov	$11,-(sp)
L10016:jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L302:mov	(r4),(sp)
jsr	pc,*$_doszof
mov	r0,_yyval
jbr	L106
L303:mov	(r4),(sp)
mov	-4(r4),-(sp)
mov	$157,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
mov	_yyval,r0
mov	$141,*10(r0)
mov	$141,*_yyval
mov	_yyval,r0
mov	12(r0),_yyval
jbr	L106
L304:mov	-2(r4),(sp)
jsr	pc,*$_doszof
mov	r0,_yyval
jbr	L106
L305:mov	_NIL,(sp)
mov	-2(r4),-(sp)
mov	-6(r4),-(sp)
mov	$6,-(sp)
jsr	pc,_buildtr
add	$6,sp
mov	r0,-(sp)
mov	$15,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L306:mov	_NIL,(sp)
mov	-2(r4),-(sp)
mov	$110,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L307:mov	-2(r4),(sp)
mov	-4(r4),-(sp)
mov	$106,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L308:cmp	$104,-2(r4)
jne	L309
mov	_NIL,(sp)
mov	-4(r4),-(sp)
mov	$20,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,-4(r4)
L309:mov	(r4),_idname
mov	_NIL,(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,(sp)
mov	-4(r4),-(sp)
mov	$105,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L310:mov	(r4),_idname
tst	_blevel
jne	L311
mov	_idname,r1
mul	$26,r1
tst	10+_stab(r1)
jne	L311
~q=177300
mov	_idname,r1
mul	$26,r1
add	$_stab,r1
mov	r1,(sp)
mov	$L312,-(sp)
jsr	pc,*$_werror
tst	(sp)+
mov	$4,(sp)
clr	-(sp)
mov	$4,-(sp)
mov	_NIL,-(sp)
mov	_NIL,-(sp)
mov	$141,-(sp)
jsr	pc,*$_block
add	$12,sp
mov	r0,-500(r5)
mov	-500(r5),r0
mov	_idname,14(r0)
mov	$2,(sp)
mov	-500(r5),-(sp)
jsr	pc,*$_defid
tst	(sp)+
L311:mov	_NIL,(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
mov	(r4),r1
mul	$26,r1
mov	r1,r0
mov	_lineno,r1
neg	r1
mov	r1,24+_stab(r0)
jbr	L106
L313:clr	(sp)
jsr	pc,*$_bcon
mov	r0,_yyval
mov	_yyval,r0
mov	2+_lastcon,10+2(r0)
mov	_lastcon,10(r0)
mov	_yyval,r0
mov	$40000,14(r0)
tst	(r4)
jeq	L314
mov	_yyval,-(sp)
add	$2,(sp)
mov	$5,-(sp)
jsr	pc,_ctype
tst	(sp)+
mov	r0,*(sp)+
mov	_yyval,r1
mov	r0,6(r1)
L314:jbr	L106
L315:mov	_NIL,(sp)
mov	_NIL,-(sp)
mov	$5,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
mov	_yyval,r0
movf	_dcon,r0
movf	r0,10(r0)
jbr	L106
L316:jsr	pc,_getstr
mov	r0,_yyval
jbr	L106
L317:mov	-2(r4),_yyval
jbr	L106
L318:mov	(r4),(sp)
mov	-2(r4),-(sp)
jsr	pc,*$_tymerge
tst	(sp)+
mov	r0,_yyval
mov	$2,*_yyval
mov	$141,*-2(r4)
jbr	L106
L319:mov	$-1,(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L320:clr	(sp)
mov	$-1,-(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,_bdty
add	$6,sp
mov	r0,-(sp)
mov	$110,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L321:clr	(sp)
mov	-6(r4),-(sp)
mov	$110,-(sp)
jsr	pc,*$_bdty
cmp	(sp)+,(sp)+
mov	r0,_yyval
jbr	L106
L322:jbr	L160
jbr	L106
L323:jbr	L164
jbr	L106
L324:jbr	L166
jbr	L106
L325:mov	-2(r4),_yyval
jbr	L106
L326:mov	-2(r4),r1
mul	$26,r1
tst	10+_stab(r1)
jne	L327
~q=177300
mov	$4,(sp)
clr	-(sp)
mov	$44,-(sp)
mov	_NIL,-(sp)
mov	_NIL,-(sp)
mov	$141,-(sp)
jsr	pc,*$_block
add	$12,sp
mov	r0,-500(r5)
mov	-500(r5),r0
mov	-2(r4),14(r0)
mov	$2,(sp)
mov	-500(r5),-(sp)
jsr	pc,*$_defid
tst	(sp)+
L327:mov	-2(r4),_idname
mov	_NIL,(sp)
mov	_NIL,-(sp)
mov	$2,-(sp)
jsr	pc,*$_buildtr
cmp	(sp)+,(sp)+
mov	r0,_yyval
mov	_idname,r1
mul	$26,r1
mov	r1,r0
mov	_lineno,r1
neg	r1
mov	r1,24+_stab(r0)
jbr	L106
jbr	L106
L107:sub	$2,r0
cmp	r0,$257
jhi	L106
asl	r0
jmp	*L10018(r0)
	.data
L10018:L108
L109
L110
L111
L112
L113
L114
L106
L106
L118
L106
L119
L120
L121
L106
L122
L123
L124
L106
L125
L126
L106
L127
L128
L129
L106
L130
L131
L106
L106
L132
L133
L134
L135
L106
L106
L136
L137
L138
L139
L140
L141
L106
L106
L142
L143
L147
L148
L149
L106
L106
L150
L155
L158
L159
L161
L163
L165
L169
L170
L171
L172
L173
L174
L175
L176
L179
L180
L181
L182
L106
L183
L106
L184
L106
L185
L186
L187
L188
L106
L106
L189
L190
L106
L106
L106
L106
L191
L106
L193
L194
L196
L200
L106
L201
L202
L204
L207
L209
L214
L217
L223
L227
L230
L231
L106
L106
L106
L106
L232
L233
L234
L235
L238
L239
L242
L248
L255
L256
L257
L106
L258
L106
L259
L261
L267
L268
L269
L270
L271
L272
L273
L274
L275
L276
L277
L278
L279
L281
L282
L283
L284
L285
L286
L287
L288
L289
L291
L106
L292
L293
L295
L299
L300
L301
L302
L303
L304
L305
L306
L307
L308
L310
L313
L315
L316
L317
L318
L319
L320
L321
L322
L323
L324
L325
L326
.text
L106:jbr	L66
L65:jmp	cret
L63:sub	$470,sp
jbr	L64
.globl	fltused
.globl
.data
L14:.byte 142,141,144,40,142,144,164,171,0
L19:.byte 144,151,155,145,156,163,151,157,156,40,164,141,142,154
.byte 145,40,157,166,145,162,146,154,157,167,0
L24:.byte 167,150,151,154,145,163,54,40,146,157,162,163,54,40
.byte 145,164,143,56,40,164,157,157,40,144,145,145,160,154,171
.byte 40,156,145,163,164,145,144,0
L32:.byte 156,157,156,55,143,157,156,163,164,141,156,164,40,143
.byte 141,163,145,40,145,170,160,162,145,163,163,151,157,156,0
L34:.byte 143,141,163,145,40,156,157,164,40,151,156,40,163,167
.byte 151,164,143,150,0
L36:.byte 163,167,151,164,143,150,40,164,141,142,154,145,40,157
.byte 166,145,162,146,154,157,167,0
L41:.byte 144,165,160,154,151,143,141,164,145,40,144,145,146,141
.byte 165,154,164,40,151,156,40,163,167,151,164,143,150,0
L43:.byte 144,145,146,141,165,154,164,40,156,157,164,40,151,156
.byte 163,151,144,145,40,163,167,151,164,143,150,0
L48:.byte 163,167,151,164,143,150,40,164,141,142,154,145,40,157
.byte 166,145,162,146,154,157,167,0
L62:.byte 144,165,160,154,151,143,141,164,145,40,143,141,163,145
.byte 40,151,156,40,163,167,151,164,143,150,54,40,45,144,0
L68:.byte 163,164,141,164,145,40,45,144,54,40,143,150,141,162
.byte 40,60,45,157,12,0
L70:.byte 171,141,143,143,40,163,164,141,143,153,40,157,166,145
.byte 162,146,154,157,167,0
L90:.byte 163,171,156,164,141,170,40,145,162,162,157,162,0
L98:.byte 145,162,162,157,162,40,162,145,143,157,166,145,162,171
.byte 40,160,157,160,163,40,163,164,141,164,145,40,45,144,54
.byte 40,165,156,143,157,166,145,162,163,40,45,144,12,0
L102:.byte 145,162,162,157,162,40,162,145,143,157,166,145,162,171
.byte 40,144,151,163,143,141,162,144,163,40,143,150,141,162,40
.byte 45,144,12,0
L104:.byte 162,145,144,165,143,145,40,45,144,12,0
L116:.byte 146,165,156,143,164,151,157,156,40,154,145,166,145,154
.byte 40,145,162,162,157,162,0
L146:.byte 44,45,144,106,101,113,105,0
L152:.byte 146,151,145,154,144,40,157,165,164,163,151,144,145,40
.byte 157,146,40,163,164,162,165,143,164,165,162,145,0
L154:.byte 151,154,154,145,147,141,154,40,146,151,145,154,144,40
.byte 163,151,172,145,0
L157:.byte 146,151,145,154,144,40,157,165,164,163,151,144,145,40
.byte 157,146,40,163,164,162,165,143,164,165,162,145,0
L168:.byte 172,145,162,157,40,157,162,40,156,145,147,141,164,151
.byte 166,145,40,163,165,142,163,143,162,151,160,164,0
L178:.byte 146,165,156,143,164,151,157,156,40,144,145,143,154,141
.byte 162,141,164,151,157,156,40,151,156,40,142,141,144,40,143
.byte 157,156,164,145,170,164,0
L192:.byte 157,154,144,55,146,141,163,150,151,157,156,145,144,40
.byte 151,156,151,164,151,141,154,151,172,141,164,151,157,156,72
.byte 40,165,163,145,40,75,0
L199:.byte 156,145,163,164,151,156,147,40,164,157,157,40,144,145
.byte 145,160,0
L219:.byte 151,154,154,145,147,141,154,40,142,162,145,141,153,0
L225:.byte 151,154,154,145,147,141,154,40,143,157,156,164,151,156
.byte 165,145,0
L229:.byte 163,164,141,164,145,155,145,156,164,40,156,157,164,40
.byte 162,145,141,143,150,145,144,0
L237:.byte 154,157,157,160,40,156,157,164,40,145,156,164,145,162
.byte 145,144,40,141,164,40,164,157,160,0
L244:.byte 154,157,157,160,40,156,157,164,40,145,156,164,145,162
.byte 145,144,40,141,164,40,164,157,160,0
L252:.byte 154,157,157,160,40,156,157,164,40,145,156,164,145,162
.byte 145,144,40,141,164,40,164,157,160,0
L266:.byte 160,162,145,143,145,144,145,156,143,145,40,143,157,156
.byte 146,165,163,151,157,156,40,160,157,163,163,151,142,154,145
.byte 72,40,160,141,162,145,156,164,150,145,163,151,172,145,41,0
L290:.byte 157,154,144,55,146,141,163,150,151,157,156,145,144,40
.byte 141,163,163,151,147,156,155,145,156,164,40,157,160,145,162
.byte 141,164,157,162,0
L297:.byte 46,40,142,145,146,157,162,145,40,141,162,162,141,171
.byte 40,157,162,40,146,165,156,143,164,151,157,156,72,40,151
.byte 147,156,157,162,145,144,0
L312:.byte 165,156,144,145,143,154,141,162,145,144,40,151,156,151
.byte 164,151,141,154,151,172,145,162,40,156,141,155,145,40,45
.byte 56,70,163,0

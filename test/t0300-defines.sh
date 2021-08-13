#!/bin/sh
#
# Copyright (c) 2020 Nexbridge Inc. All Rights Reserved.
#

test_description='Test GUARDIAN defines 
'

. ./test-lib.sh

test_expect_success 'define_add map test no dependencies' '
	edit_loader makefile <<-EOF &&
a
\$(info Test on define_add map)
\$(define_add =OBJ,map,OBJ14)
\$(define_add =SRC,map,SRC14)
.PHONY: always
always:
        \$(TAL)/IN =SRC/=OBJ
//
EOF
	edit_loader src14 <<-EOF &&
a
! SRC14
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make
'

test_expect_success 'define_delete map with define_add' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(info Test on define_delete map)
\$(define_delete =SRC)
\$(define_delete **)
\$(define_add =OBJ,map,OBJ16)
\$(define_add =SRC,map,SRC16)
\$(define_delete =BOB)
.PHONY: always
always:
        \$(TAL)/IN =SRC/=OBJ
//
EOF
	edit_loader src16 <<-EOF &&
a
! SRC16
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make
'

test_expect_success 'define_add catalog simple run' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(info Test on define_add catalog)
\$(define_add =OBJ,map,OBJ18)
\$(define_add =SRC,map,SRC18)
\$(define_add =CAT1,catalog,THISSV)
.PHONY: always
always:
        \$(TAL)/IN =SRC/=OBJ
        -\$(SQLCOMP)/IN =OBJ,OUT COMP18/CATALOG =CAT1
//
EOF
	edit_loader src18 <<-EOF &&
a
! SRC18
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make &&
	echo THISSV > expecting &&
	cat $TEST_GUARDIAN_DIR/comp18 | fgrep "SQL -  PROGRAM CATALOG" | \
		sed "s/.*THIS/THIS/" > actual &&
	test_cmp expecting actual
'

test_expect_success 'define_delete catalog simple run with define_add' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(info Test on define_add catalog)
\$(define_add =CAT1,catalog,THISSV)
\$(define_delete =CAT1)
\$(define_add =OBJ,map,OBJ19)
\$(define_add =SRC,map,SRC19)
\$(define_add =CAT1,catalog,THATSV)
.PHONY: always
always:
        \$(TAL)/IN =SRC/=OBJ
        -\$(SQLCOMP)/IN =OBJ,OUT COMP19/CATALOG =CAT1
//
EOF
	edit_loader src19 <<-EOF &&
dq!a
a
! SRC19
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make &&
	echo THATSV > expecting &&
	cat $TEST_GUARDIAN_DIR/comp19 | fgrep "SQL -  PROGRAM CATALOG" | \
		sed "s/.*THAT/THAT/" > actual &&
	test_cmp expecting actual
'

test_expect_success 'define_add spool simple run' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(info Test on define_add spool)
\$(define_add =OBJ,map,OBJ20)
\$(define_add =SRC,map,SRC20)
\$(define_add =SPOOL,spool,\$s.#t0300)
.PHONY: always
always:
        \$(TAL)/IN =SRC,OUT =SPOOL/=OBJ
//
EOF
	edit_loader src20 <<-EOF &&
a
! SRC20
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_spoolcom <<-EOF &&
	job(loc #t0300),delete !
	EOF
	launch_make &&
	launch_spoolcom <<-EOF | fgrep "RDY" > spool &&
	job(loc #t0300)
	job(loc #t0300),delete !
	EOF
	echo > expecting &&
	cat spool > actual &&
	! test_cmp expecting actual
'

test_expect_success 'define_add spool copies' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(info Test on define_add spool)
\$(define_add =OBJ,map,OBJ21)
\$(define_add =SRC,map,SRC21)
\$(define_add =SPOOL,spool,\$s.#t0300,copies=4)
.PHONY: always
always:
        \$(TAL)/IN =SRC,OUT =SPOOL/=OBJ
//
EOF
	edit_loader src21 <<-EOF &&
a
! SRC21
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_spoolcom <<-EOF &&
	job(loc #t0300),delete !
	EOF
	launch_make &&
	launch_spoolcom <<-EOF | fgrep "RDY" > spool &&
	job(loc #t0300)
	job(loc #t0300),delete !
	EOF
	echo > expecting &&
	grep < spool "RDY 4  *[^ ]*  *[^ ]*  *4"  > actual &&
	! test_cmp expecting actual
'

test_expect_success 'define_add spool report' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(info Test on define_add spool report)
\$(define_add =OBJ,map,OBJ22)
\$(define_add =SRC,map,SRC22)
\$(define_add =SPOOL,spool,\$s.#t0300,report=nothing)
.PHONY: always
always:
        \$(TAL)/IN =SRC,OUT =SPOOL/=OBJ
//
EOF
	edit_loader src22 <<-EOF &&
a
! SRC22
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_spoolcom <<-EOF &&
	job(loc #t0300),delete !
	EOF
	launch_make &&
	launch_spoolcom <<-EOF | fgrep "RDY" > spool &&
	job(loc #t0300)
	job(loc #t0300),delete !
	EOF
	echo > expecting &&
	grep < spool "RDY 4  *[^ ]*  *[^ ]*  *1  *[^ ]*  *NOTHING  *#T0300"  > actual &&
	! test_cmp expecting actual
'

test_expect_success 'define_add spool report and copies' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(info Test on define_add spool report and copies)
\$(define_add =OBJ,map,OBJ23)
\$(define_add =SRC,map,SRC23)
\$(define_add =SPOOL,spool,\$s.#t0300,report=nothing,copies=2)
.PHONY: always
always:
        \$(TAL)/IN =SRC,OUT =SPOOL/=OBJ
//
EOF
	edit_loader src23 <<-EOF &&
a
! SRC23
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_spoolcom <<-EOF &&
	job(loc #t0300),delete !
	EOF
	launch_make &&
	launch_spoolcom <<-EOF | fgrep "RDY" > spool &&
	job(loc #t0300)
	job(loc #t0300),delete !
	EOF
	echo > expecting &&
	grep < spool "RDY 4  *[^ ]*  *[^ ]*  *2  *[^ ]*  *NOTHING  *#T0300"  > actual &&
	! test_cmp expecting actual
'

test_expect_success 'define_add search simple' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(info Test on define_add search simple)
\$(define_add =srch1,search,subvol=a)
.PHONY: always
always:
        \$(TACL)/IN MAC1,OUT TACL1/
//
EOF
	edit_loader mac1 <<-EOF &&
dq!a
a
INFO DEFINE =SRCH1,DETAIL
//
EOF
	launch_make &&
	cat > expecting <<-EOF &&
	Define Name        =SRCH1
	CLASS              SEARCH
	SUBVOL0            \$VOL.A
	EOF
	cat $TEST_GUARDIAN_DIR/tacl1 | sed "1,/INFO DEFINE/d" \
		| sed "s/^  *//g" | sed "1,\$s/\\\\[^.]*\.[^.]*\./\$VOL./g" \
		> actual &&
	test_cmp expecting actual
'

test_expect_success 'define_add search complex' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(info Test on define_add search simple)
\$(define_add =srch1,search,subvol=a,relsubvol=b,subvol=c,relsubvol=d)
.PHONY: always
always:
        \$(TACL)/IN MAC2,OUT TACL2/
//
EOF
	edit_loader mac2 <<-EOF &&
dq!a
a
INFO DEFINE =SRCH1,DETAIL
//
EOF
	launch_make &&
	cat > expecting <<-EOF &&
	Define Name        =SRCH1
	CLASS              SEARCH
	SUBVOL0            \$VOL.A
	RELSUBVOL0         B
	SUBVOL1            \$VOL.C
	RELSUBVOL1         D
	EOF
	cat $TEST_GUARDIAN_DIR/tacl2 | sed "1,/INFO DEFINE/d" \
		| sed "s/^  *//g" | sed "1,\$s/\\\\[^.]*\.[^.]*\./\$VOL./g" \
		> actual &&
	test_cmp expecting actual
'

test_done

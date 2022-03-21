#!/bin/sh
#
# Copyright (c) 2021 Nexbridge Inc. All Rights Reserved.
#

test_description='Test GUARDIAN PARAMs 
'

. ./test-lib.sh

test_expect_success 'PARAM with missing value' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(param P1)

obj1: src1 
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make
'

test_expect_success 'PARAM command with missing value' '
	edit_loader makefile <<-EOF &&
dq!a
a

obj1: src1
        PARAM p1 
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make
'

test_expect_success 'PARAM with missing end quote' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(param P1 "Hello)

obj1: src1 
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make
'

test_expect_success 'PARAM command with missing end quote' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        PARAM P1 "Hello
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make
'

test_expect_failure 'PARAM with a bad value passed to a compiler' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(param swapvol \$NOVALID)

obj1: src1 
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make -d > capture &&
	fgrep PARAM <capture >actual &&
	cat >expecting <<-EOF &&
	PARAM SWAPVOL \$NOVALID added
	EOF
	test_cmp expecting actual
'

test_expect_failure 'PARAM command with a bad value passed to a compiler' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        param swapvol \$NOVALID
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make -d > capture &&
	fgrep PARAM <capture >actual &&
	cat >expecting <<-EOF &&
	PARAM SWAPVOL \$NOVALID added
	EOF
	test_cmp expecting actual
'

test_expect_success 'PARAM add and delete single' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(param swapvol \$NOVALID)
\$(param p1 "Hello There")
\$(param p2 Hello)
\$(clear_param p2)
\$(param swapvol \$SYSTEM)
obj1: src1 
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep PARAM <capture >actual &&
	cat >expecting <<-EOF &&
	PARAM SWAPVOL \$NOVALID added
	PARAM P1 Hello There added
	PARAM P2 Hello added
	PARAM P2 removed
	PARAM SWAPVOL \$SYSTEM replaced
	EOF
	test_cmp expecting actual
'

test_expect_success 'PARAM command add and delete single' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        param swapvol \$NOVALID
        param p1 "Hello There"
        param p2 Hello
        clear param p2
        param swapvol \$SYSTEM
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	sleep 5 &&
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep PARAM <capture >actual &&
	cat >expecting <<-EOF &&
	PARAM SWAPVOL \$NOVALID added
	PARAM P1 Hello There added
	PARAM P2 Hello added
	PARAM P2 removed
	PARAM SWAPVOL \$SYSTEM replaced
	EOF
	test_cmp expecting actual
'

test_expect_success 'PARAM add and delete all' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(param swapvol \$NOVALID)
\$(param p1 "Hello There")
\$(param p2 Hello)
\$(clear_param p2)
\$(clear_param *)
obj1: src1 
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	sleep 5 &&
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep PARAM <capture >actual &&
	cat >expecting <<-EOF &&
	PARAM SWAPVOL \$NOVALID added
	PARAM P1 Hello There added
	PARAM P2 Hello added
	PARAM P2 removed
	PARAM SWAPVOL removed
	PARAM P1 removed
	EOF
	test_cmp expecting actual
'

test_expect_success 'PARAM command add and delete all' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        param swapvol \$NOVALID
        param p1 "Hello There"
        param p2 Hello
        clear param p2
        clear param all
        \$(TAL)/IN \$<,TERM \$NULL/ obj1
//
EOF
	sleep 5 &&
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep PARAM <capture >actual &&
	cat >expecting <<-EOF &&
	PARAM SWAPVOL \$NOVALID added
	PARAM P1 Hello There added
	PARAM P2 Hello added
	PARAM P2 removed
	PARAM SWAPVOL removed
	PARAM P1 removed
	EOF
	test_cmp expecting actual
'

test_done

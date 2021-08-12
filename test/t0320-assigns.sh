#!/bin/sh
#
# Copyright (c) 2021 Nexbridge Inc. All Rights Reserved.
#

test_description='Test GUARDIAN ASSIGNs 
'

. ./test-lib.sh

test_expect_success 'ASSIGN with missing file' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(assign ssv0)

obj1: src1 
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make
'

test_expect_success 'ASSIGN command with missing file' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        assign ssv0
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make
'

test_expect_success 'ASSIGN with empty file' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(assign ssv0,)

obj1: src1 
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make
'

test_expect_success 'ASSIGN command with empty file' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        assign ssv0,
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make
'

test_expect_success 'ASSIGN with a bad value passed to a compiler' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(assign SSV0,\$A.B)
\$(assign SSV1,\$SYSTEM.SYSTEM)

obj1: src1 
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make -d > capture &&
	fgrep ASSIGN <capture >actual &&
	cat >expecting <<-EOF &&
	ASSIGN SSV0 \$A.B added
	ASSIGN SSV1 \$SYSTEM.SYSTEM added
	EOF
	test_cmp expecting actual
'

test_expect_success 'ASSIGN command with a bad value passed to a compiler' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        assign SSV0,\$A.B
        assign SSV1,\$SYSTEM.SYSTEM
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	test_expect_code 2 launch_make -d > capture &&
	fgrep ASSIGN <capture >actual &&
	cat >expecting <<-EOF &&
	ASSIGN SSV0 \$A.B added
	ASSIGN SSV1 \$SYSTEM.SYSTEM added
	EOF
	test_cmp expecting actual
'

test_expect_success 'ASSIGN with a good value passed to a compiler' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(assign SSV1,\$A.B)
\$(assign SSV0,\$SYSTEM.SYSTEM)

obj1: src1 
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep ASSIGN <capture >actual &&
	cat >expecting <<-EOF &&
	ASSIGN SSV1 \$A.B added
	ASSIGN SSV0 \$SYSTEM.SYSTEM added
	EOF
	test_cmp expecting actual
'

test_expect_success 'ASSIGN command with a good value passed to a compiler' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        assign SSV1,\$A.B
        assign SSV0,\$SYSTEM.SYSTEM
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep ASSIGN <capture >actual &&
	cat >expecting <<-EOF &&
	ASSIGN SSV1 \$A.B added
	ASSIGN SSV0 \$SYSTEM.SYSTEM added
	EOF
	test_cmp expecting actual
'

test_expect_success 'ASSIGN add and delete all' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(assign SSV0,\$A.B)
\$(assign SSV1,\$SYSTEM.SYSTEM)
\$(clear_assign *)
\$(assign SSV0,\$SYSTEM.SYSTEM)

obj1: src1 
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep ASSIGN <capture >actual &&
	cat >expecting <<-EOF &&
	ASSIGN SSV0 \$A.B added
	ASSIGN SSV1 \$SYSTEM.SYSTEM added
	ASSIGN STDERR removed
	ASSIGN SSV0 removed
	ASSIGN SSV1 removed
	ASSIGN SSV0 \$SYSTEM.SYSTEM added
	EOF
	test_cmp expecting actual
'

test_expect_success 'ASSIGN command add and delete all' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        assign SSV0,\$A.B
        assign SSV1,\$SYSTEM.SYSTEM
        clear assign all
        assign SSV0,\$SYSTEM.SYSTEM
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep ASSIGN <capture >actual &&
	cat >expecting <<-EOF &&
	ASSIGN SSV0 \$A.B added
	ASSIGN SSV1 \$SYSTEM.SYSTEM added
	ASSIGN STDERR removed
	ASSIGN SSV0 removed
	ASSIGN SSV1 removed
	ASSIGN SSV0 \$SYSTEM.SYSTEM added
	EOF
	test_cmp expecting actual
'

test_expect_success 'ASSIGN add and delete single' '
	edit_loader makefile <<-EOF &&
dq!a
a
\$(assign SSV0,\$A.B)
\$(assign SSV1,\$SYSTEM.SYSTEM)
\$(clear_assign SSV0)

obj1: src1 
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep ASSIGN <capture >actual &&
	cat >expecting <<-EOF &&
	ASSIGN SSV0 \$A.B added
	ASSIGN SSV1 \$SYSTEM.SYSTEM added
	ASSIGN SSV0 removed
	EOF
	test_cmp expecting actual
'

test_expect_success 'ASSIGN command add and delete single' '
	edit_loader makefile <<-EOF &&
dq!a
a
obj1: src1 
        assign SSV0,\$A.B
        assign SSV1,\$SYSTEM.SYSTEM
        clear assign SSV0
        \$(TAL)/IN \$</ obj1
//
EOF
	edit_loader src1 <<-EOF &&
dq!a
a
! SRC1
?nolist, source EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make -d > capture &&
	fgrep ASSIGN <capture >actual &&
	cat >expecting <<-EOF &&
	ASSIGN SSV0 \$A.B added
	ASSIGN SSV1 \$SYSTEM.SYSTEM added
	ASSIGN SSV0 removed
	EOF
	test_cmp expecting actual
'

test_done

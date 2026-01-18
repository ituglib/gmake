#!/bin/sh
#
# Copyright (c) 2020 Nexbridge Inc. All Rights Reserved.
#

test_description='Test GUARDIAN ar utility
'

. ./test-lib.sh

test_expect_success 'simple ar operation' '
	edit_loader makefile <<-EOF &&
a
all: a(obj)
obj: src
        \$(TAL)/IN SRC,TERM \$NULL/OBJ
a(obj): obj
        \$(AR)/IN \$NULL,TERM \$NULL,OUT \$NULL/ \$(ARFLAGS) \$@ \$<
//
EOF
	edit_loader src <<-EOF &&
a
! SRC
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make
'

test_expect_success 'retest ar - should not run' '
	launch_make > capture &&
	sed "1,\$s/^.*:/:/" < capture > actual &&
	echo ": Nothing to be done for ${QUOTE}all${QUOTE}." >expecting &&
	test_cmp expecting actual
'

test_expect_success 'touch ar member - should not run' '
	edit_loader makefile <<-EOF &&
dq!a
a
all: a(obj)
obj: src
        \$(TAL)/IN SRC,TERM \$NULL/OBJ
a(obj): obj
        \$(delay 10 seconds)
        \$(AR)/IN \$NULL,TERM \$NULL,OUT \$NULL/ \$(ARFLAGS) \$@ \$<
//
EOF
	sleep 2 &&
	edit_loader src <<-EOF &&
a

//
EOF
	srcstat=`stat --format=%Y ${MAKE_SUBVOL_PREFIX}$this_test/src` &&
	objstat=`stat --format=%Y ${MAKE_SUBVOL_PREFIX}$this_test/obj` &&
	astat=`stat --format=%Y ${MAKE_SUBVOL_PREFIX}$this_test/a` &&
	sleep 2 &&
	launch_make --touch &&
	srcstat2=`stat --format=%Y ${MAKE_SUBVOL_PREFIX}$this_test/src` &&
	objstat2=`stat --format=%Y ${MAKE_SUBVOL_PREFIX}$this_test/obj` &&
	astat2=`stat --format=%Y ${MAKE_SUBVOL_PREFIX}$this_test/a` &&
	test $srcstat -eq $srcstat2 &&
	test $objstat -lt $objstat2 &&
	test $astat -lt $astat2
'

test_done

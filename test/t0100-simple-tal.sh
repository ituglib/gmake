#!/bin/sh
#
# Copyright (c) 2020 Nexbridge Inc. All Rights Reserved.
#

test_description='Simple TAL test
'

. ./test-lib.sh

test_expect_success 'TAL compile, always builds' '
	edit_loader makefile <<-EOF &&
a
all:
        \$(TAL)/IN A/OBJ1
//
EOF
	edit_loader a <<-EOF &&
a
PROC M MAIN;
BEGIN
END;
//
EOF
	launch_make
'

test_expect_success 'TAL compile, builds again builds' '
	launch_make
'

test_expect_success 'TAL compile, should not build' '
	edit_loader makefile <<-EOF &&
dq!a
a
all: obj1

obj1: a
        \$(TAL)/IN A/OBJ1
//
EOF
	launch_make
'

test_done

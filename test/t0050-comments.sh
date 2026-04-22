#!/bin/sh
#
# Copyright (c) 2026 Nexbridge Inc. All Rights Reserved.
#

test_description='Simple Comment Test
'

. ./test-lib.sh

test_expect_success 'Comment compile, always builds' '
	edit_loader makefile <<-EOF &&
a
all:
        # Simple comment before TAL
        \$(TAL)/IN A,TERM \$NULL/OBJ1
        # Simple command after TAL
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

test_expect_success 'Second compile, builds again builds' '
	launch_make
'

test_expect_success 'Third compile, should not build' '
	edit_loader makefile <<-EOF &&
dq!a
a
all: obj1

obj1: a
        # Comment before should not execute
        \$(TAL)/IN A,TERM \$NULL/OBJ1
        # Comment after should not execute
//
EOF
	launch_make
'

test_done

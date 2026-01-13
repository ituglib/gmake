#!/bin/sh
#
# Copyright (c) 2026 Nexbridge Inc. All Rights Reserved.
#

test_description='Test INV 
'

. ./test-lib.sh

test_expect_success 'Test INV simple case' '
	edit_loader src <<-EOF &&
dq!a
a
#include <stdio.h>

int main(int argc, char **argv) {
return 0;
}
//
EOF
	edit_loader makefile <<-EOF &&
dq!a
a
DUMP=p out;e

all:
        \$(FUP) PURGE out !
        \$(EDIT) /INV DUMP/ src
        \$(FUP) INFO out

.PHONY: all
//
EOF
	launch_make
'

test_done

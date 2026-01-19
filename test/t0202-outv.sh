#!/bin/sh
#
# Copyright (c) 2026 Nexbridge Inc. All Rights Reserved.
#

test_description='Test INV 
'

. ./test-lib.sh

test_expect_success 'Test OUTV simple case' '
	edit_loader src <<-EOF &&
dq!a
a
#include <stdio.h>

int main(int argc, char **argv) {
return 0;
}
//
e
EOF
	edit_loader makefile <<-EOF &&
dq!a
a
DUMP=
.PHONY: always
always:
        \$SYSTEM.SYSTEM.EDIT/OUTV DUMP/ src;lua;e
        @echo Dumping
        outvar DUMP
//
e
EOF
	launch_make | sed "s/^TEXT EDITOR.*/TEXT EDITOR/" \
		| sed "s/^CURRENT FILE.*/CURRENT FILE/" > actual &&
	cat >expecting <<-EOF &&
\$SYSTEM.SYSTEM.EDIT/OUTV DUMP/ src;lua;e
Dumping
outvar DUMP

TEXT EDITOR
CURRENT FILE
#include <stdio.h>

int main(int argc, char **argv) {
return 0;
}
	EOF
	test_cmp expecting actual
'

test_done

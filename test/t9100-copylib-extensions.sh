#!/bin/sh
#
# Copyright (c) 2021 Nexbridge Inc. All Rights Reserved.
#

test_description='Test copylib extensions
'

. ./test-lib.sh

test_expect_success 'simple copylib compile' '
	edit_loader makefile <<-EOF > /dev/null &&
dq!a
a
\$(define_delete =SPOOL)
\$(define_add =SPOOL,spool,\$s.#$this_test)
\$(indexsection copylib copylibt)
\$(indexsection copy2 copy2t)

all: prefix obj

prefix:
        \$(VAR)

obj: src copylib(a) copy2(c)
        \$(CC) /IN \$<,OUT =SPOOL/ \$@
//
EOF
	edit_loader src <<-EOF > /dev/null &&
a
#include <stdio.h> NOLIST
#include "copylib(a)"
#include "copy2(c)"

int main(int argc, char **argv) {
   a_def a;
   a.x = 4;
   printf("Hello world %d!\n", a.x);
}
//
EOF
	edit_loader copylib <<-EOF > /dev/null &&
a
/* HEAD */
#pragma SECTION a
typedef struct _a_def {
   int y;
   short x;
} a_def;
#pragma SECTION C
typedef struct _c_def {
   int k;
   int l;
} c_def;
//
EOF
	edit_loader copy2 <<-EOF > /dev/null &&
a
/* HEAD */
#pragma SECTION a
/* HEAD comment */
typedef struct _a_def {
   int y;
   short x;
} a_def;
#pragma SECTION C
typedef struct _c_def {
   int k;
   int l;
} c_def;
//
EOF
	launch_spoolcom_delete $this_test >/dev/null &&
	launch_make > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	\$SYSTEM.SYSTEM.C /IN src,OUT =SPOOL/ obj
	EOF
	test_cmp expecting actual
'

test_expect_success 'retest compile - should not run' '
	launch_make > capture &&
	sed "1,\$s/^.*:/:/" < capture > actual &&
	echo ": Nothing to be done for ${QUOTE}all${QUOTE}." >expecting &&
	test_cmp expecting actual
'

test_expect_success 'recompile - changed copy2 section C' '
	edit_loader copy2 <<-EOF > /dev/null &&
a

//
EOF
	launch_spoolcom_delete $this_test >/dev/null &&
	launch_make > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	\$SYSTEM.SYSTEM.C /IN src,OUT =SPOOL/ obj
	EOF
	test_cmp expecting actual
'

test_expect_success 'copylib member touch' '
	edit_loader makefile <<-EOF > /dev/null &&
dq!a
a
\$(indexsection copylib copylibt)
\$(indexsection copy2 copy2t)

copylib(a): src2
        \$(EDIT) /in src2/
//
EOF
	edit_loader src2 <<-EOF > /dev/null &&
a
g copylib !
dq!a
a
#pragma SECTION A
typedef struct a_ {
  int b;
  int c;
} a;
//
EOF
	launch_make --touch > actual &&
	cat >expecting <<-EOF &&
	touch copylib(a)
	EOF
	test_cmp expecting actual
'

test_expect_success 'retest compile after touch - should not run' '
	launch_make > capture &&
	sed "1,\$s/^.*:/:/" < capture > actual &&
	echo ": ${QUOTE}copylib(a)${QUOTE} is up to date." >expecting &&
	test_cmp expecting actual
'

test_done
